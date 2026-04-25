#include "PDFxTMDLib/Implementation/Reader/Collinear/SPDF/CDefaultLHAPDFFileReader.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h"
#include <fstream>
#include <string>

namespace PDFxTMD
{

std::vector<double> CDefaultLHAPDFFileReader::getValues(PhaseSpaceComponent comp) const
{
    std::vector<double> output;
    switch (comp)
    {
    case PhaseSpaceComponent::X:
        output = m_pdfShape_flat.x_vec;
        break;
    case PhaseSpaceComponent::Q2:
        output = m_pdfShape_flat.mu2_vec;
        break;
    default:
        throw NotSupportError("undefined Phase space component requested");
    }
    return output;
}

std::pair<double, double> CDefaultLHAPDFFileReader::getBoundaryValues(
    PhaseSpaceComponent comp) const
{
    switch (comp)
    {
    case PhaseSpaceComponent::X:
        return m_xMinMax;

    case PhaseSpaceComponent::Q2:
        return m_q2MinMax;

    default:
        throw NotSupportError("undefined Phase space component requested");
    }
}

void CDefaultLHAPDFFileReader::read(const std::string &pdfName, int setNumber)
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

    auto pdfStandardInfo = YamlStandardPDFInfoReader(*infoPathPair.first);
    if (pdfStandardInfo.second != ErrorType::None)
    {
        throw InvalidFormatException("File " + *infoPathPair.first +
                                     " is not a standard info UPDF file");
    }

    YamlStandardPDFInfo standardPDFInfo = *pdfStandardInfo.first;
    std::ifstream file(*filePathPair.first);
    if (!file.is_open())
    {
        throw PDFxTMD::FileLoadException("Unable to open file: " + *filePathPair.first);
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (isComment(line) || line.empty())
            continue;

        if (isBlockSeparator(line))
        {
            m_blockNumber++;
            m_blockLine = 0;
            if (m_blockNumber > 0)
            {
                // Pre-allocate with typical sizes
                DefaultAllFlavorShape data_;
                data_.log_x_vec.reserve(100);
                data_.log_mu2_vec.reserve(50);
                data_.x_vec.reserve(100);
                data_.mu2_vec.reserve(50);
                m_pdfShape.emplace_back(data_);
            }
            continue;
        }

        if (m_blockNumber == 0)
            continue;

        processDataLine(line, m_pdfShape[m_blockNumber - 1]);
        m_blockLine++;
    }
    // Remove any shape elements that have zero flavors
    m_pdfShape.erase(
        std::remove_if(m_pdfShape.begin(), m_pdfShape.end(),
                       [](const DefaultAllFlavorShape &shape) { return shape._pids.empty(); }),
        m_pdfShape.end());
    for (auto &pdfData_ : m_pdfShape)
    {
        pdfData_.finalizeXP2();
        m_mu2CompTotal.insert(m_mu2CompTotal.end(), pdfData_.mu2_vec.begin(),
                              pdfData_.mu2_vec.end());
    }
    // Flatten the PDF data for faster access
    size_t n_x = m_pdfShape[0].x_vec.size();
    size_t n_mu2 = m_mu2CompTotal.size();
    size_t n_flavors = m_pdfShape.at(0)._pids.size();
    m_pdfShape_flat.grids_flat.resize(n_mu2 * n_x * n_flavors, 0.0);
    m_pdfShape_flat._pids = m_pdfShape.at(0)._pids;
    m_pdfShape_flat.initPidLookup();
    // Initialize the flat structure
    m_pdfShape_flat.x_vec = m_pdfShape.at(0).x_vec;
    m_pdfShape_flat.mu2_vec = m_mu2CompTotal;
    m_pdfShape_flat.finalizeXP2();
    m_pdfShape_flat.n_flavors = m_pdfShape_flat._pids.size();
    // Copy data from the structured format to the flat array
    for (size_t ix = 0; ix < n_x; ++ix)
    {
        for (size_t iq2 = 0; iq2 < n_mu2; ++iq2)
        {
            // Find which PDF shape contains this mu2 value
            int shapeIndex = -1;
            size_t local_iq2 = 0; // Local index within the specific shape

            for (size_t s_ = 0; s_ < m_pdfShape.size(); ++s_)
            {
                const auto& mu2_vec_ = m_pdfShape[s_].mu2_vec;
                // Find the mu2 value in this shape
                auto it = std::find(mu2_vec_.begin(), mu2_vec_.end(),
                                    m_mu2CompTotal[iq2]);
                if (it != mu2_vec_.end())
                {
                    shapeIndex = s_;
                    local_iq2 = std::distance(mu2_vec_.begin(), it);
                    break;
                }
            }

            if (shapeIndex == -1)
            {
                continue; // Skip if we couldn't find this mu2 value in any shape
            }

            // For each flavor, copy the value to the flat array
            size_t iflavor = 0;
            for (auto flavor : m_pdfShape_flat._pids)
            {
                // Calculate flat index
                size_t flat_index = ix * n_mu2 * n_flavors + iq2 * n_flavors + iflavor;
                const auto& pids_ = m_pdfShape[shapeIndex]._pids;
                // Get value from the structured format if available
                if (std::find(pids_.begin(), pids_.end(),
                              flavor) != pids_.end() &&
                    ix < m_pdfShape[shapeIndex].x_vec.size())
                {
                    m_pdfShape_flat.grids_flat[flat_index] = m_pdfShape[shapeIndex].getGridFromMap(
                        static_cast<PartonFlavor>(flavor), ix, local_iq2);
                }

                iflavor++;
            }
        }
    }
    m_pdfShape_flat.grids.clear();
    // After processing all data, set the boundary values once
    const auto& x_vec_ = m_pdfShape[0].x_vec;
    if (!m_pdfShape.empty() && !x_vec_.empty())
    {
        m_xMinMax = {x_vec_.front(), x_vec_.back()};
    }

