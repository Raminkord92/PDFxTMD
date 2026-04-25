#pragma once
#include <stdexcept>
#include <string>

#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Interface/DPDF/IcDPDFExtrapolator.h"

namespace PDFxTMD
{
class CDPDFZeroExtrapolator : public IcDPDFExtrapolator<CDPDFZeroExtrapolator>
{
  public:
    double extrapolate(PartonFlavor flavor1, PartonFlavor flavor2, double x1,  double mu1_2, double x2, double mu2_2) const
    {
        return 0.0;
    }
};
} // namespace PDFxTMD