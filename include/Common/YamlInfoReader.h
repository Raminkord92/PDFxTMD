#pragma once
#include <iostream>

#include "Common/ConfigWrapper.h"

namespace PDFxTMD
{
struct YamlStandardPDFInfo
{
    OrderQCD orderQCD;
    std::string Format;
    double XMin;
    double XMax;
    double QMin;
    double QMax;
    int NumMembers;
    std::vector<int> Flvors;
};

struct YamlStandardTMDInfo : YamlStandardPDFInfo
{
    double KtMin;
    double KtMax;
    std::string TMDScheme;
};
struct YamlImpelemntationInfo
{
    std::string reader;
    std::string interpolator;
    std::string extrapolator;
};
std::pair<std::optional<YamlImpelemntationInfo>, ErrorType> YamlImpelemntationInfoReader(
    const std::string &yamlInfoPath);
template <typename T>
std::enable_if_t<std::is_same_v<T, YamlStandardPDFInfo> || std::is_same_v<T, YamlStandardTMDInfo>,
                 std::pair<std::optional<YamlStandardTMDInfo>, ErrorType>>
YamlStandardPDFInfoReader(const std::string &yamlInfoPath)
{
    ConfigWrapper ConfigWrapper;
    YamlStandardTMDInfo output;
    if (!ConfigWrapper.loadFromFile(yamlInfoPath, ConfigWrapper::Format::YAML))
    {
        return {std::nullopt, ErrorType::FILE_NOT_FOUND};
    }
    //////OrderQCD
    auto [orderQCD, errorOrderQCD] = ConfigWrapper.get<int>("OrderQCD");
    if (errorOrderQCD != ErrorType::None)
    {
        std::cout << "[PDFxTMD][YamlInfoReader] OrderQCD not "
                     "found in yaml file"
                  << std::endl;
        return {std::nullopt, errorOrderQCD};
    }
    output.orderQCD = static_cast<OrderQCD>(*orderQCD);
    //////Format
    auto [format, errorFormat] = ConfigWrapper.get<std::string>("Format");
    if (errorFormat != ErrorType::None)
    {
        std::cout << "[PDFxTMD][YamlInfoReader] Format is not "
                     "found in yaml config file"
                  << std::endl;
        return {std::nullopt, errorFormat};
    }
    output.Format = *format;

    auto [TMDScheme, errorTMDScheme] = ConfigWrapper.get<std::string>("TMDScheme");
    if (errorFormat != ErrorType::None)
    {
        std::cout << "[PDFxTMD][YamlInfoReader] Format is not "
                     "found in yaml config file"
                  << std::endl;
        return {std::nullopt, errorTMDScheme};
    }
    output.TMDScheme = *TMDScheme;
    //////NumMembers
    auto [NumMembers, errorNumMembers] = ConfigWrapper.get<int>("NumMembers");
    if (errorNumMembers != ErrorType::None)
    {
        std::cout << "[PDFxTMD][YamlInfoReader] NumMembers is not "
                     "found in yaml config file"
                  << std::endl;
        return {std::nullopt, errorNumMembers};
    }
    output.NumMembers = *NumMembers;
    //////NumMembers
    auto [Flavors, errorFlavors] = ConfigWrapper.get<std::vector<int>>("Flavors");
    if (errorFlavors != ErrorType::None)
    {
        std::cout << "[PDFxTMD][YamlInfoReader] Flavors is not "
                     "found in yaml config file"
                  << std::endl;
        return {std::nullopt, errorFlavors};
    }
    output.Flvors = *Flavors;
    //////xMin
    auto [XMin, errorXMin] = ConfigWrapper.get<double>("XMin");
    if (errorXMin != ErrorType::None)
    {
        std::cout << "[PDFxTMD][YamlInfoReader] XMin is not found "
                     "in yaml config file"
                  << std::endl;
        return {std::nullopt, errorXMin};
    }
    output.XMin = *XMin;
    //////XMax
    auto [XMax, errorXMax] = ConfigWrapper.get<double>("XMax");
    if (errorXMax != ErrorType::None)
    {
        std::cout << "[PDFxTMD][YamlInfoReader] XMax is not found "
                     "in yaml config file"
                  << std::endl;
        return {std::nullopt, errorXMin};
    }
    output.XMax = *XMax;

    //////QMin
    auto [QMin, errorQMin] = ConfigWrapper.get<double>("QMin");
    if (errorQMin != ErrorType::None)
    {
        std::cout << "[PDFxTMD][YamlInfoReader] QMin is not found "
                     "in yaml config file"
                  << std::endl;
        return {std::nullopt, errorQMin};
    }
    output.QMin = *QMin;
    //////QMax
    auto [QMax, errorQMax] = ConfigWrapper.get<double>("QMax");
    if (errorQMax != ErrorType::None)
    {
        std::cout << "[PDFxTMD][YamlInfoReader] QMax is not found "
                     "in yaml config file"
                  << std::endl;
        return {std::nullopt, errorQMax};
    }
    output.QMax = *QMax;
    if constexpr (std::is_same_v<T, YamlStandardTMDInfo>)
    {
        //////KtMin
        auto [KtMin, errorKtMin] = ConfigWrapper.get<double>("KtMin");
        if (errorKtMin != ErrorType::None)
        {
            std::cout << "[PDFxTMD][YamlInfoReader] KtMin is not "
                         "found in yaml config file"
                      << std::endl;
            return {std::nullopt, errorKtMin};
        }
        output.KtMin = *KtMin;
        //////KtMax
        auto [KtMax, errorKtMax] = ConfigWrapper.get<double>("KtMax");
        if (errorKtMax != ErrorType::None)
        {
            std::cout << "[PDFxTMD][YamlInfoReader] KtMax is not "
                         "found in yaml config file"
                      << std::endl;
            return {std::nullopt, errorKtMax};
        }
        output.KtMax = *KtMax;
    }

    return {output, ErrorType::None};
}
} // namespace PDFxTMD
