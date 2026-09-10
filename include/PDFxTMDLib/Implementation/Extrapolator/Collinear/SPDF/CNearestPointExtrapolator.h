// This file is based on the LHAPDF code

#pragma once

#include "PDFxTMDLib/Common/PDFUtils.h"
#include "PDFxTMDLib/Interface/SPDF/IcPDFExtrapolator.h"
#include <cmath>

namespace PDFxTMD
{
// Return the value in the given list that best matches the
// target value
inline double _findClosestMatch(const std::vector<double> &cands, double target)
{
    auto it = std::lower_bound(cands.begin(), cands.end(), target);
    const double upper = *it;
    const double lower =
        (it == cands.begin()) ? upper : *(--it); //< Avoid decrementing the first entry
    /// @todo Closeness in linear or log space? Hmm...
    if (std::fabs(target - upper) < std::fabs(target - lower))
        return upper;
    return lower;
}

template <typename Interpolator>
class CNearestPointExtrapolator
    : public IcAdvancedPDFExtrapolator<CNearestPointExtrapolator<Interpolator>>
{
  public:
    void setInterpolator(const Interpolator *interpolator)
    {
        m_interpolator = interpolator;
    }
    double extrapolate(PartonFlavor flavor, double x, double mu2) const
    {
        /// Find the closest valid x and Q2 points, either on- or
        /// off-grid, and use the current interpolator
        /// @todo raise error for x > 1 ?
        const auto *reader = m_interpolator->getReader();
        auto xVals = reader->getValues(PhaseSpaceComponent::X);
        auto q2Vals = reader->getValues(PhaseSpaceComponent::Q2);

        const double closestX = (isInRangeX(*reader, x)) ? x : _findClosestMatch(xVals, x);
        const double closestQ2 = (isInRangeQ2(*reader, mu2)) ? mu2 : _findClosestMatch(q2Vals, mu2);
        return this->m_interpolator->interpolate(flavor, closestX, closestQ2);
    }

    void extrapolate(double x, double mu2, std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        /// Find the closest valid x and Q2 points, either on- or
        /// off-grid, and use the current interpolator
        /// @todo raise error for x > 1 ?
        const auto *reader = m_interpolator->getReader();
        auto xVals = reader->getValues(PhaseSpaceComponent::X);
        auto q2Vals = reader->getValues(PhaseSpaceComponent::Q2);

        const double closestX = (isInRangeX(*reader, x)) ? x : _findClosestMatch(xVals, x);
        const double closestQ2 = (isInRangeQ2(*reader, mu2)) ? mu2 : _findClosestMatch(q2Vals, mu2);
        for (int i = 0; i < DEFAULT_TOTAL_PDFS; i++)
        {
            output[i] =
                this->m_interpolator->interpolate(standardPartonFlavors[i], closestX, closestQ2);
        }
    }

  private:
    const Interpolator *m_interpolator = nullptr;
};
} // namespace PDFxTMD