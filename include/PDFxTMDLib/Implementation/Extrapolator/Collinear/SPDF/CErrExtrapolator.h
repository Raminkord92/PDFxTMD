// This file is based on the LHAPDF code

#pragma once
#include <stdexcept>
#include <string>

#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Interface/SPDF/IcPDFExtrapolator.h"
namespace PDFxTMD
{
class CErrExtrapolator : public IcPDFExtrapolator<CErrExtrapolator>
{
  public:
    double extrapolate(PartonFlavor flavor, double x, double mu2) const
    {
        throw std::runtime_error("Point x=" + std::to_string(x) + ", Q2=" + std::to_string(mu2) +
                                 " is outside the PDF grid boundaries");
    }
    void extrapolate(double x, double mu2, std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        throw std::runtime_error("Point x=" + std::to_string(x) + ", Q2=" + std::to_string(mu2) +
                                 " is outside the PDF grid boundaries");
    }
};
} // namespace PDFxTMD