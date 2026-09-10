#pragma once

#include "../IInterpolator.h"

namespace PDFxTMD
{
template <typename Derived, typename Reader>
class ITMDInterpolator : public IInterpolator<Derived, Reader, X_T, KT_T, MU_T>
{
    void interpolate(X_T x, KT_T kt, MU_T mu, std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        return this->derived().interpolate(x, kt, mu, output);
    }
};
}
