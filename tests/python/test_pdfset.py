"""Integration tests: run against a freshly built extension (see README.md)."""
import copy
import gc
import math
import os
import subprocess
import sys
import weakref
from pathlib import Path

import numpy as np
import pdfxtmd as pdf
import pytest


@pytest.fixture(params=["CPDF", "TMD"])
def case(request, grid_sets):
    if request.param == "CPDF":
        return pdf.CPDFSet, pdf.GenericCPDFFactory, "mkCPDF", "pdf", grid_sets[0], (0.01, 100.0)
    return pdf.TMDSet, pdf.GenericTMDFactory, "mkTMD", "tmd", grid_sets[1], (0.01, 4.0, 100.0)


def test_import_and_existing_factories():
    assert Path(pdf.__file__).suffix == ".so" or Path(pdf.__file__).suffix == ".pyd"
    for name in ("CPDFSet", "TMDSet", "PDFUncertainty", "CollinearPDF", "TMDPDF",
                 "ICPDF", "ITMD", "ICDPD"):
        assert hasattr(pdf, name)
    if pdf.__has_dpd__:
        assert hasattr(pdf, "CollinearDPD")
    for cls in (pdf.GenericCPDFFactory, pdf.GenericTMDFactory,
                pdf.GenericCDPDFactory, pdf.CouplingFactory):
        assert cls() is not None


def test_members_and_factory_agreement(case):
    cls, factory, make, evaluate, name, point = case
    group = cls(name)
    assert len(group) == 3
    for i in range(len(group)):
        member = group[i]
        direct = getattr(factory(), make)(name, i)
        for flavor in (pdf.PartonFlavor.g, pdf.PartonFlavor.u, pdf.PartonFlavor.d):
            assert getattr(member, evaluate)(flavor, *point) == pytest.approx(
                getattr(direct, evaluate)(flavor, *point), rel=1e-12)
    expected = 10 if evaluate == "pdf" else 2.5
    assert getattr(group[0], evaluate)(pdf.PartonFlavor.g, *point) == pytest.approx(expected)
    all_method = evaluate + "_all"
    values = getattr(group[0], all_method)(*point)
    output = []
    getattr(group[0], evaluate)(*point, output)
    assert output == pytest.approx(values)
    assert len(values) == 13
    if evaluate == "pdf":
        assert group[0].pdf_batch(pdf.PartonFlavor.g, np.array([point[0]]),
                                  np.array([point[1]]))[0] == pytest.approx(expected)


def test_metadata_and_coupling(case):
    cls, _, _, _, name, _ = case
    group = cls(name, alternativeReplicaUncertainty=False)
    info = group.getStdPDFInfo()
    err = group.getPDFErrorInfo()
    assert isinstance(info, pdf.YamlStandardPDFInfo)
    assert info.NumMembers == len(group)
    assert info.SetDesc == "Python binding test grid"
    assert info.orderQCD == pdf.OrderQCD.NLO
    assert info.QMin == 2 and info.QMax == 50
    assert info.Flavors == [1, 2, 21]
    assert err.ErrorType == "symmhessian"
    assert err.size == 3 and err.errorSize == 2
    coupling = pdf.CouplingFactory().mkCoupling(name)
    assert group.alphasQ2(100) == pytest.approx(coupling.AlphaQCDMu2(100), rel=1e-12)
    assert group.alphasQ(10) == pytest.approx(group.alphasQ2(100))
    del group
    gc.collect()
    assert info.NumMembers == 3 and err.size == 3


