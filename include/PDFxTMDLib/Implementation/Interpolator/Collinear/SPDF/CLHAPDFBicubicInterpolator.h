#pragma once
#include "PDFxTMDLib/Implementation/Reader/Collinear/SPDF/CDefaultLHAPDFFileReader.h"
#include "PDFxTMDLib/Interface/SPDF/IcPDFInterpolator.h"
#include <vector>

// taken from lhapdf library!
namespace PDFxTMD
{
template <class Reader>
class CLHAPDFBicubicInterpolator
    : public IcPDFInterpolator<CLHAPDFBicubicInterpolator<Reader>, Reader>
{
  public:
    CLHAPDFBicubicInterpolator() = default;
    ~CLHAPDFBicubicInterpolator() = default;

    double interpolate(PartonFlavor flavor, double x, double q2) const;
    void interpolate(double x, double q2, std::array<double, DEFAULT_TOTAL_PDFS> &output) const;
    void initialize(const IReader<Reader> *reader);
    const IReader<Reader> *getReader() const;

  private:
    const IReader<Reader> *m_reader;
    mutable DefaultAllFlavorShape m_Shape;
};
} // namespace PDFxTMD
#include "./CLHAPDFBicubicInterpolator.tpp"
