#pragma once
#include "../IExtrapolator.h"
namespace PDFxTMD
{
template <typename Derived> class ITMDExtrapolator : public IExtrapolator<Derived, X_T, KT_T, MU_T>
{
    double extrapolate(X_T x, KT_T kt, MU_T mu,
                       std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        return this->derived().extrapolate(x, kt, mu, output);
    }
};

template <typename Derived>
class ITMDAdvancedTMDExtrapolator : public IAdvancedExtrapolator<Derived, X_T, KT_T, MU_T>
{
    double extrapolate(X_T x, KT_T kt, MU_T mu,
                       std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        return this->derived().extrapolate(x, kt, mu, output);
    }
};
}