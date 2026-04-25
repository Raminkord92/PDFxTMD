#pragma once
#include "PDFxTMDLib/Common/SPDF/AllFlavorsShape.h"
#include "PDFxTMDLib/Common/NumParser.h"
#include "PDFxTMDLib/Interface/IReader.h"

namespace PDFxTMD
{

class CDefaultLHAPDFFileReader : public IReader<CDefaultLHAPDFFileReader>
{
  public:
    void read(const std::string &pdfName, int setNumber);
    DefaultAllFlavorShape getData() const;
    std::vector<double> getValues(PhaseSpaceComponent comp) const;
    std::pair<double, double> getBoundaryValues(PhaseSpaceComponent comp) const;

  private:
    std::vector<DefaultAllFlavorShape> m_pdfShape;
    DefaultAllFlavorShape m_pdfShape_flat;
    std::vector<double> m_mu2CompTotal;
    int m_blockNumber = 0;
    int m_blockLine = 0;
    std::pair<double, double> m_xMinMax;
    std::pair<double, double> m_q2MinMax;

  private:
    void processDataLine(const std::string &line, DefaultAllFlavorShape &data);
    void readXKnots(NumParser &parser, DefaultAllFlavorShape &data);
    void readQ2Knots(NumParser &parser, DefaultAllFlavorShape &data);
    void readParticleIds(NumParser &parser, DefaultAllFlavorShape &data);
    void readValues(NumParser &parser, DefaultAllFlavorShape &data);
};
} // namespace PDFxTMD
