#include <PDFxTMDLib/Common/ConfigWrapper.h>
#include <PDFxTMDLib/Common/Exception.h>
#include <PDFxTMDLib/Common/StringUtils.h>
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlCouplingInfo.h>
#include <string>

namespace PDFxTMD
{
std::pair<std::optional<YamlCouplingInfo>, ErrorType> YamlCouplingInfoReader(
    const std::string &yamlInfoPath)
{
    ConfigWrapper ConfigWrapper;
    YamlCouplingInfo output;
    if (!ConfigWrapper.loadFromFile(yamlInfoPath, ConfigWrapper::Format::YAML))
    {
        return {std::nullopt, ErrorType::FILE_NOT_FOUND};
    }
    auto [mu_vec, mu_vecError] = ConfigWrapper.get<std::vector<double>>("AlphaS_Qs");
    if (mu_vecError == ErrorType::None)
    {
        output.mu_vec = std::move(*mu_vec);
    }
    auto [alphas_vec, alphas_vecError] = ConfigWrapper.get<std::vector<double>>("AlphaS_Vals");
    if (alphas_vecError == ErrorType::None)
    {
        output.alphas_vec = std::move(*alphas_vec);
    }

    auto [AlphaS_ThresholdDown, AlphaS_ThresholdDownError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdDown");
    auto [AlphaS_ThresholdUp, AlphaS_ThresholdUpError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdUp");
    auto [AlphaS_ThresholdStrange, AlphaS_ThresholdStrangeError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdStrange");
    auto [AlphaS_ThresholdCharm, AlphaS_ThresholdCharmError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdCharm");
    auto [AlphaS_ThresholdBottom, AlphaS_ThresholdBottomError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdBottom");
    auto [AlphaS_ThresholdTop, AlphaS_ThresholdTopError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdTop");
    if (all(AlphaS_ThresholdDownError == ErrorType::None,
            AlphaS_ThresholdUpError == ErrorType::None,
            AlphaS_ThresholdStrangeError == ErrorType::None,
            AlphaS_ThresholdCharmError == ErrorType::None,
            AlphaS_ThresholdBottomError == ErrorType::None,
            AlphaS_ThresholdTopError == ErrorType::None))
    {
        output.quarkThreshhold[PartonFlavor::d] = *AlphaS_ThresholdDown;
        output.quarkThreshhold[PartonFlavor::u] = *AlphaS_ThresholdUp;
        output.quarkThreshhold[PartonFlavor::s] = *AlphaS_ThresholdStrange;
        output.quarkThreshhold[PartonFlavor::c] = *AlphaS_ThresholdCharm;
        output.quarkThreshhold[PartonFlavor::b] = *AlphaS_ThresholdBottom;
        output.quarkThreshhold[PartonFlavor::t] = *AlphaS_ThresholdTop;
    }
    auto [ThresholdDown, ThresholdDownError] = ConfigWrapper.get<double>("ThresholdDown");
    auto [ThresholdUp, ThresholdUpError] = ConfigWrapper.get<double>("ThresholdUp");
    auto [ThresholdStrange, ThresholdStrangeError] = ConfigWrapper.get<double>("ThresholdStrange");
    auto [ThresholdCharm, ThresholdCharmError] = ConfigWrapper.get<double>("ThresholdCharm");
    auto [ThresholdBottom, ThresholdBottomError] = ConfigWrapper.get<double>("ThresholdBottom");
    auto [ThresholdTop, ThresholdTopError] = ConfigWrapper.get<double>("ThresholdTop");

    if (all(ThresholdDownError == ErrorType::None, ThresholdUpError == ErrorType::None,
            ThresholdStrangeError == ErrorType::None, ThresholdCharmError == ErrorType::None,
            ThresholdBottomError == ErrorType::None, ThresholdTopError == ErrorType::None))
    {
        output.quarkThreshhold[PartonFlavor::d] = *ThresholdDown;
        output.quarkThreshhold[PartonFlavor::u] = *ThresholdUp;
        output.quarkThreshhold[PartonFlavor::s] = *ThresholdStrange;
        output.quarkThreshhold[PartonFlavor::c] = *ThresholdCharm;
        output.quarkThreshhold[PartonFlavor::b] = *ThresholdBottom;
        output.quarkThreshhold[PartonFlavor::t] = *ThresholdTop;
    }

    bool isMasssObtained = false;
    auto [AlphaS_MDown, AlphaS_MDownError] = ConfigWrapper.get<double>("AlphaS_MDown");
    auto [AlphaS_MUp, AlphaS_MUpError] = ConfigWrapper.get<double>("AlphaS_MUp");
    auto [AlphaS_MStrange, AlphaS_MStrangeError] = ConfigWrapper.get<double>("AlphaS_MStrange");
    auto [AlphaS_MCharm, AlphaS_MCharmError] = ConfigWrapper.get<double>("AlphaS_MCharm");
    auto [AlphaS_MBottom, AlphaS_MBottomError] = ConfigWrapper.get<double>("AlphaS_MBottom");
    auto [AlphaS_MTop, AlphaS_MTopError] = ConfigWrapper.get<double>("AlphaS_MTop");
    if (all(AlphaS_MDownError == ErrorType::None, AlphaS_MUpError == ErrorType::None,
            AlphaS_MStrangeError == ErrorType::None, AlphaS_MCharmError == ErrorType::None,
            AlphaS_MBottomError == ErrorType::None, AlphaS_MTopError == ErrorType::None))
    {
        output.quarkMasses[PartonFlavor::d] = *AlphaS_MDown;
        output.quarkMasses[PartonFlavor::u] = *AlphaS_MUp;
        output.quarkMasses[PartonFlavor::s] = *AlphaS_MStrange;
        output.quarkMasses[PartonFlavor::c] = *AlphaS_MCharm;
        output.quarkMasses[PartonFlavor::b] = *AlphaS_MBottom;
        output.quarkMasses[PartonFlavor::t] = *AlphaS_MTop;
        isMasssObtained = true;
    }

    if (!isMasssObtained)
    {
        auto [MDown, MDownError] = ConfigWrapper.get<double>("MDown");
        auto [MUp, MUpError] = ConfigWrapper.get<double>("MUp");
        auto [MStrange, MStrangeError] = ConfigWrapper.get<double>("MStrange");
        auto [MCharm, MCharmError] = ConfigWrapper.get<double>("MCharm");
        auto [MBottom, MBottomError] = ConfigWrapper.get<double>("MBottom");
        auto [MTop, MTopError] = ConfigWrapper.get<double>("MTop");
        if (all(MDownError == ErrorType::None, MUpError == ErrorType::None,
                MStrangeError == ErrorType::None, MCharmError == ErrorType::None,
                MBottomError == ErrorType::None, MTopError == ErrorType::None))
        {
            output.quarkMasses[PartonFlavor::d] = *MDown;
            output.quarkMasses[PartonFlavor::u] = *MUp;
            output.quarkMasses[PartonFlavor::s] = *MStrange;
            output.quarkMasses[PartonFlavor::c] = *MCharm;
            output.quarkMasses[PartonFlavor::b] = *MBottom;
            output.quarkMasses[PartonFlavor::t] = *MTop;
            isMasssObtained = true;
        }
    }

    if (!isMasssObtained)
    {
        output.quarkMasses[PartonFlavor::d] = DOWN_DEFAULT_MASS;
        output.quarkMasses[PartonFlavor::u] = UP_DEFAULT_MASS;
        output.quarkMasses[PartonFlavor::s] = STRANGE_DEFAULT_MASS;
        output.quarkMasses[PartonFlavor::c] = CHARM_DEFAULT_MASS;
        output.quarkMasses[PartonFlavor::b] = BOTTOM_DEFAULT_MASS;
        output.quarkMasses[PartonFlavor::t] = TOP_DEFAULT_MASS;
    }

    bool isFlavorSchemeObtained = false;
    std::string flavorScheme = "";
    auto [AlphaS_FlavorScheme, AlphaS_FlavorSchemeError] =
        ConfigWrapper.get<std::string>("AlphaS_FlavorScheme");
    if (AlphaS_FlavorSchemeError == ErrorType::None)
    {
        flavorScheme = ToLower(*AlphaS_FlavorScheme);
        isFlavorSchemeObtained = true;
    }
    if (!isFlavorSchemeObtained)
    {
        auto [FlavorScheme, FlavorSchemeError] = ConfigWrapper.get<std::string>("FlavorScheme");
        if (FlavorSchemeError == ErrorType::None && FlavorScheme.has_value())
        {
            flavorScheme = ToLower(*FlavorScheme);
            isFlavorSchemeObtained = true;
        }
    }

    bool isNumFlavorsObtained = false;
    int numFlavors = DEFULT_NUM_FLAVORS;
    auto [AlphaS_NumFlavors, AlphaS_NumFlavorsError] = ConfigWrapper.get<int>("AlphaS_NumFlavors");
    if (AlphaS_NumFlavorsError == ErrorType::None)
    {
        numFlavors = *AlphaS_NumFlavors;
        isNumFlavorsObtained = true;
    }
    if (!isNumFlavorsObtained)
    {
        auto [NumFlavors, NumFlavorsError] = ConfigWrapper.get<int>("NumFlavors");
        if (NumFlavorsError == ErrorType::None && NumFlavors.has_value())
        {
            numFlavors = *NumFlavors;
            isNumFlavorsObtained = true;
        }
    }
    if (!isNumFlavorsObtained)
    {
        numFlavors = DEFULT_NUM_FLAVORS;
    }

    if (!isFlavorSchemeObtained)
    {
        flavorScheme = "variable";
    }
    output.flavorScheme =
        (flavorScheme == "variable" ? AlphasFlavorScheme::variable : AlphasFlavorScheme::fixed);
    output.AlphaS_NumFlavors = numFlavors;

    auto [MassZ, MassZError] = ConfigWrapper.get<double>("MZ");
    if (MassZError == ErrorType::None)
    {
        output.MZ = *MassZ;
    }

    auto [AlphaS_OrderQCD, AlphaS_OrderQCDError] = ConfigWrapper.get<int>("AlphaS_OrderQCD");
    if (AlphaS_OrderQCDError == ErrorType::None)
    {
        output.alphasOrder = (OrderQCD)*AlphaS_OrderQCD;
    }
    auto [AlphaS_Type, AlphaS_TypeError] = ConfigWrapper.get<std::string>("AlphaS_Type");
    if (AlphaS_TypeError == ErrorType::None)
    {
        if (AlphaS_Type == "ipol")
        {
            output.alphaCalcMethod = AlphasType::ipol;
            if (!all(mu_vecError == ErrorType::None, alphas_vecError == ErrorType::None))
            {
                throw MetadataError(
                    "Requested ipol AlphaS but the required parameters are not defined.");
            }
        }
        else if (AlphaS_Type == "analytic")
        {
            output.alphaCalcMethod = AlphasType::analytic;
            auto [alphasLambda3, alphasLambda3Error] = ConfigWrapper.get<double>("AlphaS_Lambda3");
            if (alphasLambda3Error == ErrorType::None)
            {
                output.alphasLambda3 = *alphasLambda3;
            }
            auto [alphasLambda4, alphasLambda4Error] = ConfigWrapper.get<double>("AlphaS_Lambda4");
            if (alphasLambda4Error == ErrorType::None)
            {
                output.alphasLambda4 = *alphasLambda4;
            }
            auto [alphasLambda5, alphasLambda5Error] = ConfigWrapper.get<double>("AlphaS_Lambda5");
            if (alphasLambda5Error == ErrorType::None)
            {
                output.alphasLambda5 = *alphasLambda5;
            }
            if (!all(alphasLambda3Error == ErrorType::None, alphasLambda4Error == ErrorType::None,
                     alphasLambda5Error == ErrorType::None))
            {
                throw MetadataError(
                    "Requested analytic AlphaS but the required parameters are not defined.");
            }
        }
        else if (AlphaS_Type == "ode")
        {
            output.alphaCalcMethod = AlphasType::ode;
            auto [AlphaS_MassReference, AlphaS_MassReferenceError] =
                ConfigWrapper.get<double>("AlphaS_MassReference");
            if (AlphaS_MassReferenceError == ErrorType::None)
            {
                output.AlphaS_MassReference = *AlphaS_MassReference;
            }
            auto [AlphaS_Reference, AlphaS_ReferenceError] =
                ConfigWrapper.get<double>("AlphaS_Reference");
            if (AlphaS_ReferenceError == ErrorType::None)
            {
                output.AlphaS_Reference = *AlphaS_Reference;
            }

            auto [AlphaS_MZ, AlphaS_MZError] = ConfigWrapper.get<double>("AlphaS_MZ");
            if (AlphaS_MZError == ErrorType::None)
            {
                output.AlphaS_MZ = *AlphaS_MZ;
            }
            if (!(AlphaS_MZError == ErrorType::None && MassZError == ErrorType::None) &&
                !(AlphaS_MassReferenceError == ErrorType::None &&
                  AlphaS_ReferenceError == ErrorType::None))
                throw MetadataError(
                    "Requested ODE AlphaS but there is no reference point given: define either "
                    "AlphaS_MZ and MZ, or AlphaS_MassReference and AlphaS_Reference. The latter is "
                    "given preference if both are defined.");
        }
        else
        {
            throw NotSupportError(*AlphaS_Type +
                                  " is currently not support for AlphaQCD calculation");
        }
    }
    return {output, ErrorType::None};
}

// int numFlavorsQ2(double q2, const YamlCouplingInfo& couplingInfo)
// {
//     if (couplingInfo.flavorScheme == AlphasFlavorScheme::fixed)
//         return couplingInfo.AlphaS_NumFlavors;
//     int nf = 0;
//     /// Use quark masses if flavour threshold not set explicitly
//     if (couplingInfo.quarkThreshhold.empty())
//     {
//         for (int it = 1; it <= 6; ++it)
//         {
//             auto element = couplingInfo.quarkMasses.find(static_cast<PartonFlavor>(it));
//             if (element == couplingInfo.quarkMasses.end())
//                 continue;
//             if (SQR(element->second) < q2)
//                 nf = it;
//         }
//     }
//     else
//     {
//         for (int it = 1; it <= 6; ++it)
//         {
//             auto element = couplingInfo.quarkThreshhold.find(static_cast<PartonFlavor>(it));
//             if (element == couplingInfo.quarkThreshhold.end())
//                 continue;
//             if (SQR(element->second) < q2)
//                 nf = it;
//         }
//     }
//     if (nf > couplingInfo.AlphaS_NumFlavors)
//         nf = couplingInfo.AlphaS_NumFlavors;
//     return nf;
// }
// Calculate the number of active quark flavours at energy scale Q2.
// Respects min/max nf
/// Currently returns the active number of flavors,
/// not the number actually used for lambdaQCD
/// (in case only lambda3 and lambda5 are defined and
/// we are in the 4 flavour range, we use lambda3 but this returns 4)
/// @todo Is this the "correct" behaviour?
int numFlavorsQ2(double q2, const YamlCouplingInfo &couplingInfo, int nfMin, int nfMax)
{
    if (couplingInfo.flavorScheme == AlphasFlavorScheme::fixed)
        return couplingInfo.AlphaS_NumFlavors;
    int nf = nfMin;
    /// Use quark masses if flavour threshold not set explicitly
    if (couplingInfo.quarkThreshhold.empty())
    {
        for (int it = nfMin; it <= nfMax; ++it)
        {
            auto element = couplingInfo.quarkMasses.find(static_cast<PartonFlavor>(it));
            if (element == couplingInfo.quarkMasses.end())
                continue;
            if (SQR(element->second) < q2)
                nf = it;
        }
    }
    else
    {
        for (int it = nfMin; it <= nfMax; ++it)
        {
            auto element = couplingInfo.quarkThreshhold.find(static_cast<PartonFlavor>(it));
            if (element == couplingInfo.quarkThreshhold.end())
                continue;
            if (SQR(element->second) < q2)
                nf = it;
        }
    }
    if (nf > couplingInfo.AlphaS_NumFlavors)
        nf = couplingInfo.AlphaS_NumFlavors;
    return nf;
}

// Calculate a beta function given the number of active flavours
double beta(OrderQCD qcdOrder, int nf)
{
    if (qcdOrder == OrderQCD::LO)
        return (double)0.875352187 - 0.053051647 * nf; //(33 - 2*nf)/(12*M_PI)
    if (qcdOrder == OrderQCD::NLO)
        return (double)0.6459225457 - 0.0802126037 * nf; //(153 - 19*nf)/(24*sqr(M_PI))
    if (qcdOrder == OrderQCD::N2LO)
        return (double)0.719864327 - 0.140904490 * nf +
               0.00303291339 * nf *
                   nf; //(2857 - (5033 / 9.0)*nf + (325 / 27.0)*sqr(nf))/(128*sqr(M_PI)*M_PI)
    if (qcdOrder == OrderQCD::N3LO)
        return (double)1.172686 - 0.2785458 * nf + 0.01624467 * nf * nf +
               0.0000601247 * nf * nf * nf;
    // ( (149753/6.) + 3564*ZETA_3 - ((1078361/162.) + (6502/27.)*ZETA_3)*nf +
    // ((50065/162.) + (6472/81.)*ZETA_3)*sqr(nf) +
    // (1093/729.)*sqr(nf)*nf)/(256*sqr(M_PI)*sqr(M_PI))
    if (qcdOrder == OrderQCD::N4LO)
        return (double)1.714138 - 0.5940794 * nf + 0.05607482 * nf * nf -
               0.0007380571 * nf * nf * nf - 0.00000587968 * nf * nf * nf * nf;
    // ... if you want the analytic form just look them up in hep-ph/1606.08659

    throw AlphaQCDError("Invalid index " + std::to_string((int)qcdOrder) +
                        " for requested beta function");
}

std::vector<double> betas(int nf)
{
    std::vector<double> rtn;
    rtn.reserve(4);
    for (int i = 0; i < 5; ++i)
        rtn.emplace_back(beta(static_cast<OrderQCD>(i), nf));
    return rtn;
}
} // namespace PDFxTMD
