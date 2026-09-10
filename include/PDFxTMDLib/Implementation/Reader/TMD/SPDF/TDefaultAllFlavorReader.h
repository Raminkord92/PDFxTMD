
#pragma once

#include "PDFxTMDLib/Common/SPDF/AllFlavorsShape.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Interface/IReader.h"
#include <string>
#include <vector>

namespace PDFxTMD
{
class TDefaultAllFlavorReader : public IReader<TDefaultAllFlavorReader>
{
  public:
    void read(const std::string &pdfName, int setNumber);
    DefaultAllFlavorTMDShape getData() const;
    std::pair<double, double> getBoundaryValues(PhaseSpaceComponent comp) const;

  private:
    DefaultAllFlavorTMDShape m_updfShape;
    std::pair<double, double> m_xMinMax;
    std::pair<double, double> m_q2MinMax;
    std::pair<double, double> m_kt2MinMax;
};
} // namespace PDFxTMD
