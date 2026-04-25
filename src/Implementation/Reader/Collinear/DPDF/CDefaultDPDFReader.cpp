#include "PDFxTMDLib/Implementation/Reader/Collinear/DPDF/CDefaultDPDFReader.h"

#include "PDFxTMDLib/Common/Constants.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/FileUtils.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h"
#include "PDFxTMDLib/Implementation/Reader/Collinear/SPDF/CDefaultLHAPDFFileReader.h"

#include <zstd.h>
#include <zstd_errors.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace PDFxTMD
{

namespace
{
constexpr uint32_t kNRegular = 36;
constexpr uint32_t kNJjb = 3;

uint32_t checkedNtSq(uint32_t nt)
{
    const uint64_t n = static_cast<uint64_t>(nt);
    const uint64_t nn = n * n;

    if (nn > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()))
        throw std::runtime_error("nt^2 does not fit into uint32_t");

    return static_cast<uint32_t>(nn);
}

void validateMagic(const std::array<char, 16> &magic)
{
    if (!(magic[0] == 'P' && magic[1] == 'D' && magic[2] == 'F' &&
          magic[3] == 'x' && magic[4] == 'T' && magic[5] == 'M' &&
          magic[6] == 'D' && magic[7] == '-' &&
          magic[8] == 'D' && magic[9] == 'P' && magic[10] == 'D' &&
          magic[11] == 'F' && magic[12] == 'B' && magic[13] == '1'))
    {
        throw std::runtime_error("not a PDFxTMD-DPDFB1 file");
    }
}

} // namespace

