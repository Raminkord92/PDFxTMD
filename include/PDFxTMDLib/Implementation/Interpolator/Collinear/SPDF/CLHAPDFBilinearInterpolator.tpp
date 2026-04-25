#include "CLHAPDFBilinearInterpolator.h"
#include "PDFxTMDLib/external/mlinterp/mlinterp.hpp"
#include <cassert>
#include <cmath>
#include <stdexcept>

#include "PDFxTMDLib/Common/Exception.h"

// taken from lhapdf library!
namespace PDFxTMD
{

namespace
{ // Unnamed namespace
struct shared_dataLinear
{
    // manual cache some values
    double logx;
    double logq2;
    double logx0;
    double logx1;
};

inline shared_dataLinear fill(const DefaultAllFlavorShape &grid, double x, double q2, size_t ix)
{
    // manual cache some values
    shared_dataLinear share;
    share.logq2 = log(q2);
    share.logx = log(x);
    share.logx0 = grid.log_x_vec[ix];
    share.logx1 = grid.log_x_vec[ix + 1];
    return share;
}

// One-dimensional linear interpolation for y(x)
inline double _interpolateLinear(double x, double xl, double xh, double yl, double yh)
{
    assert(x >= xl);
    assert(xh >= x);
    return yl + (x - xl) / (xh - xl) * (yh - yl);
}

inline double _interpolate(const DefaultAllFlavorShape &grid, size_t ix, size_t iq2,
                           PartonFlavor flavor, const shared_dataLinear& _share)
{
    const double f_ql = _interpolateLinear(_share.logx, _share.logx0, _share.logx1,
                                           grid.xf(ix, iq2, flavor), grid.xf(ix + 1, iq2, flavor));
    const double f_qh =
        _interpolateLinear(_share.logx, _share.logx0, _share.logx1, grid.xf(ix, iq2 + 1, flavor),
                           grid.xf(ix + 1, iq2 + 1, flavor));
    // Then interpolate in Q2, using the x-ipol results as anchor points
    return _interpolateLinear(_share.logq2, grid.log_mu2_vec.at(iq2), grid.log_mu2_vec.at(iq2 + 1),
                              f_ql, f_qh);
}
} // namespace

template <class ReaderType>
void CLHAPDFBilinearInterpolator<ReaderType>::initialize(const IReader<ReaderType> *reader)
{
    m_reader = reader;
    m_Shape = reader->getData();
}
template<class ReaderType>
const IReader<ReaderType> *CLHAPDFBilinearInterpolator<ReaderType>::getReader() const
{
    return m_reader;
}
template<class ReaderType>
void CLHAPDFBilinearInterpolator<ReaderType>::interpolate(double x, double mu2,
                                              std::array<double, DEFAULT_TOTAL_PDFS> &output) const
{
    const size_t ix = indexbelow(x, m_Shape.x_vec);
    const size_t imu2 = indexbelow(mu2, m_Shape.mu2_vec);
    shared_dataLinear shared = fill(m_Shape, x, mu2, ix);
    for (int i = 0; i < DEFAULT_TOTAL_PDFS; i++)
    {
        output[i] = _interpolate(m_Shape, ix, imu2, standardPartonFlavors[i], shared);
    }
}
template<class ReaderType>
double CLHAPDFBilinearInterpolator<ReaderType>::interpolate(PartonFlavor flavor, double x, double mu2) const
{
    const size_t ix = indexbelow(x, m_Shape.x_vec);
    const size_t imu2 = indexbelow(mu2, m_Shape.mu2_vec);
    shared_dataLinear shared = fill(m_Shape, x, mu2, ix);
    return _interpolate(m_Shape, ix, imu2, flavor, shared);
}
} // namespace PDFxTMD
