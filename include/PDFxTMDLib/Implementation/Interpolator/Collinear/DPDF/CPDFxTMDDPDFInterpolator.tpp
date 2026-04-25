#include "CPDFxTMDDPDFInterpolator.h"

#include "PDFxTMDLib/Common/Constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace PDFxTMD
{

namespace
{

double linint(double xs, double z0, double zf)
{
    return zf * xs + (1.0 - xs) * z0;
}

double simpln(double xs, double ys)
{
    return 1.0 - xs - ys;
}

uint32_t triFlavorId(uint32_t f1, uint32_t f2)
{
    if (f2 < f1 || f2 >= 8 || f1 >= 8)
        throw std::runtime_error("invalid triangular flavor ids");

    const uint32_t offset = f1 * 8u - (f1 * (f1 - 1u)) / 2u;
    return offset + (f2 - f1);
}

uint32_t scalePairId(uint32_t k1, uint32_t k2, uint32_t nt)
{
    if (k1 >= nt || k2 >= nt)
        throw std::runtime_error("invalid scale pair");

    return k1 * nt + k2;
}

int pdgToInternal(int pdg)
{
    switch (pdg)
    {
    case 2:
        return 0; // u
    case 1:
        return 1; // d
    case 3:
        return 2; // s
    case 4:
        return 3; // c
    case 5:
        return 4; // b
    case -2:
        return 5; // ubar
    case -1:
        return 6; // dbar
    case 0:
    case 21:
        return 7; // g
    default:
        return -1;
    }
}

double regAt(const UnEqualScaleGridData &data,
             uint32_t ch,
             uint32_t i,
             uint32_t j,
             uint32_t k1,
             uint32_t k2)
{
    const uint32_t nx = data.nx();
    const uint32_t nt = data.nmu();
    const uint32_t nphys = data.nxphys();

    if (i >= nx || j >= nx || k1 >= nt || k2 >= nt ||
        ch >= UnEqualScaleGridData::kNRegular)
    {
        return 0.0;
    }

    const int32_t p =
        data.pair_lut[static_cast<size_t>(i) * nx + j];

    if (p < 0)
        return 0.0;

    const size_t idx =
        static_cast<size_t>(scalePairId(k1, k2, nt)) * nphys +
        static_cast<uint32_t>(p);

    return data.reg[ch][idx];
}

double jjbAt(const UnEqualScaleGridData &data,
             uint32_t ch3,
             uint32_t i,
             uint32_t j,
             uint32_t k1,
             uint32_t k2)
{
    const uint32_t nx = data.nx();
    const uint32_t nt = data.nmu();
    const uint32_t nphys = data.nxphys();

    if (i >= nx || j >= nx || k1 >= nt || k2 >= nt ||
        ch3 >= UnEqualScaleGridData::kNJjb)
    {
        return 0.0;
    }

    const int32_t p =
        data.pair_lut[static_cast<size_t>(i) * nx + j];

    if (p < 0)
        return 0.0;

    const size_t idx =
        static_cast<size_t>(scalePairId(k1, k2, nt)) * nphys +
        static_cast<uint32_t>(p);

    return data.jjb[ch3][idx];
}

int findCornerTriangleGs09(const std::vector<double> &xgrid,
                           double xmax,
                           int nmcor,
                           double xs,
                           double ys)
{
    if (nmcor < 2)
        return -10;

    double ycomp = 0.0;

    for (int i = 1; i < nmcor; ++i)
    {
        ycomp =
            (1.0 - xs) / (1.0 - xmax) *
            xgrid[static_cast<size_t>(i)];

        if (ys < ycomp)
            return i - 1;
    }

    if (ys > ycomp)
        return -10;

    return nmcor - 2;
}

template <typename EdgeSampler>
double cornerPlaneInterpolate(const std::vector<double> &xgrid,
                              double xmax,
                              int nmcor,
                              double xs,
                              double ys,
                              EdgeSampler &&sample)
{
    const int il =
        findCornerTriangleGs09(xgrid, xmax, nmcor, xs, ys);

    if (il == -10)
        return 0.0;

    const std::array<double, 3> xv = {xmax, xmax, 1.0};

    const std::array<double, 3> yv = {
        xgrid[static_cast<size_t>(il)],
        xgrid[static_cast<size_t>(il + 1)],
        0.0};

    const std::array<double, 3> zv = {
        sample(static_cast<uint32_t>(il)),
        sample(static_cast<uint32_t>(il + 1)),
        0.0};

    const double a =
        yv[0] * (zv[1] - zv[2]) +
        yv[1] * (zv[2] - zv[0]) +
        yv[2] * (zv[0] - zv[1]);

    const double b =
        zv[0] * (xv[1] - xv[2]) +
        zv[1] * (xv[2] - xv[0]) +
        zv[2] * (xv[0] - xv[1]);

    const double c =
        xv[0] * (yv[1] - yv[2]) +
        xv[1] * (yv[2] - yv[0]) +
        xv[2] * (yv[0] - yv[1]);

    const double d =
        -xv[0] * (yv[1] * zv[2] - yv[2] * zv[1]) -
        xv[1] * (yv[2] * zv[0] - yv[0] * zv[2]) -
        xv[2] * (yv[0] * zv[1] - yv[1] * zv[0]);

    return (-d - a * xs - b * ys) / c;
}

template <typename AtFunc>
double interpolateCore(const UnEqualScaleGridData &data,
                       AtFunc &&at,
                       double x1,
                       double x2,
                       double t1,
                       double t2)
{
    const uint32_t nx = data.nx();

    if (nx < 2 || data.mus.size() < 2)
        throw std::runtime_error("grid too small for interpolation");

    int m1 = UnEqualScaleGridData::locateBracket(data.mus, t1);
    int m2 = UnEqualScaleGridData::locateBracket(data.mus, t2);

    if (m1 < 0)
        m1 = 0;
    if (m2 < 0)
        m2 = 0;

    if (m1 > static_cast<int>(data.mus.size()) - 2)
        m1 = static_cast<int>(data.mus.size()) - 2;

    if (m2 > static_cast<int>(data.mus.size()) - 2)
        m2 = static_cast<int>(data.mus.size()) - 2;

    const double delt1 =
        data.mus[static_cast<size_t>(m1 + 1)] -
        data.mus[static_cast<size_t>(m1)];

    const double delt2 =
        data.mus[static_cast<size_t>(m2 + 1)] -
        data.mus[static_cast<size_t>(m2)];

    const double frct1 =
        (t1 - data.mus[static_cast<size_t>(m1)]) / delt1;

    const double frct2 =
        (t2 - data.mus[static_cast<size_t>(m2)]) / delt2;

    if (x1 >= data.xmax)
    {
        double ffq[2][2]{};

        for (int i1 = 0; i1 < 2; ++i1)
        {
            for (int i2 = 0; i2 < 2; ++i2)
            {
                ffq[i1][i2] =
                    cornerPlaneInterpolate(
                        data.xs,
                        data.xmax,
                        data.nmcor,
                        x1,
                        x2,
                        [&](uint32_t il) {
                            return at(nx - 1,
                                      il,
                                      static_cast<uint32_t>(m1 + i1),
                                      static_cast<uint32_t>(m2 + i2));
                        });
            }
        }

        double ffq1[2]{};

        for (int i1 = 0; i1 < 2; ++i1)
            ffq1[i1] = linint(frct2, ffq[i1][0], ffq[i1][1]);

        return linint(frct1, ffq1[0], ffq1[1]);
    }

    if (x2 >= data.xmax)
    {
        double ffq[2][2]{};

        for (int i1 = 0; i1 < 2; ++i1)
        {
            for (int i2 = 0; i2 < 2; ++i2)
            {
                ffq[i1][i2] =
                    cornerPlaneInterpolate(
                        data.xs,
                        data.xmax,
                        data.nmcor,
                        x2,
                        x1,
                        [&](uint32_t il) {
                            return at(il,
                                      nx - 1,
                                      static_cast<uint32_t>(m1 + i1),
                                      static_cast<uint32_t>(m2 + i2));
                        });
            }
        }

        double ffq1[2]{};

        for (int i1 = 0; i1 < 2; ++i1)
            ffq1[i1] = linint(frct2, ffq[i1][0], ffq[i1][1]);

        return linint(frct1, ffq1[0], ffq1[1]);
    }

    int n1 = UnEqualScaleGridData::locateBracket(data.xs, x1);
    int n2 = UnEqualScaleGridData::locateBracket(data.xs, x2);

    if (n1 < 0)
        n1 = 0;
    if (n2 < 0)
        n2 = 0;

    if (n1 > static_cast<int>(nx) - 2)
        n1 = static_cast<int>(nx) - 2;

    if (n2 > static_cast<int>(nx) - 2)
        n2 = static_cast<int>(nx) - 2;

    double fflcl[2][2][2][2]{};

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            for (int k1i = 0; k1i < 2; ++k1i)
            {
                for (int k2i = 0; k2i < 2; ++k2i)
                {
                    fflcl[i][j][k1i][k2i] =
                        at(static_cast<uint32_t>(n1 + i),
                           static_cast<uint32_t>(n2 + j),
                           static_cast<uint32_t>(m1 + k1i),
                           static_cast<uint32_t>(m2 + k2i));
                }
            }
        }
    }

    double ffq2[2][2][2]{};

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            for (int k2i = 0; k2i < 2; ++k2i)
            {
                ffq2[i][j][k2i] =
                    linint(frct1, fflcl[i][j][0][k2i], fflcl[i][j][1][k2i]);
            }
        }
    }

    double ffx[2][2]{};

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            ffx[i][j] =
                linint(frct2, ffq2[i][j][0], ffq2[i][j][1]);
        }
    }

    if (data.xs[static_cast<size_t>(n1 + 1)] +
            data.xs[static_cast<size_t>(n2 + 1)] >
        1.0 + eps8)
    {
        if (data.xs[static_cast<size_t>(n1 + 1)] +
                data.xs[static_cast<size_t>(n2)] <=
            1.0 + eps8)
        {
            const double frcx1_lo =
                (x1 - data.xs[static_cast<size_t>(n1)]) /
                (data.xs[static_cast<size_t>(n1 + 1)] -
                 data.xs[static_cast<size_t>(n1)]);

            const double ffx2_1 =
                linint(frcx1_lo, ffx[0][0], ffx[1][0]);

            if (x1 > 1.0 - data.xs[static_cast<size_t>(n2 + 1)])
            {
                const double frcx2 =
                    (x2 - data.xs[static_cast<size_t>(n2)]) /
                    (1.0 - x1 - data.xs[static_cast<size_t>(n2)]);

                return linint(frcx2, ffx2_1, 0.0);
            }

            const double frcx1_hi =
                (x1 - data.xs[static_cast<size_t>(n1)]) /
                (1.0 - data.xs[static_cast<size_t>(n2 + 1)] -
                 data.xs[static_cast<size_t>(n1)]);

            const double ffx2_2 =
                linint(frcx1_hi, ffx[0][1], 0.0);

            const double frcx2 =
                (x2 - data.xs[static_cast<size_t>(n2)]) /
                (data.xs[static_cast<size_t>(n2 + 1)] -
                 data.xs[static_cast<size_t>(n2)]);

            return linint(frcx2, ffx2_1, ffx2_2);
        }

        if (data.xs[static_cast<size_t>(n2 + 1)] +
                data.xs[static_cast<size_t>(n1)] <=
            1.0 + eps8)
        {
            const double frcx2_lo =
                (x2 - data.xs[static_cast<size_t>(n2)]) /
                (data.xs[static_cast<size_t>(n2 + 1)] -
                 data.xs[static_cast<size_t>(n2)]);

            const double ffx2_1 =
                linint(frcx2_lo, ffx[0][0], ffx[0][1]);

            if (x2 > 1.0 - data.xs[static_cast<size_t>(n1 + 1)])
            {
                const double frcx1 =
                    (x1 - data.xs[static_cast<size_t>(n1)]) /
                    (1.0 - x2 - data.xs[static_cast<size_t>(n1)]);

                return linint(frcx1, ffx2_1, 0.0);
            }

            const double frcx2_hi =
                (x2 - data.xs[static_cast<size_t>(n2)]) /
                (1.0 - data.xs[static_cast<size_t>(n1 + 1)] -
                 data.xs[static_cast<size_t>(n2)]);

            const double ffx2_2 =
                linint(frcx2_hi, ffx[1][0], 0.0);

            const double frcx1 =
                (x1 - data.xs[static_cast<size_t>(n1)]) /
                (data.xs[static_cast<size_t>(n1 + 1)] -
                 data.xs[static_cast<size_t>(n1)]);

            return linint(frcx1, ffx2_1, ffx2_2);
        }

        const double frcx1 =
            (x1 - data.xs[static_cast<size_t>(n1)]) /
            (1.0 - data.xs[static_cast<size_t>(n2)] -
             data.xs[static_cast<size_t>(n1)]);

        const double frcx2 =
            (x2 - data.xs[static_cast<size_t>(n2)]) /
            (1.0 - data.xs[static_cast<size_t>(n1)] -
             data.xs[static_cast<size_t>(n2)]);

        return ffx[0][0] * simpln(frcx1, frcx2);
    }

    double ffx2[2]{};

    if (data.xs[static_cast<size_t>(n1 + 1)] < 0.1)
    {
        const double frcx1 =
            (std::log(x1) - std::log(data.xs[static_cast<size_t>(n1)])) /
            (std::log(data.xs[static_cast<size_t>(n1 + 1)]) -
             std::log(data.xs[static_cast<size_t>(n1)]));

        for (int j = 0; j < 2; ++j)
        {
            if (ffx[0][j] > 0.0 && ffx[1][j] > 0.0)
            {
                ffx2[j] =
                    std::exp(linint(frcx1,
                                    std::log(ffx[0][j]),
                                    std::log(ffx[1][j])));
            }
            else
            {
                ffx2[j] =
                    linint(frcx1, ffx[0][j], ffx[1][j]);
            }
        }
    }
    else
    {
        const double frcx1 =
            (x1 - data.xs[static_cast<size_t>(n1)]) /
            (data.xs[static_cast<size_t>(n1 + 1)] -
             data.xs[static_cast<size_t>(n1)]);

        for (int j = 0; j < 2; ++j)
            ffx2[j] = linint(frcx1, ffx[0][j], ffx[1][j]);
    }

    if (data.xs[static_cast<size_t>(n2 + 1)] < 0.1)
    {
        const double frcx2 =
            (std::log(x2) - std::log(data.xs[static_cast<size_t>(n2)])) /
            (std::log(data.xs[static_cast<size_t>(n2 + 1)]) -
             std::log(data.xs[static_cast<size_t>(n2)]));

        if (ffx2[0] > 0.0 && ffx2[1] > 0.0)
        {
            return std::exp(
                linint(frcx2, std::log(ffx2[0]), std::log(ffx2[1])));
        }

        return linint(frcx2, ffx2[0], ffx2[1]);
    }

    const double frcx2 =
        (x2 - data.xs[static_cast<size_t>(n2)]) /
        (data.xs[static_cast<size_t>(n2 + 1)] -
         data.xs[static_cast<size_t>(n2)]);

    return linint(frcx2, ffx2[0], ffx2[1]);
}

} // namespace

