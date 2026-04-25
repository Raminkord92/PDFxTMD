#pragma once
#include "PDFxTMDLib/Common/SPDF/AllFlavorsShape.h"
#include "PDFxTMDLib/Common/NumParser.h"
#include "PDFxTMDLib/Interface/IReader.h"

namespace PDFxTMD
{

class TDefaultLHAPDF_TMDReader : public IReader<TDefaultLHAPDF_TMDReader>
{
  public:
    void read(const std::string &pdfName, int setNumber);
    DefaultAllFlavorTMDShape getData() const;
    std::vector<double> getValues(PhaseSpaceComponent comp) const;
    std::pair<double, double> getBoundaryValues(PhaseSpaceComponent comp) const;

  private:
    DefaultAllFlavorTMDShape m_pdfShape;
    int m_blockNumber = 0;
    int m_blockLine = 0;
    std::pair<double, double> m_xMinMax;
    std::pair<double, double> m_q2MinMax;
    std::pair<double, double> m_kt2MinMax;

  private:
    void processDataLine(const std::string &line, DefaultAllFlavorTMDShape &data);
    void readXKnots(NumParser &parser, DefaultAllFlavorTMDShape &data);
    void readKt2Knots(NumParser &parser, DefaultAllFlavorTMDShape &data);
    void readQ2Knots(NumParser &parser, DefaultAllFlavorTMDShape &data);
    void readParticleIds(NumParser &parser, DefaultAllFlavorTMDShape &data);
    void readValues(NumParser &parser, DefaultAllFlavorTMDShape &data);
};
} // namespace PDFxTMD