def test_uncertainty_and_correlation(case):
    cls, _, _, evaluate, name, point = case
    group = cls(name)
    values = [getattr(group[i], evaluate)(pdf.PartonFlavor.g, *point) for i in range(3)]
    result = group.Uncertainty(pdf.PartonFlavor.g, *point)
    vector_result = group.Uncertainty(values)
    divisor = 1 if evaluate == "pdf" else point[1]
    assert result.central == pytest.approx(10 / divisor)
    for field in ("errplus", "errminus", "errsymm", "errplus_pdf", "errminus_pdf", "errsymm_pdf"):
        assert getattr(result, field) == pytest.approx(math.sqrt(5) / divisor)
        assert getattr(result, field) == pytest.approx(getattr(vector_result, field))
    assert result.scale == 1
    assert result.errplus_par == result.errminus_par == result.errsymm_par == 0
    assert len(result.errparts) == 1
    assert result.errparts[0] == pytest.approx((math.sqrt(5) / divisor,) * 2)
    scaled = group.Uncertainty(pdf.PartonFlavor.g, *point, cl=90)
    assert scaled.errplus > result.errplus
    assert scaled.errplus == pytest.approx(result.errplus * scaled.scale)
    assert group.Correlation(pdf.PartonFlavor.g, *point, pdf.PartonFlavor.u, *point) == pytest.approx(1)
    assert group.Correlation(values, [-v for v in values]) == pytest.approx(-1)
    for method, args in [(group.Uncertainty, ([1],)), (group.Correlation, (values, [1])),
                         (group.Uncertainty, (values, 101))]:
        with pytest.raises(RuntimeError):
            method(*args)


def test_invalid_indices(case):
    cls, _, _, _, name, _ = case
    group = cls(name)
    for index in (-1, len(group), 1000000):
        with pytest.raises(IndexError, match="member index"):
            group[index]
    with pytest.raises(TypeError):
        group[0.5]


def test_lifetime_and_gc(case):
    cls, _, _, evaluate, name, point = case
    group = cls(name)
    with pytest.raises(TypeError):
        copy.copy(group)
    parent = weakref.ref(group)
    member = group[0]
    assert group[0] is member
    expected = getattr(member, evaluate)(pdf.PartonFlavor.g, *point)
    del group
    gc.collect()
    assert parent() is not None  # member must retain its unique_ptr owner
    assert getattr(member, evaluate)(pdf.PartonFlavor.g, *point) == expected
    del member
    gc.collect()
    assert parent() is None
    # Exercise temporaries and destruction repeatedly in a separate process,
    # so a double free or dangling reference fails this test cleanly.
    code = f"""
import gc, pdfxtmd as p
for _ in range(20):
    member = p.{cls.__name__}({name!r})[0]
    gc.collect()
    assert member.{evaluate}(p.PartonFlavor.g, *{point!r}) == {expected!r}
    del member
    gc.collect()
"""
    env = os.environ.copy()
    env["PYTHONPATH"] = str(Path(pdf.__file__).resolve().parent)
    subprocess.run([sys.executable, "-c", code], env=env, check=True, timeout=60)


def test_cpp_exceptions(grid_sets):
    for cls in (pdf.CPDFSet, pdf.TMDSet):
        with pytest.raises(RuntimeError, match="not found"):
            cls("BindingTestMissingSet")


@pytest.mark.parametrize("kind,variable,default", [
    ("CPDF", "PDFXTMD_TEST_CPDF", "CT18NLO"),
    ("TMD", "PDFXTMD_TEST_TMD", "PB-LO-HERAI+II-2020-set2"),
])
def test_external_grid(kind, variable, default):
    # Opt in only: no automatic downloading or silently skipping broken sets.
    name = os.environ.get(variable)
    if not name:
        pytest.skip(f"Set {variable}={default} to test an installed physics grid")
    cls, factory, make, evaluate, point = (
        (pdf.CPDFSet, pdf.GenericCPDFFactory, "mkCPDF", "pdf", (0.01, 100.0))
        if kind == "CPDF" else
        (pdf.TMDSet, pdf.GenericTMDFactory, "mkTMD", "tmd", (0.01, 10.0, 100.0)))
    group = cls(name)
    direct = getattr(factory(), make)(name, 0)
    assert getattr(group[0], evaluate)(pdf.PartonFlavor.g, *point) == pytest.approx(
        getattr(direct, evaluate)(pdf.PartonFlavor.g, *point), rel=1e-12)
    assert math.isfinite(group.Uncertainty(pdf.PartonFlavor.g, *point).central)
