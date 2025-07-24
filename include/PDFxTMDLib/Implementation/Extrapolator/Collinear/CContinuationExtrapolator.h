// This file is based on the LHAPDF code

#pragma once

#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h"
#include "PDFxTMDLib/Interface/IExtrapolator.h"
#include "PDFxTMDLib/Interface/IInterpolator.h"
#include <cmath>
#include <iomanip>
#include <sstream>
namespace PDFxTMD
{

template <typename Interpolator>
class CContinuationExtrapolator
    : public IcAdvancedPDFExtrapolator<CContinuationExtrapolator<Interpolator>>
{
  public:
    void setInterpolator(const Interpolator *interpolator)
    {
        m_interpolator = interpolator;
    }
    void extrapolate(double x, double q2, std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        using namespace std;
        // The ContinuationExtrapolator provides an implementation
        // of the extrapolation used in the MSTW standalone code
        // (and LHAPDFv5 when using MSTW sets), G. Watt, October
        // 2014.
        auto *reader = m_interpolator->getReader();
        const auto& xVals_ = reader->getValues(PhaseSpaceComponent::X);
        const auto& q2Vals_ = reader->getValues(PhaseSpaceComponent::Q2);
        const size_t nxknots = xVals_.size(); // total number of x knots (all
                                                              // subgrids)
        const size_t nq2knots = q2Vals_.size(); // total number of q2 knots (all
                                                               // subgrids)

        const double xMin = xVals_.at(0);  // first x knot
        const double xMin1 = xVals_.at(1); // second x knot
        const double xMax = xVals_.at(nxknots - 1); // last x knot

        const double q2Min = q2Vals_.at(0); // first q2 knot
        const double q2Max1 = q2Vals_.at(nq2knots - 2); // second-last q2 knot
        const double q2Max = q2Vals_.at(nq2knots - 1); // last q2 knot

        double fxMin = 0, fxMin1 = 0, fq2Max = 0, fq2Max1 = 0, fq2Min = 0, fq2Min1 = 0, xpdf = 0,
               anom = 0;
        for (size_t i = 0; i < DEFAULT_TOTAL_PDFS; i += 1)
        {
            PartonFlavor flavor = standardPartonFlavors[i];
            if (x < xMin && (q2 >= q2Min && q2 <= q2Max))
            {
                // Extrapolation in small x only.
                fxMin = m_interpolator->interpolate(flavor, xMin, q2);   // PDF at (xMin,q2)
                fxMin1 = m_interpolator->interpolate(flavor, xMin1, q2); // PDF at (xMin1,q2)
                xpdf = _extrapolateLinear(x, xMin, xMin1, fxMin,
                                          fxMin1); // PDF at (x,q2)
            }
            else if ((x >= xMin && x <= xMax) && q2 > q2Max)
            {
                // Extrapolation in large q2 only.
                fq2Max = m_interpolator->interpolate(flavor, x, q2Max);   // PDF at (x,q2Max)
                fq2Max1 = m_interpolator->interpolate(flavor, x, q2Max1); // PDF at (x,q2Max1)
                xpdf = _extrapolateLinear(q2, q2Max, q2Max1, fq2Max,
                                          fq2Max1); // PDF at (x,q2)
            }
            else if (x < xMin && q2 > q2Max)
            {
                // Extrapolation in large q2 AND small x.
                fq2Max = m_interpolator->interpolate(flavor, xMin, q2Max);   // PDF at (xMin,q2Max)
                fq2Max1 = m_interpolator->interpolate(flavor, xMin, q2Max1); // PDF at (xMin,q2Max1)
                fxMin = _extrapolateLinear(q2, q2Max, q2Max1, fq2Max,
                                           fq2Max1);                        // PDF at (xMin,q2)
                fq2Max = m_interpolator->interpolate(flavor, xMin1, q2Max); // PDF at (xMin1,q2Max)
                fq2Max1 =
                    m_interpolator->interpolate(flavor, xMin1, q2Max1); // PDF at (xMin1,q2Max1)
                fxMin1 = _extrapolateLinear(q2, q2Max, q2Max1, fq2Max,
                                            fq2Max1); // PDF at (xMin1,q2)
                xpdf = _extrapolateLinear(x, xMin, xMin1, fxMin,
                                          fxMin1); // PDF at (x,q2)
            }
            else if (q2 < q2Min && x <= xMax)
            {
                // Extrapolation in small q2.

                if (x < xMin)
                {
                    // Extrapolation also in small x.

                    fxMin = m_interpolator->interpolate(flavor, xMin, q2Min); // PDF at (xMin,q2Min)
                    fxMin1 =
                        m_interpolator->interpolate(flavor, xMin1, q2Min); // PDF at (xMin1,q2Min)
                    fq2Min = _extrapolateLinear(x, xMin, xMin1, fxMin,
                                                fxMin1); // PDF at (x,q2Min)
                    fxMin = m_interpolator->interpolate(flavor, xMin,
                                                        1.01 * q2Min); // PDF at (xMin,1.01*q2Min)
                    fxMin1 = m_interpolator->interpolate(flavor, xMin1,
                                                         1.01 * q2Min); // PDF at (xMin1,1.01*q2Min)
                    fq2Min1 = _extrapolateLinear(x, xMin, xMin1, fxMin,
                                                 fxMin1); // PDF at (x,1.01*q2Min)
                }
                else
                {
                    // Usual interpolation in x.

                    fq2Min = m_interpolator->interpolate(flavor, x, q2Min); // PDF at (x,q2Min)
                    fq2Min1 = m_interpolator->interpolate(flavor, x,
                                                          1.01 * q2Min); // PDF at (x,1.01*q2Min)
                }

                // Calculate the anomalous dimension, dlog(f)/dlog(q2),
                // evaluated at q2Min.  Then extrapolate the PDFs to low
                // q2 < q2Min by interpolating the anomalous dimension
                // between the value at q2Min and a value of 1 for q2 <<
                // q2Min. If value of PDF at q2Min is very small, just
                // set anomalous dimension to 1 to prevent rounding
                // errors. Impose minimum anomalous dimension of -2.5.

                if (fabs(fq2Min) >= 1e-5)
                {
                    // anom = dlog(f)/dlog(q2) = q2/f * df/dq2 evaluated
                    // at q2 = q2Min, where derivative df/dq2 = (
                    // f(1.01*q2Min) - f(q2Min) ) / (0.01*q2Min).
                    anom = std::max(-2.5, (fq2Min1 - fq2Min) / fq2Min / 0.01);
                }
                else
                    anom = 1.0;

                // Interpolates between f(q2Min)*(q2/q2Min)^anom for q2
                // ~ q2Min and f(q2Min)*(q2/q2Min) for q2 << q2Min, i.e.
                // PDFs vanish as q2 --> 0.
                xpdf = fq2Min * std::pow(q2 / q2Min, anom * q2 / q2Min + 1.0 - q2 / q2Min);
            }
            else if (x > xMax)
            {
                std::ostringstream oss;
                oss << "Error in LHAPDF::ContinuationExtrapolator, x > "
                       "xMax (last x knot): ";
                oss << std::scientific << x << " > " << xMax;
                throw std::runtime_error(oss.str());
            }
            else
                throw std::runtime_error("We shouldn't be able to get here!");

            output[i] = xpdf;
        }
    }
    double extrapolate(PartonFlavor flavor, double x, double q2) const
    {
        using namespace std;
        // The ContinuationExtrapolator provides an implementation
        // of the extrapolation used in the MSTW standalone code
        // (and LHAPDFv5 when using MSTW sets), G. Watt, October
        // 2014.
        auto *reader = m_interpolator->getReader();
        const auto& xVals_ = reader->getValues(PhaseSpaceComponent::X);
        const auto& q2Vals_ = reader->getValues(PhaseSpaceComponent::Q2);
        const size_t nxknots = xVals_.size(); // total number of x knots (all
                                                              // subgrids)
        const size_t nq2knots = q2Vals_.size(); // total number of q2 knots (all
                                                               // subgrids)
        const double xMin = xVals_.at(0);  // first x knot
        const double xMin1 = xVals_.at(1); // second x knot
        const double xMax = xVals_.at(nxknots - 1); // last x knot

        const double q2Min = q2Vals_.at(0); // first q2 knot
        const double q2Max1 = q2Vals_.at(nq2knots - 2); // second-last q2 knot
        const double q2Max = q2Vals_.at(nq2knots - 1); // last q2 knot

        double fxMin, fxMin1, fq2Max, fq2Max1, fq2Min, fq2Min1, xpdf, anom;

        if (x < xMin && (q2 >= q2Min && q2 <= q2Max))
        {
            // Extrapolation in small x only.
            fxMin = m_interpolator->interpolate(flavor, xMin, q2);   // PDF at (xMin,q2)
            fxMin1 = m_interpolator->interpolate(flavor, xMin1, q2); // PDF at (xMin1,q2)
            xpdf = _extrapolateLinear(x, xMin, xMin1, fxMin,
                                      fxMin1); // PDF at (x,q2)
        }
        else if ((x >= xMin && x <= xMax) && q2 > q2Max)
        {
            // Extrapolation in large q2 only.
            fq2Max = m_interpolator->interpolate(flavor, x, q2Max);   // PDF at (x,q2Max)
            fq2Max1 = m_interpolator->interpolate(flavor, x, q2Max1); // PDF at (x,q2Max1)
            xpdf = _extrapolateLinear(q2, q2Max, q2Max1, fq2Max,
                                      fq2Max1); // PDF at (x,q2)
        }
        else if (x < xMin && q2 > q2Max)
        {
            // Extrapolation in large q2 AND small x.
            fq2Max = m_interpolator->interpolate(flavor, xMin, q2Max);   // PDF at (xMin,q2Max)
            fq2Max1 = m_interpolator->interpolate(flavor, xMin, q2Max1); // PDF at (xMin,q2Max1)
            fxMin = _extrapolateLinear(q2, q2Max, q2Max1, fq2Max,
                                       fq2Max1);                          // PDF at (xMin,q2)
            fq2Max = m_interpolator->interpolate(flavor, xMin1, q2Max);   // PDF at (xMin1,q2Max)
            fq2Max1 = m_interpolator->interpolate(flavor, xMin1, q2Max1); // PDF at (xMin1,q2Max1)
            fxMin1 = _extrapolateLinear(q2, q2Max, q2Max1, fq2Max,
                                        fq2Max1); // PDF at (xMin1,q2)
            xpdf = _extrapolateLinear(x, xMin, xMin1, fxMin,
                                      fxMin1); // PDF at (x,q2)
        }
        else if (q2 < q2Min && x <= xMax)
        {
            // Extrapolation in small q2.

            if (x < xMin)
            {
                // Extrapolation also in small x.

                fxMin = m_interpolator->interpolate(flavor, xMin, q2Min);   // PDF at (xMin,q2Min)
                fxMin1 = m_interpolator->interpolate(flavor, xMin1, q2Min); // PDF at (xMin1,q2Min)
                fq2Min = _extrapolateLinear(x, xMin, xMin1, fxMin,
                                            fxMin1); // PDF at (x,q2Min)
                fxMin = m_interpolator->interpolate(flavor, xMin,
                                                    1.01 * q2Min); // PDF at (xMin,1.01*q2Min)
                fxMin1 = m_interpolator->interpolate(flavor, xMin1,
                                                     1.01 * q2Min); // PDF at (xMin1,1.01*q2Min)
                fq2Min1 = _extrapolateLinear(x, xMin, xMin1, fxMin,
                                             fxMin1); // PDF at (x,1.01*q2Min)
            }
            else
            {
                // Usual interpolation in x.

                fq2Min = m_interpolator->interpolate(flavor, x, q2Min); // PDF at (x,q2Min)
                fq2Min1 = m_interpolator->interpolate(flavor, x,
                                                      1.01 * q2Min); // PDF at (x,1.01*q2Min)
            }

            // Calculate the anomalous dimension, dlog(f)/dlog(q2),
            // evaluated at q2Min.  Then extrapolate the PDFs to low
            // q2 < q2Min by interpolating the anomalous dimension
            // between the value at q2Min and a value of 1 for q2 <<
            // q2Min. If value of PDF at q2Min is very small, just
            // set anomalous dimension to 1 to prevent rounding
            // errors. Impose minimum anomalous dimension of -2.5.

            if (fabs(fq2Min) >= 1e-5)
            {
                // anom = dlog(f)/dlog(q2) = q2/f * df/dq2 evaluated
                // at q2 = q2Min, where derivative df/dq2 = (
                // f(1.01*q2Min) - f(q2Min) ) / (0.01*q2Min).
                anom = std::max(-2.5, (fq2Min1 - fq2Min) / fq2Min / 0.01);
            }
            else
                anom = 1.0;

            // Interpolates between f(q2Min)*(q2/q2Min)^anom for q2
            // ~ q2Min and f(q2Min)*(q2/q2Min) for q2 << q2Min, i.e.
            // PDFs vanish as q2 --> 0.
            xpdf = fq2Min * std::pow(q2 / q2Min, anom * q2 / q2Min + 1.0 - q2 / q2Min);
        }
        else if (x > xMax)
        {
            std::ostringstream oss;
            oss << "Error in LHAPDF::ContinuationExtrapolator, x > "
                   "xMax (last x knot): ";
            oss << std::scientific << x << " > " << xMax;
            throw std::runtime_error(oss.str());
        }
        else
            throw std::runtime_error("We shouldn't be able to get here!");

        return xpdf;
    }

  private:
    const Interpolator *m_interpolator = nullptr;
};
} // namespace PDFxTMD