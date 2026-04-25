#pragma once
#include "../IExtrapolator.h"

namespace PDFxTMD
{
template <typename Derived>
class IcDPDFExtrapolator : public IExtrapolator<Derived, X_T, MU_T, X_T, MU_T>
{
  public:
    double extrapolate(PartonFlavor flavor1, PartonFlavor flavor2, X_T x1, MU_T mu1_2, X_T x2,
                       MU_T mu2_2) const
    {
        return this->derived().extrapolate(flavor1, flavor2, x1, mu1_2, x2, mu2_2);
    }
};
} // namespace PDFxTMD