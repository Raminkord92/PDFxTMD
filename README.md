<img src="./logo.png" alt="PDFxTMDLib Logo" width="300">

[![C++ CI](https://github.com/Raminkord92/PDFxTMD/actions/workflows/cpp-ci.yml/badge.svg)](https://github.com/Raminkord92/PDFxTMD/actions/workflows/cpp-ci.yml)
[![Python Wheels](https://github.com/Raminkord92/PDFxTMD/actions/workflows/python-wheels.yml/badge.svg)](https://github.com/Raminkord92/PDFxTMD/actions/workflows/python-wheels.yml)

**PDFxTMDLib** is a high-performance C++ library for parton distribution functions (PDFs), supporting both collinear PDFs (cPDFs) and transverse momentum-dependent PDFs (TMDs). It is designed with modern C++17 principles for performance and extensibility, and provides interfaces for reading standard PDF grid files (LHAPDF, TMDLib) as well as custom formats.

For a comprehensive understanding of the library's architecture, features, and performance benchmarks, please refer to the full paper available on arXiv: [https://arxiv.org/abs/2412.16680](https://arxiv.org/abs/2412.16680).

---

## Features

- **Extensibility**: Easily create custom PDF implementations through template specialization.
- **Cross-platform**: Full support for Linux, Windows, and macOS.
- **Modern C++**: Built with C++17 for maximum performance and reliability.
- **Wrappers**: Fortran and Python wrappers for easy integration.
- **Double parton distributions (DPDs)**: Dense `PDFxTMD-DPDB1` grids and certified neural/sparse
  `PDFxTMD-DPDH1` deployments through the same `ICDPD` API.

---

## Building and Installation

See the **[detailed installation guide](install.md)** for dependency installation,
platform-specific commands, build options, and Python/Fortran interfaces.

The C++ library requires a C++17 compiler and CMake 3.15+. DPD support
(`ENABLE_DPD=ON` by default) requires Zstandard and oneDNN. For installation
instructions for different operating systems, see the [detailed installation guide](install.md).
- **Linux without vcpkg:** follow the [Debian/Ubuntu, Fedora, or Arch Linux instructions](install.md#linux-dependencies-without-vcpkg).
- **Windows:** vcpkg is recommended; follow the [Windows build instructions](install.md#windows-hybrid-builds-with-onednn-and-openmp) for dependencies and platform limitations.
- **vcpkg on supported platforms:** use the [manifest workflow](install.md#install-dependencies-with-vcpkg).
- **CMake FetchContent:** see [integration into another project](install.md#use-as-a-cmake-dependency-with-fetchcontent).
- **Language interfaces:** see the [Python wrapper](install.md#python-wrapper) and [Fortran interface](install.md#fortran-interface) instructions.

After installing dependencies, a standalone C++ build from the repository root is:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=installed -DENABLE_BUILDING_WRAPPERS=OFF -DENABLE_BUILDING_EXAMPLES=OFF -DENABLE_DPD=ON
cmake --build build --config Release --parallel 2
cmake --install build --config Release
```

This installs into `installed` inside the repository. For vcpkg, add the toolchain
settings from the linked workflow. To build without dense or hybrid DPD support, set
`-DENABLE_DPD=OFF`.

## API documentation

Generate the public API documentation directly into the website checkout:

```bash
cmake -S . -B build -DPDFXTMD_DOXYGEN_HTML_OUTPUT=/path/to/pdfxtmd_website/docs
cmake --build build --target doxygen
```

The `doxygen` target documents the public headers and writes HTML to the configured directory.

-----

## C++ Usage and API

PDFxTMDLib offers a flexible API with both high-level conveniences and low-level control.

### Integration Methods

#### CMake Integration (Recommended)

For projects using CMake, add these lines to your `CMakeLists.txt` file to link against the installed library:

```cmake
find_package(pdfxtmdlib CONFIG REQUIRED)
target_link_libraries(your-target-name PDFxTMD::PDFxTMDLib)
```

Set `CMAKE_PREFIX_PATH` to the absolute installation prefix when configuring your
application. For vcpkg builds, use the same toolchain and declare `zstd` and `onednn` for
DPD builds in your application's manifest. Alternatively, use the
[FetchContent workflow](install.md#use-as-a-cmake-dependency-with-fetchcontent).

#### Direct Compilation

You can also link directly with the compiler.

**Linux/macOS (GCC/Clang):**

```bash
g++ -std=c++17 your_source.cpp -lPDFxTMDLib -o your_executable
```

**Windows (MSVC from Developer Command Prompt):**

```bash
cl your_source.cpp /std:c++17 PDFxTMDLib.lib
```

### High-Level Interface: `PDFSet`

The `PDFSet` template class is the interface for most applications. It abstracts away the complexities of managing PDF set members and provides a unified API for calculations, uncertainty analysis, and metadata access.

#### Collinear PDF (cPDF) Calculations

This example shows how to instantiate a collinear PDF set and evaluate the gluon PDF.

```cpp
#include <PDFxTMDLib/PDFSet.h>
#include <iostream>

int main() {
    // Instantiate a PDFSet for a collinear distribution set
    PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> cpdfSet("MSHT20nlo_as120");

    // Access the central member (index 0)
    auto central_pdf = cpdfSet[0];

    // Define kinematics
    double x = 0.1;
    double mu2 = 10000; // Factorization scale squared

    // Evaluate the gluon PDF
    double gluon_pdf = central_pdf->pdf(PDFxTMD::PartonFlavor::g, x, mu2);

    std::cout << "Gluon PDF at x=" << x << ", mu2=" << mu2 << " GeV2: " << gluon_pdf << std::endl;
    return 0;
}
```

#### Transverse Momentum-Dependent PDF (TMD) Calculations

The process for TMDs is similar, specializing the `PDFSet` with `TMDPDFTag` and including the transverse momentum parameter $k_t^2$.

```cpp
#include <PDFxTMDLib/PDFSet.h>
#include <iostream>

int main() {
    // Instantiate a PDFSet for a TMD distribution set
    PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag> tmdSet("PB-LO-HERAI+II-2020-set2");
    
    // Access the central member (index 0)
    auto central_tmd = tmdSet[0];

    // Define kinematics
    double x = 0.001;
    double kt2 = 10;
    double mu2 = 100;

    // Evaluate the up-quark TMD
    double up_tmd = central_tmd->tmd(PDFxTMD::PartonFlavor::u, x, kt2, mu2);

    std::cout << "Up-quark TMD at x=" << x << ", kt2=" << kt2 << ", mu2=" << mu2 << ": " << up_tmd << std::endl;
    return 0;
}
```

#### Uncertainty and Correlation Analysis

PDFxTMDLib automates uncertainty and correlation calculations based on the PDF set's metadata (Hessian or Monte Carlo).

```cpp
#include <PDFxTMDLib/PDFSet.h>
#include <iostream>

int main() {
    PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> cpdfSet("MSHT20nlo_as120");
    double x = 0.1;
    double mu2 = 10000;

    // Calculate PDF uncertainty at the default confidence level
    PDFxTMD::PDFUncertainty uncertainty = cpdfSet.Uncertainty(PDFxTMD::PartonFlavor::g, x, mu2);
    std::cout << "xg = " << uncertainty.central << " + " << uncertainty.errplus << " - " << uncertainty.errminus << std::endl;

    // Calculate uncertainty at 90% confidence level
    PDFxTMD::PDFUncertainty uncertainty_90 = cpdfSet.Uncertainty(PDFxTMD::PartonFlavor::g, x, mu2, 90.0);
    std::cout << "xg (90% CL) = " << uncertainty_90.central << " + " << uncertainty_90.errplus << " - " << uncertainty_90.errminus << std::endl;

    // Calculate correlation between gluon and up quark
    double correlation = cpdfSet.Correlation(PDFxTMD::PartonFlavor::g, x, mu2, PDFxTMD::PartonFlavor::u, x, mu2);
    std::cout << "Correlation between g and u: " << correlation << std::endl;
    
    return 0;
}
```

### Factory Interfaces for Individual PDF Members

For applications that only need a specific PDF member without uncertainty analysis, factories provide a more direct and efficient approach.

```cpp
#include <PDFxTMDLib/GenericCPDFFactory.h>
#include <PDFxTMDLib/GenericTMDFactory.h>
#include <iostream>

int main() {
    using namespace PDFxTMD;
    double x = 0.001, mu2 = 100, kt2 = 10;

    // Create a single cPDF member using a factory
    auto cpdf_factory = GenericCPDFFactory();
    auto cpdf = cpdf_factory.mkCPDF("MMHT2014lo68cl", 0);
    double gluon_cpdf = cpdf.pdf(PartonFlavor::g, x, mu2);
    std::cout << "Gluon cPDF: " << gluon_cpdf << std::endl;

    // Create a single TMD member using a factory
    auto tmd_factory = GenericTMDFactory();
    auto tmd = tmd_factory.mkTMD("PB-LO-HERAI+II-2020-set2", 0);
    double gluon_tmd = tmd.tmd(PartonFlavor::g, x, kt2, mu2);
    std::cout << "Gluon TMD: " << gluon_tmd << std::endl;

    return 0;
}
```

### QCD Coupling Calculations

The strong coupling constant $\alpha_s(\mu^2)$ can be calculated either from a `PDFSet` instance or using a `CouplingFactory`.

```cpp
#include <PDFxTMDLib/PDFSet.h>
#include <PDFxTMDLib/CouplingFactory.h>
#include <iostream>

int main() {
    using namespace PDFxTMD;
    double mu2 = 10000;

    // Method 1: Using PDFSet
    PDFSet<CollinearPDFTag> cpdfSet("MSHT20nlo_as120");
    double alpha_s_from_set = cpdfSet.alphasQ2(mu2);
    std::cout << "alpha_s from PDFSet: " << alpha_s_from_set << std::endl;

    // Method 2: Using CouplingFactory
    auto couplingFactory = CouplingFactory();
    auto coupling = couplingFactory.mkCoupling("MMHT2014lo68cl");
    double alpha_s_from_factory = coupling.AlphaQCDMu2(mu2);
    std::cout << "alpha_s from Factory: " << alpha_s_from_factory << std::endl;

    return 0;
}
```

### Advanced Usage: Custom Implementations

PDFxTMDLib allows advanced users to construct PDF objects with custom components (e.g., reader, interpolator, extrapolator) by specializing the `GenericPDF` template. Type aliases are also available for convenience.

```cpp
#include <PDFxTMDLib/GenericPDF.h>
#include <PDFxTMDLib/Implementation/Extrapolator/Collinear/CErrExtrapolator.h>
#include <PDFxTMDLib/Implementation/Interpolator/Collinear/CLHAPDFBilinearInterpolator.h>
#include <PDFxTMDLib/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h>
#include <iostream>

int main() {
    using namespace PDFxTMD;

    // Custom PDF implementation with specific components
    using ReaderType = CDefaultLHAPDFFileReader;
    using InterpolatorType = CLHAPDFBilinearInterpolator<ReaderType>;
    using ExtrapolatorType = CErrExtrapolator;
    
    GenericPDF<CollinearPDFTag, ReaderType, InterpolatorType, ExtrapolatorType> 
        custom_cpdf("MMHT2014lo68cl", 0);

    // Using convenient type aliases (equivalent to default implementations)
    CollinearPDF cpdf("MMHT2014lo68cl", 0);
    TMDPDF tmd("PB-LO-HERAI+II-2020-set2", 0);
    
    return 0;
}
```

-----

## Python Wrapper

PDFxTMDLib provides a native Python interface using `pybind11`, exposing all major features of the C++ library.

### Installation

Install the wrapper directly from PyPI:

```bash
pip install pdfxtmd
```

### Quick Python Example

```python
import pdfxtmd

# Create a collinear PDF object using the factory
cpdf_factory = pdfxtmd.GenericCPDFFactory()
cpdf = cpdf_factory.mkCPDF("CT18NLO", 0)
x = 0.01
mu2 = 100
up_pdf = cpdf.pdf(pdfxtmd.PartonFlavor.u, x, mu2)
print(f"Up quark PDF: {up_pdf}")

# Create a TMD PDF object using the factory
tmd_factory = pdfxtmd.GenericTMDFactory()
tmd = tmd_factory.mkTMD("PB-NLO-HERAI+II-2023-set2-qs=0.74", 0)
kt2 = 10
gluon_tmd = tmd.tmd(pdfxtmd.PartonFlavor.g, x, kt2, mu2)
print(f"Gluon TMD: {gluon_tmd}")

# Compute all flavors at once
all_flavors = []
cpdf.pdf(x, mu2, all_flavors)
print(f"All flavors: {all_flavors}")
```

#### QCD Coupling Example

```python
import pdfxtmd

coupling_factory = pdfxtmd.CouplingFactory()
coupling = coupling_factory.mkCoupling("CT18NLO")
for scale in [10, 100, 1000, 10000]:
    print(f"Alpha_s at mu2={scale}: {coupling.AlphaQCDMu2(scale)}")
```

> **Full Python documentation:** \> See [`examples/python/readme-pyversion.md`](https://www.google.com/search?q=examples/python/readme-pyversion.md) for a complete guide and advanced usage.

#### Tutorials

  - **Jupyter Notebook Tutorial (run online):** [Open in Google Colab](https://colab.research.google.com/drive/1C_h9oGJJzt5h3m-h6o3lAJ5dl-AVv0j3#scrollTo=l1iGviJu1eUc)
  - **Local Example:** See [`examples/python/python_tutorial.py`](https://www.google.com/search?q=examples/python/python_tutorial.py)

-----

## Configuration

PDFxTMDLib uses a `config.yaml` file to locate PDF data sets. The library searches for this file in the following locations:

  - **Windows**: `C:\ProgramData\PDFxTMDLib\config.yaml`
  - **Linux/macOS**: `~/.PDFxTMDLib/config.yaml`

Example `config.yaml`:

```yaml
paths:
  - /path/to/my/pdf/sets
  - /another/path/to/pdf/data
```

The `paths` key accepts a list of directories where PDFxTMDLib will search for PDF set data. The current directory and standard system locations are searched by default. In order to download cPDF sets use lhapdf sets available at [link](https://lhapdf.hepforge.org/pdfsets), and to download TMD sets visit the official website of this repository available at [pdfxtmdlib.org](https://pdfxtmdlib.org/downloads/).

-----

## Visualization Tools

For easy visualization of PDFs and TMDs, you can use the [QtPDFxTMDPlotter](https://github.com/Raminkord92/QtPDFxTMDPlotter), a Qt-based graphical tool built on top of PDFxTMDLib.

-----

## Contributing

Contributions are welcome\! If you are interested in contributing to the project, please open an issue or contact [raminkord92@gmail.com](mailto:raminkord92@gmail.com).

-----

## License

This project is licensed under the GPL-3.0 License. For more details, see the [LICENSE](https://www.google.com/search?q=LICENSE) file.

-----

## Contact

For any inquiries, please contact [raminkord92@gmail.com](mailto:raminkord92@gmail.com).

-----
