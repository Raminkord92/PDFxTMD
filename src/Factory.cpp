#include "PDFxTMDLib/Factory.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlImpelemntationInfo.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h"
#include "PDFxTMDLib/GenericPDF.h"
#include "PDFxTMDLib/Implementation/Coupling/Analytic/AnalyticQCDCoupling.h"
#include "PDFxTMDLib/Implementation/Coupling/Interpolation/InterpolateQCDCoupling.h"
#include "PDFxTMDLib/Implementation/Coupling/ODE/ODEQCDCoupling.h"
#include "PDFxTMDLib/Implementation/Coupling/Null/NullQCDCoupling.h"
#include "PDFxTMDLib/Implementation/Extrapolator/Collinear/SPDF/CContinuationExtrapolator.h"
#include "PDFxTMDLib/Implementation/Extrapolator/Collinear/SPDF/CErrExtrapolator.h"
#include "PDFxTMDLib/Implementation/Extrapolator/Collinear/SPDF/CNearestPointExtrapolator.h"
#include "PDFxTMDLib/Implementation/Extrapolator/TMD/SPDF/TErrExtrapolator.h"
#include "PDFxTMDLib/Implementation/Extrapolator/TMD/SPDF/TZeroExtrapolator.h"
#include "PDFxTMDLib/Implementation/Interpolator/Collinear/SPDF/CLHAPDFBicubicInterpolator.h"
#include "PDFxTMDLib/Implementation/Interpolator/Collinear/SPDF/CLHAPDFBilinearInterpolator.h"
#include "PDFxTMDLib/Implementation/Interpolator/TMD/SPDF/TTrilinearInterpolator.h"
#include "PDFxTMDLib/Implementation/Interpolator/TMD/SPDF/TTrilinearTMDLibInterpolator.h"
#include "PDFxTMDLib/Implementation/Reader/Collinear/SPDF/CDefaultLHAPDFFileReader.h"
#include "PDFxTMDLib/Implementation/Reader/TMD/SPDF/TDefaultLHAPDF_TMDReader.h"
#include "PDFxTMDLib/Implementation/Reader/TMD/SPDF/TDefaultAllFlavorReader.h"
#ifdef PDFXTMD_HAS_DPD_HYBRID
#include "PDFxTMDLib/Implementation/DPD/Hybrid/CHybridDPD.h"
#endif
#include <memory>

