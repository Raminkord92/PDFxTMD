#pragma once
#include <stdexcept>
#include <string>

#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Interface/SPDF/ITMDExtrapolator.h"
namespace PDFxTMD
{
class TErrExtrapolator : public ITMDExtrapolator<TErrExtrapolator>
{
  public:
    double extrapolate(PartonFlavor flavor, double x, double kt2, double mu2) const
    {
        throw std::runtime_error("Requested data is outside the PDF grid boundaries");
    }
    void extrapolate(double x, double kt2, double mu2,
                     std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        throw std::runtime_error("Requested data is outside the PDF grid boundaries");
    }
};
} // namespace PDFxTMD