#include <PDFxTMDLib/GenericPDF.h>
#include <PDFxTMDLib/Implementation/Extrapolator/Collinear/SPDF/CErrExtrapolator.h>
#include <PDFxTMDLib/Implementation/Interpolator/Collinear/SPDF/CLHAPDFBilinearInterpolator.h>
#include <PDFxTMDLib/Implementation/Reader/Collinear/SPDF/CDefaultLHAPDFFileReader.h>
#include <iostream>

constexpr const char *PartonFlavorToString(const int flavorIndex)
{
    switch (flavorIndex)
    {
    case 0:
        return "Anti Top";
    case 1:
        return "Anti Bottom";
    case 2:
        return "Anti Charm";
    case 3:
        return "Anti Strange";
    case 4:
        return "Anti Up";
    case 5:
        return "Anti Down";
    case 6:
        return "Gluon";
    case 7:
        return "Down";
    case 8:
        return "Up";
    case 9:
        return "Strange";
    case 10:
        return "Charm";
    case 11:
        return "Bottom";
    case 12:
        return "Top";
    default:
        break;
    }
    return "";
}

int main()
{
    using namespace PDFxTMD;
    double x = 0.001, kt2 = 10, mu2 = 100;
    // GenericPDF advanced usage
    using ExtrapolatorType = CErrExtrapolator;
    using ReaderType = CDefaultLHAPDFFileReader;
    using InterpolatorType = CLHAPDFBilinearInterpolator<ReaderType>;
    using PDFTag = CollinearPDFTag;
    using PDFType = GenericPDF<PDFTag, ReaderType, InterpolatorType, ExtrapolatorType>;
    PDFType cpdf("MMHT2014lo68cl", 0);
    std::cout << "PDF gluon (advanced usage):  " << cpdf.pdf(PartonFlavor::g, x, mu2) << std::endl;
    std::cout << "PDF up (advanced usage):  " << cpdf.pdf(PartonFlavor::u, x, mu2) << std::endl;
    std::cout << "PDF down (advanced usage):  " << cpdf.pdf(PartonFlavor::d, x, mu2) << std::endl;
    std::cout << "PDF strange (advanced usage):  " << cpdf.pdf(PartonFlavor::s, x, mu2)
              << std::endl;
    std::cout << "PDF charm (advanced usage):  " << cpdf.pdf(PartonFlavor::c, x, mu2) << std::endl;
    std::cout << "PDF bottom (advanced usage):  " << cpdf.pdf(PartonFlavor::b, x, mu2) << std::endl;
    std::array<double, 13> pdfAll;
    cpdf.pdf(x, mu2, pdfAll);
    for (int flavorIndex = 0; flavorIndex < 13; flavorIndex++)
    {
        std::cout << "PDF all flavors (advanced usage): " << PartonFlavorToString(flavorIndex)
                  << ": " << pdfAll[flavorIndex] << std::endl;
    }
    // calculate cPDFs with type alias
    CollinearPDF cpdfPDF("MMHT2014lo68cl", 0);
    std::cout << "Gluon (type alias):  " << cpdfPDF.pdf(PartonFlavor::g, x, mu2) << std::endl;
    std::cout << "Up (type alias):  " << cpdfPDF.pdf(PartonFlavor::u, x, mu2) << std::endl;
    std::cout << "Down (type alias):  " << cpdfPDF.pdf(PartonFlavor::d, x, mu2) << std::endl;
    std::cout << "Strange (type alias):  " << cpdfPDF.pdf(PartonFlavor::s, x, mu2) << std::endl;
    std::cout << "Charm (type alias):  " << cpdfPDF.pdf(PartonFlavor::c, x, mu2) << std::endl;
    std::cout << "Bottom (type alias):  " << cpdfPDF.pdf(PartonFlavor::b, x, mu2) << std::endl;
    // calculate cPDFs for all flavors calculation with type alias
    std::array<double, 13> mmhtAllTypeAlias;
    cpdfPDF.pdf(x, mu2, mmhtAllTypeAlias);
    for (int flavorIndex = 0; flavorIndex < 13; flavorIndex++)
    {
        std::cout << "cPDF All flavors type alias: " << PartonFlavorToString(flavorIndex) << ": "
                  << mmhtAllTypeAlias[flavorIndex] << std::endl;
    }

    // Calculating TMD with type alias
    TMDPDF tmdPDF("PB-LO-HERAI+II-2020-set2", 0);
    std::cout << "TMD gluon (type alias):  " << tmdPDF.tmd(PartonFlavor::g, x, kt2, mu2)
              << std::endl;
    std::cout << "TMD up (type alias):  " << tmdPDF.tmd(PartonFlavor::u, x, kt2, mu2) << std::endl;
    std::cout << "TMD down (type alias):  " << tmdPDF.tmd(PartonFlavor::d, x, kt2, mu2)
              << std::endl;
    std::cout << "TMD strange (type alias):  " << tmdPDF.tmd(PartonFlavor::s, x, kt2, mu2)
              << std::endl;
    std::cout << "TMD charm (type alias):  " << tmdPDF.tmd(PartonFlavor::c, x, kt2, mu2)
              << std::endl;
    std::cout << "TMD bottom (type alias):  " << tmdPDF.tmd(PartonFlavor::b, x, kt2, mu2)
              << std::endl;
    // calculate TMDs for all flavors calculation with type alias
    std::array<double, 13> pbAllTypeAlias;
    tmdPDF.tmd(x, kt2, mu2, pbAllTypeAlias);
    for (int flavorIndex = 0; flavorIndex < 13; flavorIndex++)
    {
        std::cout << "TMD type alias all flavors: " << PartonFlavorToString(flavorIndex) << ": "
                  << pbAllTypeAlias[flavorIndex] << std::endl;
    }
    return 0;
}