    if (!m_mu2CompTotal.empty())
    {
        m_q2MinMax = {m_mu2CompTotal.front(), m_mu2CompTotal.back()};
    }
    m_mu2CompTotal.clear();
    m_pdfShape.clear();
}

DefaultAllFlavorShape CDefaultLHAPDFFileReader::getData() const
{
    return m_pdfShape_flat;
}

void CDefaultLHAPDFFileReader::readXKnots(NumParser &parser, DefaultAllFlavorShape &data)
{
    double value;
    while (parser.hasMore())
    {
        parser >> value;
        data.x_vec.emplace_back(value);
    }

    if (data.x_vec.empty())
    {
        throw std::runtime_error("No x knots found in grid");
    }
}

void CDefaultLHAPDFFileReader::readQ2Knots(NumParser &parser, DefaultAllFlavorShape &data)
{
    double value;
    while (parser.hasMore())
    {
        parser >> value;
        data.mu2_vec.emplace_back(value * value); // Store Q²
    }

    if (data.mu2_vec.empty())
    {
        throw std::runtime_error("No Q² knots found in grid");
    }
}

void CDefaultLHAPDFFileReader::readParticleIds(NumParser &parser, DefaultAllFlavorShape &data)
{
    int id;
    while (parser.hasMore())
    {
        parser >> id;
        if (std::find(data._pids.begin(), data._pids.end(), id) == data._pids.end())
        {
            data._pids.emplace_back(id);
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

void CDefaultLHAPDFFileReader::readValues(NumParser &parser, DefaultAllFlavorShape &data)
{
    double value;
    while (parser.hasMore())
    {
        for (auto flavor : data._pids)
        {
            if (!(parser >> value))
            {
                continue;
            }
            data.grids[static_cast<PartonFlavor>(flavor)].push_back(value);
        }
    }
}

void CDefaultLHAPDFFileReader::processDataLine(const std::string &line, DefaultAllFlavorShape &data)
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
            readQ2Knots(parser, data);
            break;
        case 2:
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
