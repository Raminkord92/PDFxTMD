# PDFxTMDLib Python API

**PDFxTMDLib** provides a powerful and easy-to-use Python interface for high-performance Parton Distribution Function (PDF) calculations. It offers unified access to collinear PDFs (cPDFs), Transverse Momentum-Dependent PDFs (TMDs), double parton distributions (DPDs), uncertainty analysis, and QCD coupling calculations.

This guide covers the installation and usage of the Python bindings.

-----

## Installation

You can install the package directly from PyPI:

```bash
pip install pdfxtmd
```

-----

## High-Level API: `PDFSet`

The `PDFSet` interface is the recommended way to work with PDF sets. It handles the entire collection of PDF members (central value and error sets) and provides a simple API for calculating **uncertainties** and **correlations**.

### 1\. Collinear PDF (cPDF) Uncertainty

```python
import pdfxtmd

# 1. Initialize a CPDFSet for a given PDF set name
cpdf_set = pdfxtmd.CPDFSet("CT18NLO")
print(f"Loaded CPDF Set: CT18NLO with {len(cpdf_set)} members.")

# 2. Define kinematics
x = 0.01
mu2 = 100

# 3. Get the central value PDF from member 0
central_cpdf = cpdf_set[0]
up_pdf_central = central_cpdf.pdf(pdfxtmd.PartonFlavor.u, x, mu2)
print(f"Central Up Quark PDF: {up_pdf_central:.6f}")

# 4. Calculate the PDF uncertainty (the result is a PDFUncertainty object)
uncertainty = cpdf_set.Uncertainty(pdfxtmd.PartonFlavor.u, x, mu2)
print(f"Uncertainty: central={uncertainty.central:.6f}, +{uncertainty.errplus:.6f}, -{uncertainty.errminus:.6f}")

# 5. Calculate the correlation between two different partons
correlation = cpdf_set.Correlation(
    pdfxtmd.PartonFlavor.u, x, mu2,  # PDF A
    pdfxtmd.PartonFlavor.d, x, mu2   # PDF B
)
print(f"Correlation between u and d quarks: {correlation:.4f}")
```

### 2\. TMD Uncertainty

The interface for TMDs is analogous. Just use `TMDSet` and include the transverse momentum `kt2`.

```python
import pdfxtmd

# 1. Initialize a TMDSet
tmd_set = pdfxtmd.TMDSet("PB-NLO-HERAI+II-2018-set2")
print(f"\nLoaded TMD Set: {tmd_set.getStdPDFInfo().SetDesc} with {len(tmd_set)} members.")

# 2. Define kinematics
x = 0.01
mu2 = 100
kt2 = 10

# 3. Calculate the TMD uncertainty
uncertainty_tmd = tmd_set.Uncertainty(pdfxtmd.PartonFlavor.g, x, kt2, mu2)
print(f"Gluon TMD Uncertainty: central={uncertainty_tmd.central:.6f}, +{uncertainty_tmd.errplus:.6f}, -{uncertainty_tmd.errminus:.6f}")
```

-----

## Low-Level API: Factories

For applications where you only need to evaluate a **single PDF member** and do not require uncertainty analysis, the factory interface offers a more direct approach.

```python
import pdfxtmd

x = 0.01
mu2 = 100
kt2 = 10

# --- Collinear PDFs (cPDF) using Factory ---
cpdf_factory = pdfxtmd.GenericCPDFFactory()
cpdf = cpdf_factory.mkCPDF("CT18NLO", 0) # Get member 0

# Evaluate a single flavor
up_pdf = cpdf.pdf(pdfxtmd.PartonFlavor.u, x, mu2)
print(f"CPDF (Up Quark) from Factory: {up_pdf:.6f}")

# Evaluate all flavors at once (returns a list)
all_flavors_cpdf = cpdf.pdf_all(x, mu2)
print(f"All CPDF Flavors from Factory: {all_flavors_cpdf}")


# --- TMDs using Factory ---
tmd_factory = pdfxtmd.GenericTMDFactory()
tmd = tmd_factory.mkTMD("PB-NLO-HERAI+II-2018-set2", 0) # Get member 0

# Evaluate a single flavor
gluon_tmd = tmd.tmd(pdfxtmd.PartonFlavor.g, x, kt2, mu2)
print(f"\nTMD (Gluon) from Factory: {gluon_tmd:.6f}")

# Evaluate all TMD flavors at once (returns a list)
all_flavors_tmd = tmd.tmd_all(x, kt2, mu2)
print(f"All TMD Flavors from Factory: {all_flavors_tmd}")
```

-----

