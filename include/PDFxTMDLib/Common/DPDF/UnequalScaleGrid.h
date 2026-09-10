#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace PDFxTMD
{

struct PairIJ
{
    uint16_t i = 0;
    uint16_t j = 0;
};

struct UnEqualScaleGridData
{
    static constexpr uint32_t kNPartons = 8;
    static constexpr uint32_t kNRegular = 36;
    static constexpr uint32_t kNJjb = 3;
    static constexpr uint32_t kNChannels = kNRegular + kNJjb;

    // xs is the x grid.
    // mus stores t = log(mu^2), not mu.
    std::vector<double> xs;
    std::vector<double> mus;
    std::vector<double> mu2s;

    std::vector<PairIJ> phys_pairs;
    std::vector<int32_t> pair_lut; // nx * nx

    // reg[ch][scale_id * nphys + phys_pair_id]
    // scale_id = k1 * nt + k2
    std::vector<std::vector<double>> reg; // [36][nt * nt * nphys]

    // jjb[0] = s sbar
    // jjb[1] = c cbar
    // jjb[2] = b bbar
    std::vector<std::vector<double>> jjb; // [3][nt * nt * nphys]

    double xmax = 0.0;
    int nmcor = 0;

    uint32_t nmu() const
    {
        if (mus.size() > static_cast<size_t>(std::numeric_limits<uint32_t>::max()))
            throw std::runtime_error("nmu does not fit into uint32_t");
        return static_cast<uint32_t>(mus.size());
    }

    uint32_t nx() const
    {
        if (xs.size() > static_cast<size_t>(std::numeric_limits<uint32_t>::max()))
            throw std::runtime_error("nx does not fit into uint32_t");
        return static_cast<uint32_t>(xs.size());
    }

    uint32_t nxphys() const
    {
        if (phys_pairs.size() > static_cast<size_t>(std::numeric_limits<uint32_t>::max()))
            throw std::runtime_error("nphys does not fit into uint32_t");
        return static_cast<uint32_t>(phys_pairs.size());
    }

    uint32_t ntsq() const
    {
        const uint64_t n = static_cast<uint64_t>(mus.size());
        const uint64_t nn = n * n;
        if (nn > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()))
            throw std::runtime_error("nt^2 does not fit into uint32_t");
        return static_cast<uint32_t>(nn);
    }

    // Keep this only if other PDFxTMD code still calls ntphys().
    // For DPDFB1 it now means the full scale square.
    uint32_t ntphys() const
    {
        return ntsq();
    }

    static int locateBracket(const std::vector<double> &arr, double x)
    {
        const int n = static_cast<int>(arr.size());
        if (n < 2)
            throw std::runtime_error("locateBracket needs at least two grid points");

        if (x == arr.front())
            return 0;

        if (x >= arr.back())
            return n - 2;

        int ju = n;
        int jl = -1;

        while ((ju - jl) > 1)
        {
            const int jm = (ju + jl) / 2;
            if (x >= arr[static_cast<size_t>(jm)])
                jl = jm;
            else
                ju = jm;
        }

        return jl;
    }
};

} // namespace PDFxTMD