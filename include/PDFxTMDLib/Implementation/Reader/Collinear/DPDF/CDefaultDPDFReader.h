#pragma once
#include "PDFxTMDLib/Common/DPDF/UnequalScaleGrid.h"
#include "PDFxTMDLib/Common/NumParser.h"
#include "PDFxTMDLib/Interface/IReader.h"
#include <map>

namespace PDFxTMD
{
class CDefaultDPDFReader : public IReader<CDefaultDPDFReader>
{
    public:
    void read(const std::string &pdfName, int setNumber);
    UnEqualScaleGridData getData() const;
    std::vector<double> getValues(PhaseSpaceComponent comp) const;
    std::pair<double, double> getBoundaryValues(PhaseSpaceComponent comp) const;
    private:
    UnEqualScaleGridData m_gridData;
    std::pair<double, double> m_xMinMax;
    std::pair<double, double> m_q2MinMax;
};

}