## Double Parton Distributions (DPDs)

DPDs are available when the package is built with DPD support. The same Python
API handles dense `PDFxTMD-DPDB1` and hybrid `PDFxTMD-DPDH1` sets.

```python
import numpy as np
import pdfxtmd

print("DPD support:", pdfxtmd.__has_dpd__)

dpd = pdfxtmd.GenericCDPDFactory().mkCDPD(
    "MSTW2008lo68cl_GSDPDF_PDFxTMD", 0
)

value = dpd.dpd(
    pdfxtmd.PartonFlavor.g,
    pdfxtmd.PartonFlavor.g,
    1e-2, 100.0,
    2e-2, 400.0,
)
print("g-g DPD:", value)

x1 = np.array([1e-3, 1e-2, 5e-2])
x2 = np.array([2e-3, 2e-2, 1e-1])
mu1_2 = np.full_like(x1, 100.0)
mu2_2 = np.full_like(x2, 400.0)

values = dpd.dpd_batch(
    pdfxtmd.PartonFlavor.g,
    pdfxtmd.PartonFlavor.g,
    x1, mu1_2, x2, mu2_2,
)
print(values)
```

The NumPy batch overload accepts arrays with matching shapes and releases the
Python GIL during native evaluation.

-----

## QCD Coupling ($\alpha_s$) Calculations

You can calculate the strong coupling constant $\alpha_s$ in two ways.

```python
import pdfxtmd

cpdf_set = pdfxtmd.CPDFSet("CT18NLO")

# Method A: Directly from the PDFSet object (Recommended)
print("--- Method A: From PDFSet ---")
alpha_s_from_set = cpdf_set.alphasQ2(10000)
print(f"Alpha_s at mu2=10000: {alpha_s_from_set:.5f}")


# Method B: Using the low-level CouplingFactory
print("\n--- Method B: From CouplingFactory ---")
coupling_factory = pdfxtmd.CouplingFactory()
coupling = coupling_factory.mkCoupling("CT18NLO")
alpha_s_from_factory = coupling.AlphaQCDMu2(10000)
print(f"Alpha_s at mu2=10000: {alpha_s_from_factory:.5f}")
```

-----

## Complete Example: Plotting PDFs with Uncertainties

This example demonstrates a complete workflow: loading PDF/TMD sets, calculating values and uncertainties over a range of *x*, and plotting the results using `matplotlib`.

```python
import pdfxtmd
import numpy as np
import matplotlib.pyplot as plt

# --- Part 1: Collinear PDF (cPDF) ---
cpdf_set = pdfxtmd.CPDFSet("CT18NLO")
print(f"Loaded cPDF set: {cpdf_set.getStdPDFInfo().SetDesc}")

x_values = np.logspace(-4, -1, 100)
mu2 = 1000

# Calculate central values and uncertainties
uncertainties = [cpdf_set.Uncertainty(pdfxtmd.PartonFlavor.g, x, mu2) for x in x_values]
central_pdfs = [u.central for u in uncertainties]
upper_band = [u.central + u.errplus for u in uncertainties]
lower_band = [u.central - u.errminus for u in uncertainties]

# Plot the cPDF
plt.figure(figsize=(10, 6))
plt.plot(x_values, central_pdfs, label='Gluon PDF (CT18NLO)', color='blue')
plt.fill_between(x_values, lower_band, upper_band, color='blue', alpha=0.2, label='Native CL Uncertainty')
plt.xscale('log')
plt.xlabel('$x$')
plt.ylabel('$xg(x, \mu^2)$')
plt.title(f'Gluon PDF with Uncertainty at $\mu^2 = {mu2} \ GeV^2$')
plt.legend()
plt.grid(True, which="both", ls="--")
plt.savefig('gluon_pdf_plot.png')
print("Saved plot to gluon_pdf_plot.png")
plt.close()


# --- Part 2: Transverse Momentum-Dependent PDF (TMD) ---
tmd_set = pdfxtmd.TMDSet("PB-LO-HERAI+II-2020-set2")
print(f"Loaded TMD set: {tmd_set.getStdPDFInfo().SetDesc}")

kt2 = 1

# Calculate central values and uncertainties
tmd_uncertainties = [tmd_set.Uncertainty(pdfxtmd.PartonFlavor.g, x, kt2, mu2) for x in x_values]
central_tmds = [u.central for u in tmd_uncertainties]
tmd_upper_band = [u.central + u.errplus for u in tmd_uncertainties]
tmd_lower_band = [u.central - u.errminus for u in tmd_uncertainties]

# Plot the TMD
plt.figure(figsize=(10, 6))
plt.plot(x_values, central_tmds, label='Gluon TMD (PB-LO-2020)', color='green')
plt.fill_between(x_values, tmd_lower_band, tmd_upper_band, color='green', alpha=0.2, label='Native CL Uncertainty')
plt.xscale('log')
plt.xlabel('$x$')
plt.ylabel('$xg(x, k_t^2, \mu^2)$')
plt.title(f'Gluon TMD with Uncertainty at $k_t^2 = {kt2} \ GeV^2, \mu^2 = {mu2} \ GeV^2$')
plt.legend()
plt.grid(True, which="both", ls="--")
plt.savefig('gluon_tmd_plot.png')
print("Saved plot to gluon_tmd_plot.png")
plt.close()
```