void CDefaultDPDFReader::read(const std::string &pdfName, int setNumber)
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
    std::ifstream file(*filePathPair.first, std::ios::binary);
    if (!file.is_open())
    {
        throw FileLoadException("Unable to open file: " + *filePathPair.first);
    }

    m_gridData = UnEqualScaleGridData{};

    // magic -------------------------------------------------------------
    std::array<char, 16> magic{};
    FileUtils::readRaw(file, magic.data(), magic.size());
    validateMagic(magic);

    // header ------------------------------------------------------------
    const uint32_t version = FileUtils::readU32(file);
    const uint32_t nx = FileUtils::readU32(file);
    const uint32_t nt = FileUtils::readU32(file);
    const uint32_t npartons = FileUtils::readU32(file);
    const uint32_t nphys = FileUtils::readU32(file);
    static_cast<void>(FileUtils::readU32(file)); // reserved

    if (version != 1)
        throw std::runtime_error("unsupported PDFxTMD-DPDFB1 version");

    if (npartons != UnEqualScaleGridData::kNPartons)
        throw std::runtime_error("unexpected npartons in PDFxTMD-DPDFB1");

    if (nx < 2 || nt < 2)
        throw std::runtime_error("grid too small");

    // grids -------------------------------------------------------------
    m_gridData.xs.resize(nx);
    m_gridData.mus.resize(nt);

    FileUtils::readRaw(file, m_gridData.xs.data(), sizeof(double) * nx);
    FileUtils::readRaw(file, m_gridData.mus.data(), sizeof(double) * nt);

    // physical x-pairs --------------------------------------------------
    m_gridData.phys_pairs.resize(nphys);

    for (uint32_t p = 0; p < nphys; ++p)
    {
        m_gridData.phys_pairs[p].i = FileUtils::readU16(file);
        m_gridData.phys_pairs[p].j = FileUtils::readU16(file);

        if (m_gridData.phys_pairs[p].i >= nx || m_gridData.phys_pairs[p].j >= nx)
            throw std::runtime_error("pair index out of range in DPDFB1");
    }

    // rebuild pair_lut --------------------------------------------------
    m_gridData.pair_lut.assign(static_cast<size_t>(nx) * nx, -1);

    for (uint32_t p = 0; p < nphys; ++p)
    {
        const auto &ij = m_gridData.phys_pairs[p];

        m_gridData.pair_lut[static_cast<size_t>(ij.i) * nx + ij.j] =
            static_cast<int32_t>(p);
    }

    // payload -----------------------------------------------------------
    //
    // New DPDFB1 layout:
    //   scale_id = k1 * nt + k2
    //   plane    = nt * nt * nphys
    //
    // Payload is stored as float, compressed by zstd:
    //   reg[36][plane]
    //   jjb[3][plane]
    //
    const uint32_t ntsq = checkedNtSq(nt);
    const size_t plane =
        static_cast<size_t>(ntsq) * static_cast<size_t>(nphys);

    const size_t reg_sz =
        static_cast<size_t>(kNRegular) * plane;

    const size_t jjb_sz =
        static_cast<size_t>(kNJjb) * plane;

    const size_t total_sz = reg_sz + jjb_sz;
    const size_t raw_bytes = sizeof(float) * total_sz;

    const uint64_t payload_bytes_u64 = FileUtils::readU64(file);
    if (payload_bytes_u64 > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
        throw std::runtime_error("payload too large");

    const size_t payload_bytes = static_cast<size_t>(payload_bytes_u64);

    std::vector<uint8_t> comp(payload_bytes);
    FileUtils::readRaw(file, comp.data(), payload_bytes);

    std::vector<float> payload(total_sz);

    const size_t dsize = ZSTD_decompress(
        reinterpret_cast<void *>(payload.data()),
        raw_bytes,
        comp.data(),
        payload_bytes);

    if (ZSTD_isError(dsize))
    {
        throw std::runtime_error(
            std::string("zstd decompression failed: ") +
            ZSTD_getErrorName(dsize));
    }

    if (dsize != raw_bytes)
        throw std::runtime_error("zstd decompressed size mismatch");

    // split payload into reg/jjb ----------------------------------------
    m_gridData.reg.resize(kNRegular);
    m_gridData.jjb.resize(kNJjb);

    for (size_t ch = 0; ch < kNRegular; ++ch)
    {
        m_gridData.reg[ch].resize(plane);

        const size_t off = ch * plane;
        for (size_t n = 0; n < plane; ++n)
        {
            m_gridData.reg[ch][n] =
                static_cast<double>(payload[off + n]);
        }
    }

    const size_t jjb_offset =
        static_cast<size_t>(kNRegular) * plane;

    for (size_t ch = 0; ch < kNJjb; ++ch)
    {
        m_gridData.jjb[ch].resize(plane);

        const size_t off = jjb_offset + ch * plane;
        for (size_t n = 0; n < plane; ++n)
        {
            m_gridData.jjb[ch][n] =
                static_cast<double>(payload[off + n]);
        }
    }

    // interpolation metadata --------------------------------------------
    m_gridData.xmax = m_gridData.xs.back();

    m_gridData.nmcor =
        UnEqualScaleGridData::locateBracket(
            m_gridData.xs,
            1.0 - m_gridData.xmax + eps8) +
        1;

    if (m_gridData.nmcor < 2)
    {
        throw std::runtime_error(
            "corner interpolation setup failed: xmax is too close to 1");
    }

    m_xMinMax = {m_gridData.xs.front(), m_gridData.xs.back()};

    // mus stores t = log(mu^2), so Q2 = exp(t).
    m_gridData.mu2s.resize(m_gridData.mus.size());

    std::transform(
        m_gridData.mus.begin(),
        m_gridData.mus.end(),
        m_gridData.mu2s.begin(),
        [](double t) {
            return std::exp(t);
        });

    m_q2MinMax = {m_gridData.mu2s.front(), m_gridData.mu2s.back()};
}

UnEqualScaleGridData CDefaultDPDFReader::getData() const
{
    return m_gridData;
}

std::vector<double> CDefaultDPDFReader::getValues(PhaseSpaceComponent comp) const
{
    std::vector<double> output;

    switch (comp)
    {
    case PhaseSpaceComponent::X:
        output = m_gridData.xs;
        break;

    case PhaseSpaceComponent::Q2:
        output = m_gridData.mu2s;
        break;

    default:
        throw NotSupportError("undefined Phase space component requested");
    }

    return output;
}

std::pair<double, double> CDefaultDPDFReader::getBoundaryValues(
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

} // namespace PDFxTMD