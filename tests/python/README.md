# Python binding integration tests

From the repository root, use a virtual environment and build the native extension:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install setuptools wheel pybind11 pytest numpy
# Linux DPD dependencies: libzstd-dev and libdnnl-dev.
PDFXTMD_ENABLE_DPD=ON CMAKE_BUILD_PARALLEL_LEVEL=2 python setup.py build_ext --inplace
PYTHONPATH="$PWD/src" python -c 'import pdfxtmd; print(pdfxtmd.__file__)'
PYTHONPATH="$PWD/src" python -m pytest tests/python -v
```

The printed module path must point to the newly built extension under `src/`.
Use `PDFXTMD_ENABLE_DPD=OFF` to check the supported core-only configuration.
Use a separate `--build-temp build/core` or `--build-temp build/dpd` when
switching configurations, to avoid reusing a previous build's output.

The default tests create tiny collinear and TMD grids in pytest's temporary
directory and read them using the actual native readers and interpolators.
They check known symmetric Hessian errors, confidence-level scaling,
correlations, factory agreement, metadata, indexing, and member ownership.
Garbage-collection tests include both weak-reference checks and a subprocess
that repeatedly evaluates members returned from temporary sets.

External physics grids are opt-in and never downloaded by tests:

```sh
PDFXTMD_TEST_CPDF=CT18NLO PDFXTMD_TEST_TMD=PB-LO-HERAI+II-2020-set2 \
  PYTHONPATH="$PWD/src" python -m pytest tests/python -v
```

Install the named sets in a configured PDFxTMDLib data directory, or run from a
directory containing the set folders and pass an absolute test directory.
Unspecified external-grid tests are reported as skipped; configured grids that
cannot load fail the tests. Generated grids test bindings independently of
large data downloads, but do not replace validation with external physics data.

The standalone C++ smoke test compares the same set operations with the native
factory API. After building the static target, compile it on Linux, adjusting
the archive path to your CMake build directory:

```sh
c++ -std=c++17 -O2 -Iinclude tests/cpp/pdfset_smoke.cpp \
  build/temp.linux-x86_64-cpython-312/pdfxtmd/libPDFxTMDLib.a \
  -lzstd -ldnnl -fopenmp -o build/pdfset-smoke
build/pdfset-smoke CPDF CT18NLO
build/pdfset-smoke TMD PB-LO-HERAI+II-2020-set2
```

The native test requires a set with error members and a nonzero uncertainty.
For a DPD-disabled static archive, omit `-lzstd -ldnnl -fopenmp`.
