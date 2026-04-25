#pragma once

#include <string>
#include <vector>

#include "PDFxTMDLib/Common/SPDF/AllFlavorsShape.h"
#include "PDFxTMDLib/Implementation/Reader/TMD/SPDF/TDefaultLHAPDF_TMDReader.h"
#include "PDFxTMDLib/Interface/SPDF/ITMDInterpolator.h"
#include "PDFxTMDLib/Interface/IReader.h"
#include "PDFxTMDLib/external/mlinterp/mlinterp.hpp"

namespace PDFxTMD
{
template <class ReaderType>
class TTrilinearInterpolator : public ITMDInterpolator<TTrilinearInterpolator<ReaderType>, ReaderType>
{
  public:
    explicit TTrilinearInterpolator() = default;
    void initialize(const IReader<ReaderType> *reader)
    {
        m_reader = reader;
        m_tmdShape = reader->getData();
        m_dimensions = {static_cast<int>(m_tmdShape.x_vec.size()),
                        static_cast<int>(m_tmdShape.kt2_vec.size()),
                        static_cast<int>(m_tmdShape.mu2_vec.size())};
    }
    double interpolate(PartonFlavor flavor, double x, double kt2, double mu2) const
    {
        using namespace mlinterp;
        double *selectedPdf = &m_tmdShape.grids[flavor][0];
        double output[1];
        double logX = std::log(x);
        double logkt2 = std::log(kt2);
        double logMu2 = std::log(mu2);
        interp(m_dimensions.data(), 1, selectedPdf, output, m_tmdShape.log_x_vec.data(), &logX, m_tmdShape.log_kt2_vec.data(), &logkt2
               , m_tmdShape.log_mu2_vec.data(), &logMu2);

        return output[0] < 0 ? 0 : output[0] / kt2;
    }
    void interpolate(double x, double kt2, double mu2,
                     std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        using namespace mlinterp;
        double output_[1];
        double logX = std::log(x);
        double logkt2 = std::log(kt2);
        double logMu2 = std::log(mu2);

        for (int i = 0; i < DEFAULT_TOTAL_PDFS; i++)
        {
            double *selectedPdf = &m_tmdShape.grids[standardPartonFlavors[i]][0];
            interp(m_dimensions.data(), 1, selectedPdf, output_, m_tmdShape.log_x_vec.data(), &logX,
                   m_tmdShape.log_kt2_vec.data(), &logkt2, m_tmdShape.log_mu2_vec.data(), &logMu2);

            output[i] = (output_[0] < 0 ? 0 : output_[0] / kt2);
        }
    }
    const IReader<ReaderType> *getReader() const
    {
        return m_reader;
    }

  private:
    const IReader<ReaderType> *m_reader;
    std::array<int, 3> m_dimensions;
    mutable DefaultAllFlavorTMDShape m_tmdShape;
};

} // namespace PDFxTMD
