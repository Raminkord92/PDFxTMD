#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "Common/AllFlavorsShape.h"
#include "Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h"
#include "Interface/IInterpolator.h"

namespace PDFxTMD {

class CNRBicubicInterpolator : public IcPDFInterpolator<CNRBicubicInterpolator, CDefaultLHAPDFFileReader> {
public:
    using Base = IcPDFInterpolator<CNRBicubicInterpolator, CDefaultLHAPDFFileReader>;
    using PartonFlavor = ::PDFxTMD::PartonFlavor;  // Explicitly qualify the type

    void initialize(const IReader<CDefaultLHAPDFFileReader>* reader) {
        grid_data_ = reader->getData();
        validateGrid();
        precomputeDerivatives();
    }

    double interpolate(PartonFlavor flavor, double x, double mu2) const {
        const double logx = std::log(x);
        const double logmu2 = std::log(mu2);

        const size_t ix = findIndex(grid_data_.log_x_vec, logx);
        const size_t imu2 = findIndex(grid_data_.log_mu2_vec, logmu2);

        if (needsLinearFallback(ix, imu2)) {
            return linearInterpolation(flavor, logx, logmu2, ix, imu2);
        }

        return bicubicInterpolation(flavor, logx, logmu2, ix, imu2);
    }

private:
    struct InterpolationData {
        double logx, logmu2;
        double dlogx, dlogmu;
        double tlogx, tlogmu;
        bool q_lower, q_upper;
    };

    DefaultAllFlavorShape grid_data_;
    std::unordered_map<PartonFlavor, std::vector<double>> derivatives_;  // Fixed to 1D storage

    void validateGrid() const {
        if (grid_data_.log_x_vec.size() < 4)
            throw std::runtime_error("Need at least 4 x-points for bicubic interpolation");
        if (grid_data_.log_mu2_vec.size() < 2)
            throw std::runtime_error("Need at least 2 mu2-points for interpolation");
    }

    void precomputeDerivatives() {
        for (const auto& flavor : grid_data_.flavors) {
            const auto& grid = grid_data_.grids.at(flavor);
            auto& deriv = derivatives_[flavor];
            const size_t nx = grid_data_.log_x_vec.size();
            const size_t nmu = grid_data_.log_mu2_vec.size();

            deriv.resize(nx * nmu);

            for (size_t imu = 0; imu < nmu; ++imu) {
                for (size_t ix = 0; ix < nx; ++ix) {
                    if (ix == 0 || ix == nx-1) {
                        deriv[ix*nmu + imu] = 0.0;
                    } else {
                        const double dx = grid_data_.log_x_vec[ix+1] - grid_data_.log_x_vec[ix-1];
                        deriv[ix*nmu + imu] = (grid[(ix+1)*nmu + imu] - grid[(ix-1)*nmu + imu]) / dx;
                    }
                }
            }
        }
    }

    double bicubicInterpolation(PartonFlavor flavor, double logx, double logmu2, 
                               size_t ix, size_t imu2) const {
        const auto& grid = grid_data_.grids.at(flavor);
        const auto& deriv = derivatives_.at(flavor);
        const size_t nmu = grid_data_.log_mu2_vec.size();

        const double x0 = grid_data_.log_x_vec[ix];
        const double x1 = grid_data_.log_x_vec[ix+1];
        const double mu0 = grid_data_.log_mu2_vec[imu2];
        const double mu1 = grid_data_.log_mu2_vec[imu2+1];

        const double tx = (logx - x0)/(x1 - x0);
        const double tmu = (logmu2 - mu0)/(mu1 - mu0);

        // Get 4x4 grid points for bicubic interpolation
        const double f[4][4] = {
            {grid[(ix-1)*nmu + imu2-1], grid[(ix-1)*nmu + imu2], grid[(ix-1)*nmu + imu2+1], grid[(ix-1)*nmu + imu2+2]},
            {grid[ix*nmu + imu2-1],     grid[ix*nmu + imu2],     grid[ix*nmu + imu2+1],     grid[ix*nmu + imu2+2]},
            {grid[(ix+1)*nmu + imu2-1], grid[(ix+1)*nmu + imu2], grid[(ix+1)*nmu + imu2+1], grid[(ix+1)*nmu + imu2+2]},
            {grid[(ix+2)*nmu + imu2-1], grid[(ix+2)*nmu + imu2], grid[(ix+2)*nmu + imu2+1], grid[(ix+2)*nmu + imu2+2]}
        };

        // Perform bicubic interpolation
        double result = 0.0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result += f[i][j] * cubicBasis(i-1, tx) * cubicBasis(j-1, tmu);
            }
        }
        return result;
    }

    static double cubicBasis(int i, double t) {
        switch(i) {
            case -1: return (1 - t)*(1 - t)*(1 - t)/6.0;
            case 0:  return (3*t*t*t - 6*t*t + 4)/6.0;
            case 1:  return (-3*t*t*t + 3*t*t + 3*t + 1)/6.0;
            case 2:  return t*t*t/6.0;
            default: return 0.0;
        }
    }

    double linearInterpolation(PartonFlavor flavor, double logx, double logmu2, 
                              size_t ix, size_t imu2) const {
        const auto& grid = grid_data_.grids.at(flavor);
        const size_t nmu = grid_data_.log_mu2_vec.size();

        ix = std::clamp(ix, size_t(0), grid_data_.log_x_vec.size()-2);
        imu2 = std::clamp(imu2, size_t(0), grid_data_.log_mu2_vec.size()-2);

        const double x0 = grid_data_.log_x_vec[ix];
        const double x1 = grid_data_.log_x_vec[ix+1];
        const double mu0 = grid_data_.log_mu2_vec[imu2];
        const double mu1 = grid_data_.log_mu2_vec[imu2+1];

        const double tx = (logx - x0)/(x1 - x0);
        const double tmu = (logmu2 - mu0)/(mu1 - mu0);

        const double f00 = grid[ix*nmu + imu2];
        const double f01 = grid[ix*nmu + imu2+1];
        const double f10 = grid[(ix+1)*nmu + imu2];
        const double f11 = grid[(ix+1)*nmu + imu2+1];

        return (1 - tx)*(1 - tmu)*f00 + 
               (1 - tx)*tmu*f01 + 
               tx*(1 - tmu)*f10 + 
               tx*tmu*f11;
    }

    bool needsLinearFallback(size_t ix, size_t imu2) const {
        return ix >= grid_data_.log_x_vec.size()-1 || 
               imu2 >= grid_data_.log_mu2_vec.size()-1 ||
               ix < 1 || imu2 < 1 ||  // Added boundary checks
               grid_data_.log_x_vec[ix+1] <= grid_data_.log_x_vec[ix] ||
               grid_data_.log_mu2_vec[imu2+1] <= grid_data_.log_mu2_vec[imu2];
    }

    static size_t findIndex(const std::vector<double>& vec, double value) {
        auto it = std::upper_bound(vec.begin(), vec.end(), value);
        return it == vec.begin() ? 0 : std::distance(vec.begin(), it) - 1;
    }
};

} // namespace PDFxTMD