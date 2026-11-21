"""Regression for the scalar/all-flavor Q2-edge bug (requires a rebuilt pdfxtmd)."""

import math

import pdfxtmd
import pytest


def _write_curved_grid(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    name = "BicubicEdgeRegression"
    folder = tmp_path / name
    folder.mkdir()
    (folder / f"{name}.info").write_text(
        """SetDesc: Bicubic Q2-edge regression grid
Format: lhagrid1
OrderQCD: 0
NumMembers: 1
Flavors: [-1, 2, 21]
XMin: 0.001
XMax: 0.9
QMin: 1
QMax: 5
ErrorType: none
AlphaS_Type: ipol
AlphaS_Qs: [1, 2, 3, 5]
AlphaS_Vals: [0.5, 0.4, 0.3, 0.2]
"""
    )
    xs = [0.001, 0.02, 0.05, 0.1, 0.2, 0.5, 0.9]
    qs = [1.0, 2.0, 3.0, 5.0]
    data = ["PdfType: central", "Format: lhagrid1", "---",
            " ".join(map(str, xs)), " ".join(map(str, qs)), "-1 2 21"]
    for x in xs:
        t = math.log(x)
        curved = 2.7 + 0.8*t + 0.17*t*t + 0.009*t*t*t
        for q in qs:
            factor = 1 + 0.15*math.log(q*q)
            data.append(" ".join(f"{c*curved*factor:.17g}" for c in (2.0, 2.5, 3.0)))
    data.append("---")
    (folder / f"{name}_0000.dat").write_text("\n".join(data) + "\n")
    return name


def test_bicubic_scalar_matches_pdf_all_at_low_q2(tmp_path, monkeypatch):
    name = _write_curved_grid(tmp_path, monkeypatch)
    pdf = pdfxtmd.GenericCPDFFactory().mkCPDF(name, 0)
    # Index mapping: dbar -> 5, g -> 6, u -> 8.
    flavors = ((pdfxtmd.PartonFlavor.dbar, 5),
               (pdfxtmd.PartonFlavor.g, 6),
               (pdfxtmd.PartonFlavor.u, 8))
    for q2 in (1, 1.01, 4, 4.01, 9, 16):
        for x in (0.03, 0.085, 0.099, 0.1001, 0.15, 0.45):
            values = pdf.pdf_all(x, q2)
            for pid, index in flavors:
                assert pdf.pdf(pid, x, q2) == pytest.approx(values[index], rel=2e-12, abs=1e-12)

    # At QMin, the x-interpolant must not switch to a piecewise linear curve.
    t = math.log(0.1)
    h = 1e-5
    def f(u):
        return pdf.pdf(pdfxtmd.PartonFlavor.dbar, math.exp(u), 1.0)
    left = (f(t) - f(t-h))/h
    right = (f(t+h) - f(t))/h
    assert abs(right-left) < 1e-3*max(1.0, abs(left))