-----

## Additional Information

### Error Handling

The API raises a `RuntimeError` for invalid kinematic inputs.

```python
try:
    cpdf.pdf(pdfxtmd.PartonFlavor.u, -0.1, mu2)  # Invalid x
except RuntimeError as e:
    print(f"Caught expected error for invalid x: {e}")
```

### Enumerating Parton Flavors

You can inspect all available parton flavors and their integer codes.

```python
print("\n--- All PartonFlavor enum values ---")
for name, flavor in pdfxtmd.PartonFlavor.__members__.items():
    print(f"  {name}: {flavor.value}")
```

### Full Code Examples

For more detailed examples, see the full tutorials in the project repository:

  * [python\_tutorial.py](https://www.google.com/search?q=examples/python/python_tutorial.py)
  * [Jupyter Notebook Tutorial (run online)](https://www.google.com/search?q=https://colab.research.google.com/drive/1C_h9oGJJzt5h3m-h6o3lAJ5dl-AVv0j3)

-----

## License

This project is licensed under the GNU General Public License v3.0. See the `LICENSE` file for details.

## Contact

For questions or contributions, please contact [raminkord92@gmail.com](mailto:raminkord92@gmail.com).

## High-level set API and source builds

`CPDFSet(pdfSetName, alternativeReplicaUncertainty=False)` and
`TMDSet(pdfSetName, alternativeReplicaUncertainty=False)` expose the existing
C++ `PDFSet` specializations. `len(pdf_set)` gives the member count;
`pdf_set[i]` returns an `ICPDF` or `ITMD` with the same evaluation methods as
factory-created members. Indices must satisfy `0 <= i < len(pdf_set)`;
negative and out-of-range indices raise `IndexError`.

A returned member keeps its owning set alive, including with temporary sets:

```python
import gc
central = pdfxtmd.CPDFSet("CT18NLO")[0]
gc.collect()
print(central.pdf(pdfxtmd.PartonFlavor.g, 0.01, 100.0))
```

`Uncertainty(flavor, x, mu2, cl=-1)` for collinear sets and
`Uncertainty(flavor, x, kt2, mu2, cl=-1)` for TMD sets return a
`PDFUncertainty`. The default `cl=-1` uses the set's native confidence level;
pass `cl=68.0` explicitly to request 68 percent. `Uncertainty(values, cl=-1)`
accepts one value per member. Results expose `central`, `errplus`, `errminus`,
`errsymm`, `scale`, the `errplus_pdf`/`errminus_pdf`/`errsymm_pdf` and
`errplus_par`/`errminus_par`/`errsymm_par` components, and `errparts`.
Their definitions and scaling are those of the C++ implementation.

`Correlation(flavorA, xA, mu2A, flavorB, xB, mu2B)` has additional `kt2A`
and `kt2B` arguments before the corresponding scales for TMDs.
`Correlation(valuesA, valuesB)` accepts two full member vectors.
`alphasQ(q)` and `alphasQ2(q2)` evaluate the set's coupling.
`getStdPDFInfo()` and `getPDFErrorInfo()` return metadata snapshots with the
C++ field names. Use `getStdPDFInfo().SetDesc` for the description.
All-flavor evaluation uses `member.pdf_all(x, mu2)` or
`member.tmd_all(x, kt2, mu2)` and returns a list; the existing overloads
accepting an output list are also supported.

To install these bindings from a patched source checkout on Linux:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install --upgrade pip
# Install libzstd-dev and libdnnl-dev for the default DPD-enabled build.
CMAKE_BUILD_PARALLEL_LEVEL=2 python -m pip install .
# Or build without DPD dependencies:
# PDFXTMD_ENABLE_DPD=OFF CMAKE_BUILD_PARALLEL_LEVEL=2 python -m pip install .
```

PDF grid data must be installed separately. For development builds and tests
against the freshly compiled extension, see `tests/python/README.md`.
