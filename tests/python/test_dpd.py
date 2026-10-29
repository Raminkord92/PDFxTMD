"""Regression smoke test of the existing dense DPD scalar and batch API."""
import ctypes
import ctypes.util
import math
import struct

import numpy as np
import pdfxtmd as pdf
import pytest


def test_dpd_factory_scalar_and_batch(tmp_path, monkeypatch):
    if not pdf.__has_dpd__:
        with pytest.raises(ValueError, match="disabled"):
            pdf.GenericCDPDFactory().mkCDPD("BindingTestDPD", 0)
        return
    monkeypatch.chdir(tmp_path)
    name = "BindingTestDPD"
    folder = tmp_path / name
    folder.mkdir()
    (folder / (name + ".info")).write_text("""SetDesc: Binding DPD test
Format: PDFxTMD-DPDB1
OrderQCD: 1
NumMembers: 1
Flavors: [1, 2, 21]
XMin: 0.01
XMax: 0.5
QMin: 2
QMax: 20
""")
    # DPDB1: full scale square, physical x pairs, 36 regular + 3 jjb channels.
    xs = [0.01, 0.05, 0.1, 0.3, 0.5]
    ts = [math.log(q * q) for q in (2, 5, 10, 20)]
    pairs = [(i, j) for i, x in enumerate(xs) for j, y in enumerate(xs) if x + y < 1]
    count = 39 * len(ts)**2 * len(pairs)
    raw = struct.pack("=" + "f" * count, *([2.0] * count))
    library = ctypes.util.find_library("zstd")
    assert library, "A DPD-enabled build requires the Zstandard runtime"
    zstd = ctypes.CDLL(library)
    zstd.ZSTD_compressBound.argtypes = [ctypes.c_size_t]
    zstd.ZSTD_compressBound.restype = ctypes.c_size_t
    zstd.ZSTD_compress.argtypes = [ctypes.c_void_p, ctypes.c_size_t,
                                  ctypes.c_void_p, ctypes.c_size_t, ctypes.c_int]
    zstd.ZSTD_compress.restype = ctypes.c_size_t
    zstd.ZSTD_isError.argtypes = [ctypes.c_size_t]
    zstd.ZSTD_isError.restype = ctypes.c_uint
    dest = ctypes.create_string_buffer(zstd.ZSTD_compressBound(len(raw)))
    size = zstd.ZSTD_compress(dest, len(dest), raw, len(raw), 1)
    assert not zstd.ZSTD_isError(size)
    data = b"PDFxTMD-DPDB1".ljust(16, b"\0")
    data += struct.pack("=6I", 1, len(xs), len(ts), 8, len(pairs), 0)
    data += struct.pack("=5d4d", *xs, *ts)
    data += b"".join(struct.pack("=2H", *pair) for pair in pairs)
    data += struct.pack("=Q", size) + dest.raw[:size]
    (folder / (name + "_0000.dat")).write_bytes(data)
    direct = pdf.CollinearDPD(name, 0)
    factory = pdf.GenericCDPDFactory().mkCDPD(name, 0)
    args = (pdf.PartonFlavor.g, pdf.PartonFlavor.g, 0.05, 25.0, 0.1, 100.0)
    value = factory.dpd(*args)
    assert math.isfinite(value) and value > 0
    assert direct.dpd(*args) == pytest.approx(value, rel=1e-12)
    arrays = [np.array([v]) for v in args[2:]]
    assert factory.dpd_batch(*args[:2], *arrays)[0] == pytest.approx(value)
    assert factory.dpd(*args[:2], 0.6, 25.0, 0.5, 100.0) == 0
    with pytest.raises(ValueError):
        factory.dpd(*args[:2], -0.1, 25.0, 0.1, 100.0)
