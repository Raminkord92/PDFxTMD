#include "PDFxTMDLib/Common/SPDF/AllFlavorsShape.h"

namespace PDFxTMD
{

void DefaultAllFlavorShape::finalizeXP2()
{
    log_mu2_vec.clear();
    log_x_vec.clear();
    log_mu2_vec.reserve(mu2_vec.size());
    log_x_vec.reserve(x_vec.size());

    for (double mu2 : mu2_vec)
    {
        log_mu2_vec.push_back(std::log(mu2));
    }
    for (double x : x_vec)
    {
        log_x_vec.push_back(std::log(x));
    }
    n_xs = log_x_vec.size();
    n_mu2s = log_mu2_vec.size();
    n_flavors = _pids.size();

    stride_iq2 = n_flavors;
    stride_ix = n_mu2s * n_flavors;
    grids_flat.reserve(n_xs * n_mu2s * n_flavors);
}

double DefaultAllFlavorShape::_ddxBicubic(size_t ix, size_t iq2, int flavorId)
{
    const size_t nxknots = n_xs;
    double del1 = (ix == 0) ? 0 : log_x_vec[ix] - log_x_vec[ix - 1];
    double del2 = (ix == nxknots - 1) ? 0 : log_x_vec[ix + 1] - log_x_vec[ix];

    // Use precomputed differences where possible
    if (ix > 0 && ix < nxknots - 1)
    {
        const double lddx = (xf(ix, iq2, flavorId) - xf(ix - 1, iq2, flavorId)) / del1;
        const double rddx = (xf(ix + 1, iq2, flavorId) - xf(ix, iq2, flavorId)) / del2;
        return (lddx + rddx) * 0.5; // Avoid division by 2
    }
    if (ix == 0)
    {
        return (xf(ix + 1, iq2, flavorId) - xf(ix, iq2, flavorId)) / del2;
    }
    if (ix == nxknots - 1)
    {
        return (xf(ix, iq2, flavorId) - xf(ix - 1, iq2, flavorId)) / del1;
    }
    throw std::runtime_error("Invalid index in _ddxBicubic");
}
double DefaultAllFlavorShape::getGridFromMap(PartonFlavor flavor, int ix, int iq2) const
{
    return grids.at(flavor)[ix * n_mu2s + iq2];
}
int findPidInPids(int pid, const std::vector<int> &pids)
{
    // Use std::find with hint for small vectors
    auto it = std::find(pids.begin(), pids.end(), pid);
    return (it == pids.end()) ? -1 : static_cast<int>(std::distance(pids.begin(), it));
}

void DefaultAllFlavorShape::initPidLookup()
{
    if (_pids.empty())
    {
#if defined(ENABLE_LOG) && (ENABLE_LOG == 1)
        std::cerr << "Error: PID list empty during lookup initialization" << std::endl;
#endif
        throw std::runtime_error("Empty PID list");
    }

    // Fill lookup table for -6 to 22
    for (int i = -6; i <= 6; i++)
    {
        _lookup[i + 6] = findPidInPids(i, _pids);
    }
    _lookup[0 + 6] = findPidInPids(21, _pids);  // Gluon (21) mapped to index 6
    _lookup[13 + 6] = findPidInPids(22, _pids); // Photon (22) mapped to index 19
}

void DefaultAllFlavorShape::_computePolynomialCoefficients()
{
    const size_t nxknots = n_xs;
    coefficients_flat.resize((nxknots - 1) * n_mu2s * n_flavors * 4);

    // Precompute strides for coefficients_flat
    size_t stride_coeff_ix = n_mu2s * n_flavors * 4;
    size_t stride_coeff_iq2 = n_flavors * 4;
    size_t stride_coeff_id = 4;

    for (size_t ix = 0; ix < nxknots - 1; ++ix)
    {
        double dlogx_ = log_x_vec[ix + 1] - log_x_vec[ix];
        for (size_t iq2 = 0; iq2 < n_mu2s; ++iq2)
        {
            for (size_t id = 0; id < n_flavors; ++id)
            {
                double VL = xf(ix, iq2, id);
                double VH = xf(ix + 1, iq2, id);
                double VDL = _ddxBicubic(ix, iq2, id) * dlogx_;
                double VDH = _ddxBicubic(ix + 1, iq2, id) * dlogx_;

                // Polynomial coefficients
                double a = VDH + VDL - 2 * VH + 2 * VL;
                double b = 3 * VH - 3 * VL - 2 * VDL - VDH;
                double c = VDL;
                double d = VL;

                size_t base = ix * stride_coeff_ix + iq2 * stride_coeff_iq2 + id * stride_coeff_id;
                coefficients_flat[base + 0] = a;
                coefficients_flat[base + 1] = b;
                coefficients_flat[base + 2] = c;
                coefficients_flat[base + 3] = d;
            }
        }
    }
}

void DefaultAllFlavorShape::initializeBicubicCoeficient()
{
    _shape = {static_cast<int>(n_xs), static_cast<int>(n_mu2s), static_cast<int>(n_flavors)};
    _computePolynomialCoefficients();

    dlogx.resize(n_xs - 1);
    for (size_t i = 0; i < n_xs - 1; ++i)
    {
        dlogx[i] = log_x_vec[i + 1] - log_x_vec[i];
    }
    dlogq.resize(n_mu2s - 1);
    for (size_t i = 0; i < n_mu2s - 1; ++i)
    {
        dlogq[i] = log_mu2_vec[i + 1] - log_mu2_vec[i];
    }
}

const double &DefaultAllFlavorShape::coeff(int ix, int iq2, int flavorId, int in) const
{
    // Use precomputed shape strides
    return coefficients_flat[ix * n_mu2s * n_flavors * 4 + iq2 * n_flavors * 4 + flavorId * 4 + in];
}

void DefaultAllFlavorTMDShape::finalizeXKt2P2()
{
    finalizeXP2();
    log_kt2_vec.clear();
    log_kt2_vec.reserve(kt2_vec.size());

    for (double kt2 : kt2_vec)
    {
        log_kt2_vec.push_back(std::log(kt2));
    }
}

} // namespace PDFxTMD