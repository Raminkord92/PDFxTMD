#pragma once
#include "../IInterpolator.h"

namespace PDFxTMD
{
template <typename Derived, typename Reader>
class IcDPDFInterpolator : public IInterpolator<Derived, Reader, X_T, MU_T, X_T, MU_T>
{
    void interpolate(PartonFlavor flavor1, PartonFlavor flavor2, X_T x1, MU_T mu1_2, X_T x2, MU_T mu2_2 ) const
    {
        return this->derived().interpolate(flavor1, flavor2, x1, mu1_2, x2, mu2_2);
    }
};
} // namespace PDFxTMD
