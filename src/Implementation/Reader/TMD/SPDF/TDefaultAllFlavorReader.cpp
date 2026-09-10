#include "PDFxTMDLib/Implementation/Reader/TMD/SPDF/TDefaultAllFlavorReader.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h"
#include <fstream>
#include <set>

namespace PDFxTMD
{

DefaultAllFlavorTMDShape TDefaultAllFlavorReader::getData() const
{
    return m_updfShape;
}
std::pair<double, double> TDefaultAllFlavorReader::getBoundaryValues(PhaseSpaceComponent comp) const
{
    std::pair<double, double> output;
    switch (comp)
    {
    case PhaseSpaceComponent::X:
        output = {m_updfShape.x_vec.front(), m_updfShape.x_vec.back()};
        break;
    case PhaseSpaceComponent::Kt2:
        output = {m_updfShape.kt2_vec.front(), m_updfShape.kt2_vec.back()};
        break;
    case PhaseSpaceComponent::Q2:
        output = {m_updfShape.mu2_vec.front(), m_updfShape.mu2_vec.back()};
        break;
    default:
        throw NotSupportError("undefined Phase space component requested");
    }
    return output;
}
void TDefaultAllFlavorReader::read(const std::string &pdfName, int setNumber)
{
    auto filePathPair = StandardPDFSetPath(pdfName, setNumber);
    if (filePathPair.second != ErrorType::None)
    {
        throw FileLoadException("PDF set " + pdfName + " not found!");
    }

    auto infoPathPair = StandardInfoFilePath(pdfName);
    if (infoPathPair.second != ErrorType::None)
    {
        throw FileLoadException("Unable to find info file of PDF set " + pdfName);
    }

    auto updfStandardInfo = YamlStandardPDFInfoReader(*infoPathPair.first);
    if (updfStandardInfo.second != ErrorType::None)
    {
        throw InvalidFormatException("File " + *infoPathPair.first +
                                     " is not a standard info UPDF file");
    }

    YamlStandardTMDInfo standardUPDFInfo = *updfStandardInfo.first;
    bool isSchemeSupported = false;
    bool isFormatSupported = false;
    if (standardUPDFInfo.TMDScheme == "PB TMD-EW")
    {
        isSchemeSupported = true;
    }
    if (standardUPDFInfo.TMDScheme == "PB TMD")
    {
        isSchemeSupported = true;
    }

    if (!isSchemeSupported)
    {
        throw NotSupportError("TMDScheme " + standardUPDFInfo.TMDScheme +
                              " is currently not supported!");
    }
    if (standardUPDFInfo.Format == "allflavorUpdf")
    {
        isFormatSupported = true;
    }
    if (!isFormatSupported)
    {
        throw NotSupportError("Format " + standardUPDFInfo.Format + " is currently not supported!");
    }
    std::ifstream file(*filePathPair.first);
    if (!file.is_open())
    {
        throw PDFxTMD::FileLoadException("Unable to open file: " + *filePathPair.first);
    }

    // Read header lines
    std::string headerInfo[4];
    for (auto &line : headerInfo)
    {
        std::getline(file, line);
    }

    std::set<double> log_xSet, log_q2Set, log_pSet;
    double log_x = 0, log_q2 = 0, log_p = 0, tbar = 0, bbar = 0, cbar = 0, sbar = 0, ubar = 0,
           dbar = 0, g = 0, d = 0, u = 0, s = 0, c = 0, b = 0, t = 0, photon = 0, z0 = 0, wplus = 0,
           wminus = 0, higgs = 0;
    m_updfShape.grids[PartonFlavor::tbar].reserve(1000);
    m_updfShape.grids[PartonFlavor::bbar].reserve(1000);
    m_updfShape.grids[PartonFlavor::cbar].reserve(1000);
    m_updfShape.grids[PartonFlavor::sbar].reserve(1000);
    m_updfShape.grids[PartonFlavor::dbar].reserve(1000);
    m_updfShape.grids[PartonFlavor::ubar].reserve(1000);
    m_updfShape.grids[PartonFlavor::g].reserve(1000);
    m_updfShape.grids[PartonFlavor::t].reserve(1000);
    m_updfShape.grids[PartonFlavor::b].reserve(1000);
    m_updfShape.grids[PartonFlavor::c].reserve(1000);
    m_updfShape.grids[PartonFlavor::s].reserve(1000);
    m_updfShape.grids[PartonFlavor::d].reserve(1000);
    m_updfShape.grids[PartonFlavor::u].reserve(1000);

    while (file >> log_x >> log_q2 >> log_p >> tbar >> bbar >> cbar >> sbar >> ubar >> dbar >> g >>
           d >> u >> s >> c >> b >> t >> photon)
    {
        if (standardUPDFInfo.TMDScheme == "PB TMD-EW")
        {
            file >> z0 >> wplus >> wminus >> higgs;
        }
        else
        {
        }
        if (log_x == 0 || log_q2 == 0 || log_p == 0)
        {
            throw InvalidFormatException("Invalid data file, log(x), log(q2), and log(p) "
                                         "cannot be 0");
        }

        // Store unique log values
        log_xSet.insert(log_x);
        log_q2Set.insert(log_q2);
        log_pSet.insert(log_p);

        // Store data in shape
        m_updfShape.grids[PartonFlavor::tbar].push_back(tbar);
        m_updfShape.grids[PartonFlavor::bbar].push_back(bbar);
        m_updfShape.grids[PartonFlavor::cbar].push_back(cbar);
        m_updfShape.grids[PartonFlavor::sbar].push_back(sbar);
        m_updfShape.grids[PartonFlavor::ubar].push_back(ubar);
        m_updfShape.grids[PartonFlavor::dbar].push_back(dbar);
        m_updfShape.grids[PartonFlavor::g].push_back(g);
        m_updfShape.grids[PartonFlavor::gNS].push_back(g);
        m_updfShape.grids[PartonFlavor::d].push_back(d);
        m_updfShape.grids[PartonFlavor::u].push_back(u);
        m_updfShape.grids[PartonFlavor::s].push_back(s);
        m_updfShape.grids[PartonFlavor::c].push_back(c);
        m_updfShape.grids[PartonFlavor::b].push_back(b);
        m_updfShape.grids[PartonFlavor::t].push_back(t);
        m_updfShape.grids[PartonFlavor::photon].push_back(photon);
        if (standardUPDFInfo.TMDScheme == "PB TMD-EW")
        {
            m_updfShape.grids[PartonFlavor::z0].push_back(z0);
            m_updfShape.grids[PartonFlavor::wplus].push_back(wplus);
            m_updfShape.grids[PartonFlavor::wminus].push_back(wminus);
            m_updfShape.grids[PartonFlavor::higgs].push_back(higgs);
        }
    }

    m_updfShape.log_x_vec.assign(log_xSet.begin(), log_xSet.end());
    m_updfShape.log_kt2_vec.assign(log_q2Set.begin(), log_q2Set.end());
    auto log_pSetSize = log_pSet.size();
    m_updfShape.mu2_vec.reserve(log_pSetSize);
    for (auto logP : log_pSet)
    {
        double mu = std::exp(logP);
        m_updfShape.mu2_vec.push_back(mu * mu);
        m_updfShape.log_mu2_vec.push_back(2 * logP);
    }
    for (auto log_kt2 : log_q2Set)
    {
        m_updfShape.kt2_vec.emplace_back(std::exp(log_kt2));
    }
    for (auto log_x : log_xSet)
    {
        m_updfShape.x_vec.emplace_back(std::exp(log_x));
    }
}

} // namespace PDFxTMD