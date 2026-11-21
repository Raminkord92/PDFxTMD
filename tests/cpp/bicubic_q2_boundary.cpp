// Standalone regression: no external PDF grid or Python extension required.
#include "PDFxTMDLib/Implementation/Interpolator/Collinear/SPDF/CLHAPDFBicubicInterpolator.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

struct SyntheticReader : PDFxTMD::IReader<SyntheticReader> {
    PDFxTMD::DefaultAllFlavorShape shape;
    PDFxTMD::DefaultAllFlavorShape getData() const { return shape; }
};

void require(bool good, const char* description) {
    if (!good) throw std::runtime_error(description);
}

bool close(double a, double b, double tol = 2e-12) {
    return std::abs(a - b) <= tol * std::max({1.0, std::abs(a), std::abs(b)});
}

SyntheticReader make_reader(const std::vector<double>& q2) {
    SyntheticReader reader;
    auto& shape = reader.shape;
    shape.x_vec = {0.001, 0.02, 0.05, 0.10, 0.20, 0.50, 0.90};
    shape.mu2_vec = q2;
    for (auto pid : PDFxTMD::standardPartonFlavors)
        shape._pids.push_back(pid == PDFxTMD::PartonFlavor::gNS ? 21 : static_cast<int>(pid));
    shape.initPidLookup();
    shape.finalizeXP2();
    shape.grids_flat.assign(shape.n_xs * shape.n_mu2s * shape.n_flavors, 0.0);
    for (size_t ix = 0; ix < shape.n_xs; ++ix) {
        double t = std::log(shape.x_vec[ix]);
        double poly = 2.7 + 0.8*t + 0.17*t*t + 0.009*t*t*t;
        for (size_t iq = 0; iq < shape.n_mu2s; ++iq) {
            for (size_t id = 0; id < shape.n_flavors; ++id) {
                shape.grids_flat[ix*shape.stride_ix + iq*shape.stride_iq2 + id] =
                    (2.0 + 0.2*static_cast<double>(id)) * poly *
                    (1.0 + 0.15*std::log(q2[iq]));
            }
        }
    }
    return reader;
}

void test_grid(const std::vector<double>& q2, const char* label) {
    SyntheticReader reader = make_reader(q2);
    PDFxTMD::CLHAPDFBicubicInterpolator<SyntheticReader> interp;
    interp.initialize(&reader);

    size_t compared = 0;
    for (double mu2 : q2) {
        for (double x : {0.025, 0.08, 0.099, 0.1001, 0.15, 0.45}) {
            std::array<double, DEFAULT_TOTAL_PDFS> values{};
            interp.interpolate(x, mu2, values);
            for (size_t id = 0; id < PDFxTMD::standardPartonFlavors.size(); ++id) {
                const double scalar = interp.interpolate(PDFxTMD::standardPartonFlavors[id], x, mu2);
                require(std::isfinite(scalar) && std::isfinite(values[id]), "non-finite value");
                if (!close(scalar, values[id])) {
                    std::cerr << "Scalar/vector mismatch: " << label << " x=" << x
                              << " q2=" << mu2 << " id=" << id
                              << " scalar=" << scalar << " all=" << values[id] << '\n';
                    throw std::runtime_error("scalar/vector mismatch");
                }
                ++compared;
            }
        }
    }

    // The polynomial precomputation uses the same derivative at each log-x knot.
    // Both one-sided first derivatives of the returned PDF must therefore agree.
    // In the original implementation the scalar path used linear log-x
    // interpolation at Q_min, resulting in a finite slope jump at x=0.1.
    const double x0 = 0.1;
    const double h = 1e-5;
    const auto xf_log = [&](double logx) {
        return interp.interpolate(PDFxTMD::PartonFlavor::dbar, std::exp(logx), q2.front());
    };
    const double t0 = std::log(x0);
    const double left = (xf_log(t0) - xf_log(t0-h))/h;
    const double right = (xf_log(t0+h) - xf_log(t0))/h;
    const double jump = right-left;
    require(std::abs(jump) < 1e-3*std::max(1.0, std::abs(left)),
            "x-slope jump at a Q_min grid knot");
    std::cout << "PASS " << label << ": " << compared
              << " scalar/all-flavor comparisons, x=0.1 slope jump=" << jump << '\n';
}

} // namespace

int main() {
    try {
        test_grid({1, 4, 9, 25}, "regular Q2 grid");
        test_grid({1, 9}, "two-knot Q2 grid");
        test_grid({1, 4, 4, 9, 25}, "repeated Q2 knot");
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
