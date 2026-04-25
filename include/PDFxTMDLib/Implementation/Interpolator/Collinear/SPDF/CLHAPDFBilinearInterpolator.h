#pragma once
#include "PDFxTMDLib/Implementation/Reader/Collinear/SPDF/CDefaultLHAPDFFileReader.h"
#include "PDFxTMDLib/Interface/SPDF/IcPDFInterpolator.h"

namespace PDFxTMD
{
// taken from lhapdf library!
template<class ReaderType>
class CLHAPDFBilinearInterpolator
    : public IcPDFInterpolator<CLHAPDFBilinearInterpolator<ReaderType>, ReaderType>
{
  public:
    CLHAPDFBilinearInterpolator() = default;
    ~CLHAPDFBilinearInterpolator() = default;

    // Main interface method - hot path
    double interpolate(PartonFlavor flavor, double x, double mu2) const;
    void interpolate(double x, double mu2, std::array<double, DEFAULT_TOTAL_PDFS> &output) const;
    void initialize(const IReader<ReaderType> *reader);
    const IReader<ReaderType> *getReader() const;

  private:
    const IReader<ReaderType> *m_reader;
    mutable DefaultAllFlavorShape m_Shape;
};
} // namespace PDFxTMD
#include "./CLHAPDFBilinearInterpolator.tpp"