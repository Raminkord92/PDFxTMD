#include "PDFxTMDLib/Implementation/Reader/TMD/SPDF/TDefaultLHAPDF_TMDReader.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h"
#include <fstream>
#include <set>

namespace PDFxTMD
{
std::vector<double> TDefaultLHAPDF_TMDReader::getValues(PhaseSpaceComponent comp) const
{
    std::vector<double> output;
    switch (comp)
    {
    case PhaseSpaceComponent::X:
        output = m_pdfShape.x_vec;
        break;
    case PhaseSpaceComponent::Q2:
        output = m_pdfShape.mu2_vec;
        break;
    case PhaseSpaceComponent::Kt2:
        output = m_pdfShape.kt2_vec;
        break;
    default:
        throw NotSupportError("undefined Phase space component requested");
    }
    return output;
}

std::pair<double, double> TDefaultLHAPDF_TMDReader::getBoundaryValues(
    PhaseSpaceComponent comp) const
{
    switch (comp)
    {
    case PhaseSpaceComponent::X:
        return m_xMinMax;
    case PhaseSpaceComponent::Q2:
        return m_q2MinMax;
    case PhaseSpaceComponent::Kt2:
        return m_kt2MinMax;
    default:
        throw NotSupportError("undefined Phase space component requested");
    }
}

void TDefaultLHAPDF_TMDReader::read(const std::string &pdfName, int setNumber)
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
    std::ifstream file(*filePathPair.first);
    if (!file.is_open())
    {
        throw PDFxTMD::FileLoadException("Unable to open file: " + *filePathPair.first);
    }

    std::string line;
    DefaultAllFlavorTMDShape data_;
    m_pdfShape.x_vec.reserve(51);
    m_pdfShape.mu2_vec.reserve(51);
    m_pdfShape.kt2_vec.reserve(51);
    while (std::getline(file, line))
    {
        if (isComment(line) || line.empty())
            continue;

        if (isBlockSeparator(line))
        {
            m_blockNumber++;
            if (m_blockNumber > 1)
            {
                throw NotSupportError("[PDFxTMD] Currently only single subgrid is supported.");
            }
            m_blockLine = 0;
            if (m_blockNumber > 0)
            {
            }
            continue;
        }

        if (m_blockNumber == 0)
            continue;

        processDataLine(line, m_pdfShape);
        m_blockLine++;
    }

    for (auto partonFlavor : standardPartonFlavors)
    {
        if (m_pdfShape.grids[partonFlavor].size() == 0)
        {
            m_pdfShape.grids[partonFlavor].resize(
                m_pdfShape.x_vec.size() * m_pdfShape.kt2_vec.size() * m_pdfShape.mu2_vec.size(),
                0.);
        }
    }
    m_pdfShape.finalizeXKt2P2();
    m_xMinMax = {m_pdfShape.x_vec.front(), m_pdfShape.x_vec.back()};
    m_q2MinMax = {m_pdfShape.mu2_vec.front(), m_pdfShape.mu2_vec.back()};
    m_kt2MinMax = {m_pdfShape.kt2_vec.front(), m_pdfShape.kt2_vec.back()};
}

DefaultAllFlavorTMDShape TDefaultLHAPDF_TMDReader::getData() const
{
    return m_pdfShape;
}

void TDefaultLHAPDF_TMDReader::readXKnots(NumParser &parser, DefaultAllFlavorTMDShape &data)
{
    double value;
    while (parser.hasMore())
    {
        if (parser >> value)
            data.x_vec.emplace_back(value);
    }

    if (data.x_vec.empty())
    {
        throw std::runtime_error("No x knots found in grid");
    }
}

void TDefaultLHAPDF_TMDReader::readQ2Knots(NumParser &parser, DefaultAllFlavorTMDShape &data)
{
    double value;
    while (parser.hasMore())
    {
        if (parser >> value)
            data.mu2_vec.emplace_back(value * value);
    }

    if (data.mu2_vec.empty())
    {
        throw std::runtime_error("No Q² knots found in grid");
    }
}
void TDefaultLHAPDF_TMDReader::readKt2Knots(NumParser &parser, DefaultAllFlavorTMDShape &data)
{
    double value;
    while (parser.hasMore())
    {
        if (parser >> value)
            data.kt2_vec.emplace_back(value * value);
    }

    if (data.kt2_vec.empty())
    {
        throw std::runtime_error("No Q² knots found in grid");
    }
}

void TDefaultLHAPDF_TMDReader::readParticleIds(NumParser &parser, DefaultAllFlavorTMDShape &data)
{
    int id;
    while (parser.hasMore())
    {
        if (parser >> id)
        {
            if (std::find(data._pids.begin(), data._pids.end(), id) == data._pids.end())
            {
                data._pids.emplace_back(id);
            }
        }
    }

    if (data._pids.empty())
    {
        throw std::runtime_error("No particle IDs found in grid");
    }
    size_t gridSize = data.x_vec.size() * data.mu2_vec.size();
    for (const auto &flavor : data._pids)
    {
        data.grids[static_cast<PartonFlavor>(flavor)].reserve(
            data.grids[static_cast<PartonFlavor>(flavor)].size() + gridSize);
    }
}

void TDefaultLHAPDF_TMDReader::readValues(NumParser &parser, DefaultAllFlavorTMDShape &data)
{
    double value;
    while (parser.hasMore())
    {
        for (auto flavor : data._pids)
        {
            if (parser >> value)
            {
                data.grids[static_cast<PartonFlavor>(flavor)].push_back(value);
            }
        }
    }
}

void TDefaultLHAPDF_TMDReader::processDataLine(const std::string &line,
                                               DefaultAllFlavorTMDShape &data)
{
    NumParser parser(line);

    try
    {
        switch (m_blockLine)
        {
        case 0:
            readXKnots(parser, data);
            break;
        case 1:
            readKt2Knots(parser, data);
            break;
        case 2:
            readQ2Knots(parser, data);
            break;
        case 3:
            readParticleIds(parser, data);
            break;
        default:
            readValues(parser, data);
            break;
        }
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Error in block " + std::to_string(m_blockNumber) + ", line " +
                                 std::to_string(m_blockLine) + ": " + e.what());
    }
}

} // namespace PDFxTMD