template <class Reader>
const IReader<Reader> *CPDFxTMDDPDFInterpolator<Reader>::getReader() const
{
    return m_reader;
}

template <class Reader>
void CPDFxTMDDPDFInterpolator<Reader>::initialize(const IReader<Reader> *reader)
{
    m_reader = reader;
    m_Shape = reader->getData();
}

template <class Reader>
double CPDFxTMDDPDFInterpolator<Reader>::interpolate(
    PartonFlavor flavor1,
    PartonFlavor flavor2,
    X_T x1,
    MU_T mu1_2,
    X_T x2,
    MU_T mu2_2) const
{
    double x1i = x1;
    double x2i = x2;

    int f1i = static_cast<int>(flavor1);
    int f2i = static_cast<int>(flavor2);

    if (x1i < eps6)
        x1i = eps6;

    if (x2i < eps6)
        x2i = eps6;

    if (x1i + x2i > 1.0 - eps8)
        return 0.0;

    if (mu1_2 <= 0.0 || mu2_2 <= 0.0)
        throw std::runtime_error("DPDF interpolation requires positive mu^2 values");

    double t1 = std::log(mu1_2);
    double t2 = std::log(mu2_2);

    // Important:
    // DPDFB1 stores the full requested scale square.
    // Do not sort t1/t2 globally.
    t1 = std::min(std::max(t1, m_Shape.mus.front()), m_Shape.mus.back());
    t2 = std::min(std::max(t2, m_Shape.mus.front()), m_Shape.mus.back());

    if (f1i == 6 || f2i == 6 || f1i == -6 || f2i == -6)
        return 0.0;

    // Special q qbar channels:
    //   0 -> s sbar
    //   1 -> c cbar
    //   2 -> b bbar
    //
    // Stored orientation is quark-first.
    // If query is antiquark-first, exchange the full legs.
    if ((f1i + f2i == 0) && std::abs(f1i) > 2 && std::abs(f1i) < 6)
    {
        const uint32_t ch3 =
            static_cast<uint32_t>(std::abs(f1i) - 3);

        if (f1i < 0)
        {
            std::swap(x1i, x2i);
            std::swap(t1, t2);
        }

        auto at = [&](uint32_t i, uint32_t j, uint32_t k1, uint32_t k2) {
            return jjbAt(m_Shape, ch3, i, j, k1, k2);
        };

        return x1i * x2i * interpolateCore(m_Shape, at, x1i, x2i, t1, t2);
    }

    // Sea symmetry used by the generator/model for s,c,b.
    if (f1i < -2)
        f1i = std::abs(f1i);

    if (f2i < -2)
        f2i = std::abs(f2i);

    f1i = pdgToInternal(f1i);
    f2i = pdgToInternal(f2i);

    if (f1i < 0 ||
        f1i >= static_cast<int>(UnEqualScaleGridData::kNPartons) ||
        f2i < 0 ||
        f2i >= static_cast<int>(UnEqualScaleGridData::kNPartons))
    {
        throw std::runtime_error("unsupported flavor indices for unequal-scale model");
    }

    // Regular channels are stored in triangular flavor order only.
    // If flavor order changes, exchange the full legs:
    // (x1,t1,f1) <-> (x2,t2,f2)
    if (f2i < f1i)
    {
        std::swap(f1i, f2i);
        std::swap(x1i, x2i);
        std::swap(t1, t2);
    }

    const uint32_t ch =
        triFlavorId(static_cast<uint32_t>(f1i),
                    static_cast<uint32_t>(f2i));

    auto at = [&](uint32_t i, uint32_t j, uint32_t k1, uint32_t k2) {
        return regAt(m_Shape, ch, i, j, k1, k2);
    };

    return x1i * x2i * interpolateCore(m_Shape, at, x1i, x2i, t1, t2);
}

} // namespace PDFxTMD