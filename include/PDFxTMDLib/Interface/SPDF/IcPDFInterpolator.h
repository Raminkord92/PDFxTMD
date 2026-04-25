#pragma once
#include "../IInterpolator.h"
#include "PDFxTMDLib/Common/PartonUtils.h"

namespace PDFxTMD
{
template <typename Derived, typename Reader>
class IcPDFInterpolator : public IInterpolator<Derived, Reader, X_T, MU_T>
{
    double interpolate(PartonFlavor parton1, PartonFlavor parton2, X_T x1, MU_T mu1_2, X_T x2, MU_T mu2_2) const
    {
         return this->derived().interpolate(parton1, parton2, x1, mu1_2, x2, mu2_2);
    }
};
} // namespace PDFxTMD
