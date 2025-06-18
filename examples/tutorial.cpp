#define PDFxTMD_ENABLE_LOG 1
#include <iostream>
#include <PDFxTMDLib/Factory.h>
#include <array>
#include <PDFxTMDLib/Common/PartonUtils.h>
#include <PDFxTMDLib/GenericPDF.h>
#include <PDFxTMDLib/Implementation/Interpolator/Collinear/CLHAPDFBilinearInterpolator.h>
#include <PDFxTMDLib/Implementation/Extrapolator/Collinear/CErrExtrapolator.h>
#include <PDFxTMDLib/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h>
#include <PDFxTMDLib/PDFSet.h>
#include <PDFxTMDLib/Common/Logger.h>

constexpr const char* PartonFlavorToString(const int flavorIndex)
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
    // calculatig cPDFs with PDFSet approach. It gives a vector of all PDFSet members.
    PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> PDFSet("MMHT2014lo68cl");
    //Calculating PDF uncertainty is easily done via Uncertainty method. 
    //There is also an overload similar to LHAPDF: void Uncertainty(const std::vector<double> &values, double cl, PDFUncertainty &resUncertainty). 
    PDFxTMD::PDFUncertainty uncertainty;
    /*
    it can also be calculated using this overload:
    uncertainty = PDFSet.Uncertainty(PDFxTMD::PartonFlavor::g, 0.1, 10000, NO_REQUESTED_CONFIDENCE_LEVEL)
    or
    uncertainty = PDFSet.Uncertainty(PDFxTMD::PartonFlavor::g, 0.1, 10000)
    */
    PDFSet.Uncertainty(PDFxTMD::PartonFlavor::g, 0.1, 10000, NO_REQUESTED_CONFIDENCE_LEVEL, uncertainty);
    std::cout << "PDF uncertainties on g computed with MMHT2014lo68cl  at its default CL" << std::endl;
    std::cout << " xg= " << uncertainty.central << " +" << uncertainty.errplus << " -" << uncertainty.errminus << " (+-" << uncertainty.errsymm << ")" << std::endl;
    ////////////////////////////////////////////////////////////////////////////////////
    PDFxTMD::PDFUncertainty uncertainty90PerCent = PDFSet.Uncertainty(PDFxTMD::PartonFlavor::g, 0.1, 10000, 90);
    std::cout << "PDF uncertainties on g computed with MMHT2014lo68cl  at CL = 90%" << std::endl;
    std::cout << " xg= " << uncertainty90PerCent.central << " +" << uncertainty90PerCent.errplus << " -" << uncertainty90PerCent.errminus << " (+-" << uncertainty90PerCent.errsymm << ")" << std::endl;
    //////////////////////////////////////////////////////////////////////////////////////
    /*
    correlation between two PDFSets are easily calculable using:
    double Correlation(PartonFlavor flavorA, double xA, double mu2A, PartonFlavor flavorB, double xB, double mu2B)
    there is also similar method to LHAPDF:
    double Correlation(const std::vector<double>& valuesA, const std::vector<double>& valuesB) const
    */
    double correlation = PDFSet.Correlation(PDFxTMD::PartonFlavor::g, 1e-1, 10000, PDFxTMD::PartonFlavor::g, 1e-1, 10000);
    std::cout << "correlation between g and g at same scale and momentum fraction: " << correlation << std::endl;
    // calculating cPDFs with factory method
    auto PDFFactory = PDFxTMD::GenericCPDFFactory();
    auto MMHT2014LO68cl = PDFFactory.mkCPDF("MMHT2014lo68cl", 0);
    double x = 0.0001;
    double mu2 = 100;
    std::cout << "Gluon factory method:  " << MMHT2014LO68cl.pdf(PDFxTMD::PartonFlavor::g, x, mu2) << std::endl;
    std::cout << "Up factory method:  " << MMHT2014LO68cl.pdf(PDFxTMD::PartonFlavor::u, x, mu2) << std::endl;
    std::cout << "Down factory method:  " << MMHT2014LO68cl.pdf(PDFxTMD::PartonFlavor::d, x, mu2) << std::endl;
    std::cout << "Strange factory method:  " << MMHT2014LO68cl.pdf(PDFxTMD::PartonFlavor::s, x, mu2) << std::endl;
    std::cout << "Charm factory method:  " << MMHT2014LO68cl.pdf(PDFxTMD::PartonFlavor::c, x, mu2) << std::endl;
    std::cout << "Bottom factory method:  " << MMHT2014LO68cl.pdf(PDFxTMD::PartonFlavor::b, x, mu2) << std::endl;
    //calculating cPDFs for all flavors calculation
    std::array<double, 13> mmhtAll;
    MMHT2014LO68cl.pdf(x, mu2, mmhtAll);
    for (int flavorIndex = 0; flavorIndex < 13; flavorIndex++)
    {
        std::cout << "cPDF All flavors factory method: " << PartonFlavorToString(flavorIndex) << ": " << mmhtAll[flavorIndex] << std::endl;
    }

    //calculate cPDFs with type alias
    PDFxTMD::CollinearPDF cpdfPDF("MMHT2014lo68cl", 0);
    std::cout << "Gluon (type alias):  " << cpdfPDF.pdf(PDFxTMD::PartonFlavor::g, x, mu2) << std::endl;
    std::cout << "Up (type alias):  " << cpdfPDF.pdf(PDFxTMD::PartonFlavor::u, x, mu2) << std::endl;
    std::cout << "Down (type alias):  " << cpdfPDF.pdf(PDFxTMD::PartonFlavor::d, x, mu2) << std::endl;
    std::cout << "Strange (type alias):  " << cpdfPDF.pdf(PDFxTMD::PartonFlavor::s, x, mu2) << std::endl;
    std::cout << "Charm (type alias):  " << cpdfPDF.pdf(PDFxTMD::PartonFlavor::c, x, mu2) << std::endl;
    std::cout << "Bottom (type alias):  " << cpdfPDF.pdf(PDFxTMD::PartonFlavor::b, x, mu2) << std::endl;
    //calculate cPDFs for all flavors calculation with type alias
    std::array<double, 13> mmhtAllTypeAlias;
    cpdfPDF.pdf(x, mu2, mmhtAllTypeAlias);
    for (int flavorIndex = 0; flavorIndex < 13; flavorIndex++)
    {
        std::cout << "cPDF All flavors type alias: " << PartonFlavorToString(flavorIndex) << ": " << mmhtAllTypeAlias[flavorIndex] << std::endl;
    }


    //calculate TMDs with factory method
    auto TMDFactory = PDFxTMD::GenericTMDFactory();
    auto PBTMD2020= TMDFactory.mkTMD("PB-LO-HERAI+II-2020-set2", 0);
    double kt2 = 10;
    std::cout << "TMD gluon factory method:  " << PBTMD2020.tmd(PDFxTMD::PartonFlavor::g, x, kt2, mu2) << std::endl;
    std::cout << "TMD up factory method :  " << PBTMD2020.tmd(PDFxTMD::PartonFlavor::u, x, kt2, mu2) << std::endl;
    std::cout << "TMD down factory method:  " << PBTMD2020.tmd(PDFxTMD::PartonFlavor::d, x, kt2, mu2) << std::endl;
    std::cout << "TMD strange factory method:  " << PBTMD2020.tmd(PDFxTMD::PartonFlavor::s, x, kt2, mu2) << std::endl;
    std::cout << "TMD charm factory method:  " << PBTMD2020.tmd(PDFxTMD::PartonFlavor::c, x, kt2, mu2) << std::endl;
    std::cout << "TMD bottom factory method:  " << PBTMD2020.tmd(PDFxTMD::PartonFlavor::b, x, kt2, mu2) << std::endl;
    //calculate TMDs for all flavors calculation
    std::array<double, 13> pbAll;
    PBTMD2020.tmd(x, kt2, mu2, pbAll);
    for (int flavorIndex = 0; flavorIndex < 13; flavorIndex++)
    {
        std::cout << "TMD factory method all flavors: " << PartonFlavorToString(flavorIndex) << ": " << pbAll[flavorIndex] << std::endl;
    }
    //Calculating TMD with type alias
    PDFxTMD::TMDPDF tmdPDF("PB-LO-HERAI+II-2020-set2", 0);
    std::cout << "TMD gluon (type alias):  " << tmdPDF.tmd(PDFxTMD::PartonFlavor::g, x, kt2, mu2) << std::endl;
    std::cout << "TMD up (type alias):  " << tmdPDF.tmd(PDFxTMD::PartonFlavor::u, x, kt2, mu2) << std::endl;
    std::cout << "TMD down (type alias):  " << tmdPDF.tmd(PDFxTMD::PartonFlavor::d, x, kt2, mu2) << std::endl;
    std::cout << "TMD strange (type alias):  " << tmdPDF.tmd(PDFxTMD::PartonFlavor::s, x, kt2, mu2) << std::endl;
    std::cout << "TMD charm (type alias):  " << tmdPDF.tmd(PDFxTMD::PartonFlavor::c, x, kt2, mu2) << std::endl;
    std::cout << "TMD bottom (type alias):  " << tmdPDF.tmd(PDFxTMD::PartonFlavor::b, x, kt2, mu2) << std::endl;
    //calculate TMDs for all flavors calculation with type alias
    std::array<double, 13> pbAllTypeAlias;
    tmdPDF.tmd(x, kt2, mu2, pbAllTypeAlias);
    for (int flavorIndex = 0; flavorIndex < 13; flavorIndex++)
    {
        std::cout << "TMD type alias all flavors: " << PartonFlavorToString(flavorIndex) << ": " << pbAllTypeAlias[flavorIndex] << std::endl;
    }

    //GenericPDF advanced usage
    using ExtrapolatorType = PDFxTMD::CErrExtrapolator;
    using ReaderType = PDFxTMD::CDefaultLHAPDFFileReader;
    using InterpolatorType = PDFxTMD::CLHAPDFBilinearInterpolator;
    using PDFTag = PDFxTMD::CollinearPDFTag;
    using PDFType = PDFxTMD::GenericPDF<PDFTag, ReaderType, InterpolatorType, ExtrapolatorType>;
    PDFType pdf("MMHT2014lo68cl", 0);
    std::cout << "PDF gluon (advanced usage):  " << pdf.pdf(PDFxTMD::PartonFlavor::g, x, mu2) << std::endl;
    std::cout << "PDF up (advanced usage):  " << pdf.pdf(PDFxTMD::PartonFlavor::u, x, mu2) << std::endl;
    std::cout << "PDF down (advanced usage):  " << pdf.pdf(PDFxTMD::PartonFlavor::d, x, mu2) << std::endl;
    std::cout << "PDF strange (advanced usage):  " << pdf.pdf(PDFxTMD::PartonFlavor::s, x, mu2) << std::endl;
    std::cout << "PDF charm (advanced usage):  " << pdf.pdf(PDFxTMD::PartonFlavor::c, x, mu2) << std::endl;
    std::cout << "PDF bottom (advanced usage):  " << pdf.pdf(PDFxTMD::PartonFlavor::b, x, mu2) << std::endl;
    std::array<double, 13> pdfAll;
    pdf.pdf(x, mu2, pdfAll);
    for (int flavorIndex = 0; flavorIndex < 13; flavorIndex++)
    {
        std::cout << "PDF all flavors (advanced usage): " << PartonFlavorToString(flavorIndex) << ": " << pdfAll[flavorIndex] << std::endl;
    }

    //calculate coupling with factory method
    auto CouplingFactory = PDFxTMD::CouplingFactory();
    auto MMHT2014LO68cl_coupling = CouplingFactory.mkCoupling("MMHT2014lo68cl");
    std::cout << "Coupling factory method (mu2 = 100):  " << MMHT2014LO68cl_coupling.AlphaQCDMu2(100) << std::endl;
    std::cout << "Coupling factory method (mu2 = 1000):  " << MMHT2014LO68cl_coupling.AlphaQCDMu2(1000) << std::endl;
    std::cout << "Coupling factory method (mu2 = 10000):  " << MMHT2014LO68cl_coupling.AlphaQCDMu2(10000) << std::endl;
    return 0;
}