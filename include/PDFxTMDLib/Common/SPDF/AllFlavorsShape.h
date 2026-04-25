#pragma once
#include "PDFxTMDLib/Common/PartonUtils.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include "PDFxTMDLib/Common/StringUtils.h"

namespace PDFxTMD
{

inline bool isBlockSeparator(std::string &line)
{
    trim(line);
    return  line == "---";
}

inline bool isComment(const std::string &line)
{
    return !line.empty() && line[0] == '#';
}

int findPidInPids(int pid, const std::vector<int> &pids);

struct DefaultAllFlavorShape
{
    DefaultAllFlavorShape()
    {
        grids_flat.reserve(1024);
        _lookup.fill(-1);
    }

    alignas(64) std::vector<double> log_x_vec;
    alignas(64) std::vector<double> log_mu2_vec;
    alignas(64) std::vector<double> x_vec;
    alignas(64) std::vector<double> mu2_vec;
    size_t n_xs = 0;
    size_t n_mu2s = 0;
    size_t n_flavors = 0;
    alignas(64) std::vector<double> dlogx; // Differences between consecutive log_x_vec
    alignas(64) std::vector<double> dlogq; // Differences between consecutive log_mu2_vec
    alignas(64) std::vector<double> coefficients_flat;

    // Precomputed strides for fast indexing
    size_t stride_ix = 0;
    size_t stride_iq2 = 0;

    std::vector<int> _shape;
    const double &coeff(int ix, int iq2, int flavorId, int in) const;
    void initializeBicubicCoeficient();
    void finalizeXP2();
    void initPidLookup();
    std::unordered_map<PartonFlavor, std::vector<double>> grids;
    double getGridFromMap(PartonFlavor flavor, int ix, int iq2) const;
    // Removed grids map; use grids_flat only for speed
    alignas(64) std::vector<double> grids_flat;

    inline double xf(int ix, int iq2, int flavorId) const
    {
        // Use precomputed strides to avoid multiplications
        return grids_flat[ix * stride_ix + iq2 * stride_iq2 + flavorId];
    }

    inline int get_pid(int id) const
    {
        // hardcoded lookup table for particle ids
        // -6,...,-1,21/0,1,...,6,22
        // if id outside of this range, search in list of ids
        if (-6 <= id && id <= 6)
            return _lookup[id + 6];
        else if (id == 21)
            return _lookup[0 + 6];
        else if (id == 22)
            return _lookup[13];
        else
            return findPidInPids(id, _pids);
    }

    std::vector<int> _pids;

  private:
    double _ddxBicubic(size_t ix, size_t iq2, int flavorId);
    void _computePolynomialCoefficients();
    std::array<int, 29> _lookup; // Fixed-size lookup for -6 to 22
};

struct DefaultAllFlavorTMDShape : DefaultAllFlavorShape
{
    DefaultAllFlavorTMDShape() = default;
    alignas(64) std::vector<double> log_kt2_vec;
    alignas(64) std::vector<double> kt2_vec;
    void finalizeXKt2P2();
};

} // namespace PDFxTMD