# PDFxTMDLib Python API

PDFxTMDLib provides Python bindings for fast and unified access to collinear PDFs (CPDFs), TMDs, and QCD coupling calculations. This README covers installation and usage for Python users only.

## Installation

Install the package from PyPI:

```bash
pip install pdfxtmd
```

## Quick Start

```python
import pdfxtmd

# Create a CPDF factory and CPDF object
cpdf_factory = pdfxtmd.GenericCPDFFactory()
cpdf = cpdf_factory.mkCPDF("CT18NLO", 0)

# Evaluate a single flavor
x = 0.01
mu2 = 100
print("Up quark PDF:", cpdf.pdf(pdfxtmd.PartonFlavor.u, x, mu2))
print("Gluon PDF:", cpdf.pdf(pdfxtmd.PartonFlavor.g, x, mu2))

# Evaluate all flavors at once (output is modified in-place)
all_flavors = [0.0] * 13  # Must be length 13
cpdf.pdf(x, mu2, all_flavors)
print("All flavors (CPDF):", all_flavors)

# Create a TMD factory and TMD object
tmd_factory = pdfxtmd.GenericTMDFactory()
tmd = tmd_factory.mkTMD("PB-LO-HERAI+II-2020-set2", 0)

kt2 = 10
print("Gluon TMD:", tmd.tmd(pdfxtmd.PartonFlavor.g, x, kt2, mu2))
print("Up quark TMD:", tmd.tmd(pdfxtmd.PartonFlavor.u, x, kt2, mu2))
```

## QCD Coupling Example

```python
coupling_factory = pdfxtmd.CouplingFactory()
coupling = coupling_factory.mkCoupling("CT18NLO")
for scale in [10, 100, 1000, 10000]:
    print(f"Alpha_s at mu2={scale}:", coupling.AlphaQCDMu2(scale))
```

## Error Handling

The Python API raises exceptions for invalid input:

```python
try:
    cpdf.pdf(pdfxtmd.PartonFlavor.u, -0.1, mu2)  # Invalid x
except Exception as e:
    print("Expected error for invalid x in CPDF:", e)

try:
    tmd.tmd(pdfxtmd.PartonFlavor.g, x, -5, mu2)  # Invalid kt2
except Exception as e:
    print("Expected error for invalid kt2 in TMD:", e)

try:
    coupling.AlphaQCDMu2(-1)  # Invalid mu2
except Exception as e:
    print("Expected error for invalid mu2 in Coupling:", e)
```

## Enumerating Parton Flavors

```python
print("All PartonFlavor enum values:")
for name, flavor in pdfxtmd.PartonFlavor.__members__.items():
    print(f"  {name}: {flavor.value}")
```

## Full Tutorial

See [examples/python_tutorial.ipynb](examples/python_tutorial.ipynb) and [examples/python_tutorial.py](examples/python_tutorial.py) for more details and advanced usage.

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.

## Contact

For questions or contributions, contact [raminkord92@gmail.com](mailto:raminkord92@gmail.com).