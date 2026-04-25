#include "PDFxTMDLib/GenericPDF.h"
#include <iostream>

int main()
{
    using namespace PDFxTMD;
    CollinearDPDF cDPDF("MSTW2008lo68cl_GSDPDF_PDFxTMD", 0);
    CollinearPDF cPDF("MSTW2008lo68cl", 0);
    PartonFlavor flavor1 = PartonFlavor::g;
    PartonFlavor flavor2 = PartonFlavor::g;
    double x1 = 0.464738;
    double x2 = 0.366379;
    double mu1_2 = 6.37085;
    double mu2_2 = 6.37085;
    auto res1 = cDPDF.dpdf(flavor1, flavor2, x1, mu1_2, x2, mu2_2);
    auto res2 = cPDF.pdf(flavor1, x1, mu1_2) * cPDF.pdf(flavor2, x2, mu2_2);
    std::cout << "DPDF value is: " << res1 << "--facotrized: " << res2 << std::endl;
    return 0;
}