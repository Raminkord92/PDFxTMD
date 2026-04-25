#pragma once
#include "../IExtrapolator.h"

namespace PDFxTMD
{
template <typename Derived> class IcPDFExtrapolator : public IExtrapolator<Derived, X_T, MU_T>
{
    double extrapolate(X_T x, MU_T mu, std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        return this->derived().extrapolate(x, mu, output);
    }
};

template <typename Derived>
class IcAdvancedPDFExtrapolator : public IAdvancedExtrapolator<Derived, X_T, MU_T>
{
    double extrapolate(X_T x, MU_T mu, std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        return this->derived().extrapolate(x, mu, output);
    }
};
} // namespace PDFxTMD