namespace PDFxTMD
{

//////// start reader types
enum class TReader
{
    TDefaultLHAPDF_TMDReader,
    TDefaultTMDLibAllflavorReader
};

enum class CReader
{
    UNKNOWN,
    CDefaultLHAPDFFileReader
};

enum class CDReader
{
    UNKNOWN,
    CDefaultDPDReader
};


TReader TReaderType(const std::string &type)
{
    if (type == "TDefaultLHAPDF_TMDReader")
    {
        return TReader::TDefaultLHAPDF_TMDReader;
    }
    else if (type == "TDefaultTMDLibAllflavorReader")
    {
        return TReader::TDefaultTMDLibAllflavorReader;
    }
    throw NotSupportError("This file reader is not supported");
}

CReader CReaderType(const std::string &type)
{
    if (type == "CDefaultLHAPDFFileReader")
    {
        return CReader::CDefaultLHAPDFFileReader;
    }
    throw NotSupportError("This file reader is not supported");
}

CDReader CDReaderType(const std::string &type)
{
    if (type == "CDefaultDPDReader")
    {
        return CDReader::CDefaultDPDReader;
    }
    throw NotSupportError("This file reader is not supported");
}
///////end reader types
////// start interpolator type
enum class TInterpolator
{
    TTrilinearInterpolator,
    TTrilinearTMDLibInterpolator
};

// collinear supported interpolator type
enum class CInterpolator
{
    CLHAPDFBilinearInterpolator,
    CLHAPDFBicubicInterpolator,
};

enum class CDInterpolator
{
    CPDFxTMDDPDInterpolator,
};

CInterpolator CInterpolatorType(const std::string &type)
{
    if (type == "CLHAPDFBilinearInterpolator")
    {
        return CInterpolator::CLHAPDFBilinearInterpolator;
    }
    else if (type == "CLHAPDFBicubicInterpolator")
    {
        return CInterpolator::CLHAPDFBicubicInterpolator;
    }

    throw NotSupportError("This interpolator is not supported");
}

CDInterpolator CDInterpolatorType(const std::string &type)
{
    if (type == "CPDFxTMDDPDInterpolator")
    {
        return CDInterpolator::CPDFxTMDDPDInterpolator;
    }
    throw NotSupportError("This interpolator is not supported");
}

TInterpolator TInterpolatorType(const std::string &type)
{
    if (type == "TTrilinearInterpolator")
    {
        return TInterpolator::TTrilinearInterpolator;
    }
    else if (type == "TTrilinearTMDLibInterpolator")
    {
        return TInterpolator::TTrilinearTMDLibInterpolator;
    }
    throw NotSupportError("This interpolator is not supported");
}
////// end interpolator type
////// start extrapolator type
enum class CExtrapolator
{
    CContinuationExtrapolator,
    CErrExtrapolator,
    CNearestPointExtrapolator,
};

enum class TExtrapolator
{
    TZeroExtrapolator,
    TErrExtrapolator,
};

enum class CDExtrapolator
{
    CDPDZeroExtrapolator
};

CExtrapolator CExtrapolatorype(const std::string &type)
{
    if (type == "CContinuationExtrapolator")
    {
        return CExtrapolator::CContinuationExtrapolator;
    }
    else if (type == "CErrExtrapolator")
    {
        return CExtrapolator::CErrExtrapolator;
    }
    else if (type == "CNearestPointExtrapolator")
    {
        return CExtrapolator::CNearestPointExtrapolator;
    }

    throw NotSupportError("This extrapolator is not supported");
}

TExtrapolator TExtrapolatorype(const std::string &type)
{
    if (type == "TZeroExtrapolator")
    {
        return TExtrapolator::TZeroExtrapolator;
    }
    else if (type == "TErrExtrapolator")
    {
        return TExtrapolator::TErrExtrapolator;
    }
    throw NotSupportError("This extrapolator is not supported");
}
CDExtrapolator CDExtrapolatoraType(const std::string &type)
{
    if (type == "CDPDZeroExtrapolator")
    {
        return CDExtrapolator::CDPDZeroExtrapolator;
    }
    throw NotSupportError("This extrapolator is not supported");
}
////// end extrapolator type
ITMD GenericTMDFactory::mkTMD(const std::string &pdfSetName, int setMember)
{
    auto infoPathPair = StandardInfoFilePath(pdfSetName);
    if (infoPathPair.second != ErrorType::None)
    {
        throw FileLoadException("Unable to find info file of PDF set " + pdfSetName);
    }
    std::string format{};
    std::pair<std::optional<YamlStandardTMDInfo>, ErrorType> standardInfoPair = YamlStandardPDFInfoReader(*infoPathPair.first);
    if (standardInfoPair.second == ErrorType::None)
    {
        if (standardInfoPair.first.has_value())
        {
            YamlStandardTMDInfo stdInfo = *standardInfoPair.first;
            format = stdInfo.Format;
        }
    }
    if (format != "allflavorUpdf" && format != "lhagrid_tmd1")
    {
        throw NotSupportError("Format " + format + " is currently not supported");
    }
    TReader readerType;
    auto [impelmentationInfo, error] = YamlImpelemntationInfoReader(*infoPathPair.first);
    if (error != ErrorType::None || !impelmentationInfo.has_value())
    {
        throw InvalidFormatException("Unable to read implementation information from " +
                                     *infoPathPair.first);
    }
    const auto &implementationInfo = *impelmentationInfo;
    if (implementationInfo.reader == "")
    {
        if (format == "allflavorUpdf")
        {
            readerType = TReader::TDefaultTMDLibAllflavorReader;
        }
        else if (format == "lhagrid_tmd1")
        {
            readerType = TReader::TDefaultLHAPDF_TMDReader;
        }
    }
    else
    {
        readerType = TReaderType(implementationInfo.reader);
    }

    TInterpolator interpolatorType;
    if (implementationInfo.interpolator == "")
    {
        if (format == "allflavorUpdf")
        {
            interpolatorType = TInterpolator::TTrilinearTMDLibInterpolator;
        }
        else
        {
            interpolatorType = TInterpolator::TTrilinearInterpolator;
        }
    }
    else
    {
        interpolatorType = TInterpolatorType(implementationInfo.interpolator);
    }
    TExtrapolator extrapolatorType;
    if (implementationInfo.extrapolator == "")
    {
        extrapolatorType = TExtrapolator::TZeroExtrapolator;
    }
    else
    {
        extrapolatorType = TExtrapolatorype(implementationInfo.extrapolator);
    }

    if (readerType == TReader::TDefaultLHAPDF_TMDReader)
    {
        if (interpolatorType == TInterpolator::TTrilinearInterpolator)
        {
            if (extrapolatorType == TExtrapolator::TErrExtrapolator)
            {
                return ITMD(
                    GenericPDF<TMDPDFTag, TDefaultLHAPDF_TMDReader,
                               TTrilinearInterpolator<TDefaultLHAPDF_TMDReader>, TErrExtrapolator>(
                        pdfSetName, setMember));
            }
            else if (extrapolatorType == TExtrapolator::TZeroExtrapolator)
            {
                return ITMD(
                    GenericPDF<TMDPDFTag, TDefaultLHAPDF_TMDReader,
                               TTrilinearInterpolator<TDefaultLHAPDF_TMDReader>, TZeroExtrapolator>(
                        pdfSetName, setMember));
            }
        }
    }
    else if (readerType == TReader::TDefaultTMDLibAllflavorReader)
    {
        if (interpolatorType == TInterpolator::TTrilinearTMDLibInterpolator)
        {
            if (extrapolatorType == TExtrapolator::TErrExtrapolator)
            {
                return ITMD(GenericPDF<TMDPDFTag, TDefaultAllFlavorReader,
                                       TTrilinearTMDLibInterpolator<TDefaultAllFlavorReader>,
                                       TErrExtrapolator>(pdfSetName, setMember));
            }
            else if (extrapolatorType == TExtrapolator::TZeroExtrapolator)
            {
                return ITMD(GenericPDF<TMDPDFTag, TDefaultAllFlavorReader,
                                       TTrilinearTMDLibInterpolator<TDefaultAllFlavorReader>,
                                       TZeroExtrapolator>(pdfSetName, setMember));
            }
        }
    }
    throw NotSupportError("Not known combination of Reader, Interpolator, "
                          "Extrapolator is selected for this TMD");
}

ICPDF GenericCPDFFactory::mkCPDF(const std::string &pdfSetName, int setMember)
{
    auto infoPathPair = StandardInfoFilePath(pdfSetName);
    if (infoPathPair.second != ErrorType::None)
    {
        throw FileLoadException("Unable to find info file of PDF set " + pdfSetName);
    }
    std::string format{};
    std::pair<std::optional<YamlStandardTMDInfo>, ErrorType> standardInfoPair = YamlStandardPDFInfoReader(*infoPathPair.first);
    if (standardInfoPair.second == ErrorType::None)
    {
        if (standardInfoPair.first.has_value())
        {
            YamlStandardTMDInfo stdInfo = *standardInfoPair.first;
            format = stdInfo.Format;
        }
    }
    if (format != "lhagrid1")
    {
        throw NotSupportError("Format " + format + " is currently not supported");
    }
    CReader readerType;
    auto [impelmentationInfo, error] = YamlImpelemntationInfoReader(*infoPathPair.first);
    if (error != ErrorType::None || !impelmentationInfo.has_value())
    {
        throw InvalidFormatException("Unable to read implementation information from " +
                                     *infoPathPair.first);
    }
    const auto &implementationInfo = *impelmentationInfo;
    auto selectedReader = implementationInfo.reader;
    if (selectedReader == "")
    {
        readerType = CReader::CDefaultLHAPDFFileReader;
    }
    else
    {
        readerType = CReaderType(selectedReader);
    }
    CInterpolator interpolatorType;
    auto selectedInterpolator = implementationInfo.interpolator;
    if (selectedInterpolator == "")
    {
        interpolatorType = CInterpolator::CLHAPDFBicubicInterpolator;
    }
    else
    {
        interpolatorType = CInterpolatorType(selectedInterpolator);
    }
    CExtrapolator extrapolatorType;
    auto selectedExtrapolator = implementationInfo.extrapolator;
    if (selectedExtrapolator == "")
    {
        extrapolatorType = CExtrapolator::CContinuationExtrapolator;
    }
    else
    {
        extrapolatorType = CExtrapolatorype(selectedExtrapolator);
    }

    if (readerType == CReader::CDefaultLHAPDFFileReader)
    {
        if (interpolatorType == CInterpolator::CLHAPDFBicubicInterpolator)
        {
            if (extrapolatorType == CExtrapolator::CContinuationExtrapolator)
            {
                return ICPDF(
                    std::move(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                         CLHAPDFBicubicInterpolator<CDefaultLHAPDFFileReader>,
                                         CContinuationExtrapolator<CLHAPDFBicubicInterpolator<CDefaultLHAPDFFileReader>>>(
                        pdfSetName, setMember)));
            }
            else if (extrapolatorType == CExtrapolator::CErrExtrapolator)
            {
                return ICPDF(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                        CLHAPDFBicubicInterpolator<CDefaultLHAPDFFileReader>, CErrExtrapolator>(
                    pdfSetName, setMember));
            }
            else if (extrapolatorType == CExtrapolator::CNearestPointExtrapolator)
            {
                return ICPDF(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                        CLHAPDFBicubicInterpolator<CDefaultLHAPDFFileReader>,
                                        CNearestPointExtrapolator<CLHAPDFBicubicInterpolator<CDefaultLHAPDFFileReader>>>(
                    pdfSetName, setMember));
            }
        }
        else if (interpolatorType == CInterpolator::CLHAPDFBilinearInterpolator)
        {
            if (extrapolatorType == CExtrapolator::CContinuationExtrapolator)
            {
                return ICPDF(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                        CLHAPDFBilinearInterpolator<CDefaultLHAPDFFileReader>,
                                        CContinuationExtrapolator<CLHAPDFBilinearInterpolator<CDefaultLHAPDFFileReader>>>(
                    pdfSetName, setMember));
            }
            else if (extrapolatorType == CExtrapolator::CErrExtrapolator)
            {
                return ICPDF(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                        CLHAPDFBilinearInterpolator<CDefaultLHAPDFFileReader>, CErrExtrapolator>(pdfSetName,
                                                                                       setMember));
            }
            else if (extrapolatorType == CExtrapolator::CNearestPointExtrapolator)
            {
                return ICPDF(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                        CLHAPDFBilinearInterpolator<CDefaultLHAPDFFileReader>,
                                        CNearestPointExtrapolator<CLHAPDFBilinearInterpolator<CDefaultLHAPDFFileReader>>>(
                    pdfSetName, setMember));
            }
        }
    }

    throw NotSupportError("Not known combination of Reader, Interpolator, "
                          "Extrapolator is selected for this collinear PDF");
}

ICDPD GenericCDPDFactory::mkCDPD(const std::string &pdfSetName, int setMember)
{
#ifndef PDFXTMD_HAS_DPD
    throw NotSupportError("DPD support is disabled; rebuild with ENABLE_DPD=ON");
#else
    auto infoPathPair = StandardInfoFilePath(pdfSetName);
    if (infoPathPair.second != ErrorType::None)
    {
        throw FileLoadException("Unable to find info file of PDF set " + pdfSetName);
    }
    std::string format{};
    std::pair<std::optional<YamlStandardTMDInfo>, ErrorType> standardInfoPair =
        YamlStandardPDFInfoReader(*infoPathPair.first);
    if (standardInfoPair.second == ErrorType::None)
    {
        if (standardInfoPair.first.has_value())
        {
            YamlStandardTMDInfo stdInfo = *standardInfoPair.first;
            format = stdInfo.Format;
        }
    }
    if (format == "PDFxTMD-DPDH1")
    {
#ifdef PDFXTMD_HAS_DPD_HYBRID
        return ICDPD(CHybridDPD(pdfSetName, setMember));
#else
        throw NotSupportError(
            "Format PDFxTMD-DPDH1 requires a PDFxTMD build with hybrid DPD support");
#endif
    }
    if (format != "PDFxTMD-DPDB1")
    {
        throw NotSupportError("Format " + format + " is currently not supported for DPDs");
    }
    CDReader readerType;
    auto [impelmentationInfo, error] = YamlImpelemntationInfoReader(*infoPathPair.first);
    if (error != ErrorType::None || !impelmentationInfo.has_value())
    {
        throw InvalidFormatException("Unable to read implementation information from " +
                                     *infoPathPair.first);
    }
    const auto &implementationInfo = *impelmentationInfo;
    auto selectedReader = implementationInfo.reader;
    if (selectedReader == "")
    {
        readerType = CDReader::CDefaultDPDReader;
    }
    else
    {
        readerType = CDReaderType(selectedReader);
    }
    CDInterpolator interpolatorType;
    auto selectedInterpolator = implementationInfo.interpolator;
    if (selectedInterpolator == "")
    {
        interpolatorType = CDInterpolator::CPDFxTMDDPDInterpolator;
    }
    else
    {
        interpolatorType = CDInterpolatorType(selectedInterpolator);
    }
    CDExtrapolator extrapolatorType;
    auto selectedExtrapolator = implementationInfo.extrapolator;
    if (selectedExtrapolator == "")
    {
        extrapolatorType = CDExtrapolator::CDPDZeroExtrapolator;
    }
    else
    {
        extrapolatorType = CDExtrapolatoraType(selectedExtrapolator);
    }

    if (readerType == CDReader::CDefaultDPDReader)
    {
        if (interpolatorType == CDInterpolator::CPDFxTMDDPDInterpolator)
        {
            if (extrapolatorType == CDExtrapolator::CDPDZeroExtrapolator)
            {
                return ICDPD(std::move(
                    GenericPDF<CollinearDPDTag, CDefaultDPDReader,
                               CPDFxTMDDPDInterpolator<CDefaultDPDReader>, CDPDZeroExtrapolator>(
                        pdfSetName, setMember)));
            }
        }
    }

    throw NotSupportError("Not known combination of Reader, Interpolator, "
                          "Extrapolator is selected for this collinear PDF");
#endif
}

IQCDCoupling CouplingFactory::mkCoupling(const std::string &pdfSetName)
{
    std::pair<std::optional<std::string>, ErrorType> pdfSetInfo = StandardInfoFilePath(pdfSetName);
    YamlCouplingInfo couplingInfo_;
    if (pdfSetInfo.second != ErrorType::None)
    {
        throw FileLoadException("PDFset info " + pdfSetName + " not found");
    }
    std::pair<std::optional<YamlCouplingInfo>, ErrorType> couplingInfo =
        YamlCouplingInfoReader(*pdfSetInfo.first);
    if (couplingInfo.second != ErrorType::None || !couplingInfo.first.has_value())
    {
        throw std::runtime_error("Coupling info not found!");
    }

    couplingInfo_ = *couplingInfo.first;
    if (couplingInfo_.alphaCalcMethod == AlphasType::ipol)
    {
        InterpolateQCDCoupling interpCoupling = InterpolateQCDCoupling();
        interpCoupling.initialize(couplingInfo_);
        return IQCDCoupling(interpCoupling);
    }
    if (couplingInfo_.alphaCalcMethod == AlphasType::analytic)
    {
        AnalyticQCDCoupling analyticCoupling = AnalyticQCDCoupling();
        analyticCoupling.initialize(couplingInfo_);
        return IQCDCoupling(analyticCoupling);
    }
    if (couplingInfo_.alphaCalcMethod == AlphasType::ode)
    {
        ODEQCDCoupling odeQCDCoupling = ODEQCDCoupling();
        odeQCDCoupling.initialize(couplingInfo_);
        return IQCDCoupling(odeQCDCoupling);
    }
    if (couplingInfo_.alphaCalcMethod == AlphasType::None)
    {
        NullQCDCoupling nullQCDCoupling = NullQCDCoupling();
        return IQCDCoupling(nullQCDCoupling);
    }
    throw NotSupportError("The requested coupling approach is currently not supported");
}




} // namespace PDFxTMD
