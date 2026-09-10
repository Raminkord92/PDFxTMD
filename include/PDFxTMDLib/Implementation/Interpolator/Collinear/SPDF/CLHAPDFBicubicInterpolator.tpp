#include "CLHAPDFBicubicInterpolator.h"
#include <cassert>
#if defined(_M_X64) || defined(_M_IX86)
#include <immintrin.h> // For AVX intrinsics
#endif
#include <vector>
namespace PDFxTMD
{

struct shared_data
{
    double logx, logq2, dlogx_1, dlogq_0, dlogq_1, dlogq_2, tlogq;
    double tlogx, tlogx2, tlogx3, tlogq2, tlogq3;
    bool q2_lower, q2_upper;
    int q2_edge; // 0 = normal, 1 = lower, 2 = upper, 3 = both
};
inline shared_data fillBicubic(const DefaultAllFlavorShape &grid, double x, double q2, size_t ix,
                        size_t iq2)
{
    shared_data shared;
    shared.logx = std::log(x);
    shared.logq2 = std::log(q2);
    shared.q2_lower = (iq2 == 0) || (grid.log_mu2_vec[iq2] == grid.log_mu2_vec[iq2 - 1]);
    shared.q2_upper = (iq2 + 1 == grid.log_mu2_vec.size() - 1) ||
                      (grid.log_mu2_vec[iq2 + 1] == grid.log_mu2_vec[iq2 + 2]);
    shared.q2_edge = shared.q2_lower + (shared.q2_upper << 1); // 0, 1, 2, or 3

    shared.dlogx_1 = grid.dlogx[ix];
    shared.dlogq_1 = grid.dlogq[iq2];
    shared.dlogq_0 = (shared.q2_edge & 1) ? 0.0 : 1.0 / grid.dlogq[iq2 - 1];
    shared.dlogq_2 = (shared.q2_edge & 2) ? 0.0 : 1.0 / grid.dlogq[iq2 + 1];

    shared.tlogx = (shared.logx - grid.log_x_vec[ix]) / shared.dlogx_1;
    shared.tlogx2 = shared.tlogx * shared.tlogx;
    shared.tlogx3 = shared.tlogx2 * shared.tlogx;
    shared.tlogq = (shared.logq2 - grid.log_mu2_vec[iq2]) / shared.dlogq_1;
    shared.tlogq2 = shared.tlogq * shared.tlogq;
    shared.tlogq3 = shared.tlogq2 * shared.tlogq;
    return shared;
}
/// One-dimensional linear interpolation for y(x)
/// @todo Expose for re-use
inline double _interpolateLinear(double x, double xl, double xh, double yl, double yh)
{
    assert(x >= xl);
    assert(xh >= x);
    return yl + (x - xl) / (xh - xl) * (yh - yl);
}

/// One-dimensional cubic interpolation
///
/// @arg t is the fractional distance of the evaluation x into the dx
/// interval.  @arg vl and @arg vh are the function values at the low and
/// high edges of the interval. @arg vdl and @arg vdh are linearly
/// extrapolated value changes from the product of dx and the discrete low-
/// and high-edge derivative estimates.
///
/// @note See Numerical Recipes 3.6:
/// http://www.it.uom.gr/teaching/linearalgebra/NumericalRecipiesInC/c3-6.pdf
///
/// @todo Expose for re-use
/// One-dimensional cubic interpolation
inline double _interpolateCubic(double t, double vl, double vdl, double vh, double vdh)
{
    const double t2 = t * t;
    const double t3 = t * t2;
    const double p0 = (2 * t3 - 3 * t2 + 1) * vl;
    const double m0 = (t3 - 2 * t2 + t) * vdl;
    const double p1 = (-2 * t3 + 3 * t2) * vh;
    const double m1 = (t3 - t2) * vdh;
    return p0 + m0 + p1 + m1;
}
inline double _interpolateCubic(double t, double t2, double t3, double vl, double vdl, double vh,
                                double vdh)
{
    const double p0 = (2 * t3 - 3 * t2 + 1) * vl;
    const double m0 = (t3 - 2 * t2 + t) * vdl;
    const double p1 = (-2 * t3 + 3 * t2) * vh;
    const double m1 = (t3 - t2) * vdh;
    return p0 + m0 + p1 + m1;
}
/// Cubic interpolation using a passed array of coefficients
///
/// @todo Expose for re-use

inline double _interpolateCubic(double t, double t2, double t3, const double *coeffs)
{
    return coeffs[0] * t3 + coeffs[1] * t2 + coeffs[2] * t + coeffs[3];
}
/// Cubic interpolation using a passed array of coefficients
inline double _interpolateCubic(double t, const double *coeffs)
{
    double t2 = t * t;
    double t3 = t2 * t;
    return coeffs[0] * t3 + coeffs[1] * t2 + coeffs[2] * t + coeffs[3];
}

inline double _interpolate(const DefaultAllFlavorShape &grid, size_t ix, size_t iq2,
                           PartonFlavor pid, shared_data &_share)
{
    int flavorId = grid.get_pid(static_cast<int>(pid));
    if (flavorId == -1)
        return 0.0;

#if defined(_M_X64) || defined(_M_IX86)
    // Prefetch coefficients for next iteration
    _mm_prefetch((const char *)&grid.coeff(ix, iq2 + 1, flavorId, 0), _MM_HINT_T0);
#endif
    double vl = _interpolateCubic(_share.tlogx, _share.tlogx2, _share.tlogx3,
                                  &grid.coeff(ix, iq2, flavorId, 0));
    double vh = _interpolateCubic(_share.tlogx, _share.tlogx2, _share.tlogx3,
                                  &grid.coeff(ix, iq2 + 1, flavorId, 0));

    double vdiff = vh - vl;
    double vdl, vdh;
    if (_share.q2_lower)
    {
        vdl = vdiff;
        double vhh = _interpolateCubic(_share.tlogx, _share.tlogx2, _share.tlogx3,
                                       &grid.coeff(ix, iq2 + 2, flavorId, 0));
        vdh = (vdiff + (vhh - vh) * _share.dlogq_1 * _share.dlogq_2) * 0.5;
    }
    else if (_share.q2_upper)
    {
        vdh = vdiff;
        double vll = _interpolateCubic(_share.tlogx, _share.tlogx2, _share.tlogx3,
                                       &grid.coeff(ix, iq2 - 1, flavorId, 0));
        vdl = (vdiff + (vl - vll) * _share.dlogq_1 * _share.dlogq_0) * 0.5;
    }
    else
    {
        double vll = _interpolateCubic(_share.tlogx, _share.tlogx2, _share.tlogx3,
                                       &grid.coeff(ix, iq2 - 1, flavorId, 0));
        vdl = (vdiff + (vl - vll) * _share.dlogq_1 * _share.dlogq_0) * 0.5;
        double vhh = _interpolateCubic(_share.tlogx, _share.tlogx2, _share.tlogx3,
                                       &grid.coeff(ix, iq2 + 2, flavorId, 0));
        vdh = (vdiff + (vhh - vh) * _share.dlogq_1 * _share.dlogq_2) * 0.5;
    }

    return _interpolateCubic(_share.tlogq, _share.tlogq2, _share.tlogq3, vl, vdl, vh, vdh);
}

inline double _interpolateFallback(const DefaultAllFlavorShape &grid, size_t ix, size_t iq2,
                                   PartonFlavor flavor, shared_data &_share)
{
    int flavorId = grid.get_pid(static_cast<int>(flavor));
    if (flavorId == -1)
        return 0.0;

    double f_ql = grid.xf(ix, iq2, flavorId) +
                  (_share.tlogx * (grid.xf(ix + 1, iq2, flavorId) - grid.xf(ix, iq2, flavorId)));
    double f_qh =
        grid.xf(ix, iq2 + 1, flavorId) +
        (_share.tlogx * (grid.xf(ix + 1, iq2 + 1, flavorId) - grid.xf(ix, iq2 + 1, flavorId)));
    return f_ql + (_share.tlogq * (f_qh - f_ql));
}
template <class Reader>
void CLHAPDFBicubicInterpolator<Reader>::initialize(const IReader<Reader> *reader)
{
    m_reader = reader;
    m_Shape = reader->getData();
    m_Shape.initializeBicubicCoeficient();
    m_Shape.grids.clear();
}
template <class Reader>
const IReader<Reader> *CLHAPDFBicubicInterpolator<Reader>::getReader() const
{
    return m_reader;
}

inline void _checkGridSize(const DefaultAllFlavorShape &grid, const size_t ix, const size_t iq2)
{
    if (grid.n_xs < 4 || grid.n_mu2s < 2 || ix + 1 >= grid.n_xs || iq2 + 1 >= grid.n_mu2s)
    {
        throw std::runtime_error("Invalid grid size or index out of bounds");
    }
}
template <class Reader>
void CLHAPDFBicubicInterpolator<Reader>::interpolate(double x, double mu2,
                                             std::array<double, 13> &output) const
{
    const size_t ix = indexbelow(x, m_Shape.x_vec);
    const size_t imu2 = indexbelow(mu2, m_Shape.mu2_vec);
    shared_data shared = fillBicubic(m_Shape, x, mu2, ix, imu2);
    _checkGridSize(m_Shape, ix, imu2);
    size_t i = 0;
    for (i = 0; i < DEFAULT_TOTAL_PDFS; i++)
    {
        if (!shared.q2_lower || !shared.q2_upper)
        {
            output[i] = _interpolate(m_Shape, ix, imu2, standardPartonFlavors[i], shared);
        }
        else
        {
            output[i] = _interpolateFallback(m_Shape, ix, imu2, standardPartonFlavors[i], shared);
        }
    }
}
template <class Reader>
double CLHAPDFBicubicInterpolator<Reader>::interpolate(PartonFlavor flavor, double x, double mu2) const
{
    const size_t ix = indexbelow(x, m_Shape.x_vec);
    const size_t imu2 = indexbelow(mu2, m_Shape.mu2_vec);
    _checkGridSize(m_Shape, ix, imu2);
    shared_data shared = fillBicubic(m_Shape, x, mu2, ix, imu2);
    return (shared.q2_edge == 0) ? _interpolate(m_Shape, ix, imu2, flavor, shared)
                                 : _interpolateFallback(m_Shape, ix, imu2, flavor, shared);
}

} // namespace PDFxTMD
