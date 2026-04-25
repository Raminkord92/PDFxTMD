#pragma once
#include <stdexcept>
#include <string>

#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Interface/SPDF/ITMDExtrapolator.h"

namespace PDFxTMD
{
class TZeroExtrapolator : public ITMDExtrapolator<TZeroExtrapolator>
{
  public:
    double extrapolate(PartonFlavor flavor, double x, double kt2, double mu2) const
    {
        return 0.0;
    }
    void extrapolate(double x, double kt2, double mu2,
                     std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        output.fill(0);
    }
};
} // namespace PDFxTMD