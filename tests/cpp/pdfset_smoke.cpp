// Standalone native reference check for the Python set integration tests.
#include "PDFxTMDLib/PDFSet.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

void check(double a, double b)
{
    if (!std::isfinite(a) || !std::isfinite(b) ||
        std::abs(a - b) > 1e-12 * std::max(1.0, std::abs(b)))
        throw std::runtime_error("PDFSet/factory numerical mismatch");
}

template <typename Tag> void test(const std::string &name)
{
    using namespace PDFxTMD;
    PDFSet<Tag> group(name);
    std::vector<double> values;
    for (std::size_t i = 0; i < group.size(); ++i)
    {
        if constexpr (std::is_same_v<Tag, CollinearPDFTag>)
        {
            auto direct = GenericCPDFFactory().mkCPDF(name, i);
            values.push_back(group[i]->pdf(PartonFlavor::g, 0.01, 100));
            check(values.back(), direct.pdf(PartonFlavor::g, 0.01, 100));
        }
        else
        {
            auto direct = GenericTMDFactory().mkTMD(name, i);
            values.push_back(group[i]->tmd(PartonFlavor::g, 0.01, 4, 100));
            check(values.back(), direct.tmd(PartonFlavor::g, 0.01, 4, 100));
        }
    }
    PDFUncertainty point;
    if constexpr (std::is_same_v<Tag, CollinearPDFTag>)
        point = group.Uncertainty(PartonFlavor::g, 0.01, 100);
    else
        point = group.Uncertainty(PartonFlavor::g, 0.01, 4, 100);
    auto vector = group.Uncertainty(values);
    check(point.central, vector.central);
    check(point.errplus, vector.errplus);
    check(point.errminus, vector.errminus);
    check(group.Correlation(values, values), 1.0);
    check(group.alphasQ2(100), CouplingFactory().mkCoupling(name).AlphaQCDMu2(100));
    std::cout << "PASS " << name << " members=" << group.size() << '\n';
}

int main(int argc, char **argv)
{
    if (argc != 3) return 2;
    try
    {
        if (std::string(argv[1]) == "CPDF") test<PDFxTMD::CollinearPDFTag>(argv[2]);
        else if (std::string(argv[1]) == "TMD") test<PDFxTMD::TMDPDFTag>(argv[2]);
        else return 2;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
