#pragma once
#include "PDFxTMDLib/Common/ConfigWrapper.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/PDFUtils.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Common/YamlInfoReader.h"
#include "PDFxTMDLib/Interface/IExtrapolator.h"
#include "PDFxTMDLib/Interface/IInterpolator.h"
#include "PDFxTMDLib/Interface/IReader.h"
#ifndef THIN_MODE_ENABLED
#include "PDFxTMDLib/MissingPDFSetHandler/PDFSetDownloadHandler.h"
#endif
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace PDFxTMD
{
struct TMDPDFTag
{
};
struct CollinearPDFTag
{
};

template <typename Tag, typename Reader, typename Interpolator, typename Extrapolator>
class GenericPDF
{
  public:
    GenericPDF(const std::string &pdfName, int setNumber)
        : m_pdfName(pdfName), m_setNumber(setNumber)
    {
        loadStandardInfo();
    }
    ~GenericPDF()
    {
    }
    double pdf(PartonFlavor flavor, double x, double mu2) const
    {
        if constexpr (std::is_same_v<Tag, CollinearPDFTag>)
        {
            return computeCollinearPDF(flavor, x, mu2);
        }
        else
        {
            throw std::logic_error(
                "pdf(PartonFlavor, double, double) is not supported for this tag.");
        }
    }

    double tmd(PartonFlavor flavor, double x, double kt2, double mu2)
    {
        if constexpr (std::is_same_v<Tag, TMDPDFTag>)
        {
            return computeTMD(flavor, x, kt2, mu2);
        }
        else
        {
            throw std::logic_error(
                "pdf(PartonFlavor, double, double) is not supported for this tag.");
        }
    }

    YamlStandardTMDInfo getStdPDFInfo() const
    {
        return m_stdInfo;
    }

  private:
    void loadStandardInfo()
    {
        auto infoPathPair = StandardInfoFilePath(m_pdfName);
        if (infoPathPair.second != ErrorType::None)
            throw FileLoadException("Unable to find info file of PDF set " + m_pdfName);
        if constexpr (std::is_same_v<Tag, TMDPDFTag>)
        {
            auto pdfStandardInfo =
                YamlStandardPDFInfoReader<YamlStandardTMDInfo>(*infoPathPair.first);
            if (pdfStandardInfo.second != ErrorType::None)
                throw InvalidFormatException("Invalid standard info file " + *infoPathPair.first);
            m_stdInfo = *pdfStandardInfo.first;
        }
        else if constexpr (std::is_same_v<Tag, CollinearPDFTag>)
        {
            auto pdfStandardInfo =
                YamlStandardPDFInfoReader<YamlStandardPDFInfo>(*infoPathPair.first);
            if (pdfStandardInfo.second != ErrorType::None)
                throw InvalidFormatException("Invalid standard info file " + *infoPathPair.first);
            m_stdInfo = *pdfStandardInfo.first;
        }
        else
        {
            static_assert(!std::is_same_v<Tag, Tag>, "Unsupported Tag");
        }
    }
    double computeCollinearPDF(PartonFlavor flavor, double x, double mu2) const
    {
        if (!m_dataLoaded)
        {
            loadData();
        }
        if (!isInRange(m_reader, x, mu2))
        {
            return m_extrapolator.extrapolate(flavor, x, mu2);
        }
        return m_interpolator.interpolate(flavor, x, mu2);
    }

    double computeTMD(PartonFlavor flavor, double x, double kt2, double mu2)
    {
        if (!m_dataLoaded)
        {
            loadData();
        }
        if (!isInRange(m_reader, x, kt2, mu2))
        {
            return m_extrapolator.extrapolate(flavor, x, kt2, mu2);
        }
        return m_interpolator.interpolate(flavor, x, kt2, mu2);
    }

    void loadData() const
    {
        try
        {
            auto &reader = const_cast<Reader &>(m_reader);
            reader.read(m_pdfName, m_setNumber);
            auto &interpolator = const_cast<Interpolator &>(m_interpolator);
            interpolator.initialize(&reader);
            // Update the extrapolator's interpolator pointer based on the tag type.
            if constexpr (std::is_same_v<Tag, CollinearPDFTag>)
            {
                if constexpr (std::is_base_of_v<
                                  IcAdvancedPDFExtrapolator<Extrapolator, Interpolator>,
                                  Extrapolator>)
                {
                    auto &extrapolator = const_cast<Extrapolator &>(m_extrapolator);
                    extrapolator.setInterpolator(&interpolator);
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error reading PDF data: " << e.what() << std::endl;
            throw;
        }
        m_dataLoaded = true;
    }

    std::string m_pdfName;
    int m_setNumber;
    Reader m_reader;
    Interpolator m_interpolator;
    Extrapolator m_extrapolator;
    mutable bool m_dataLoaded = false;
    YamlStandardTMDInfo m_stdInfo;
};
} // namespace PDFxTMD
