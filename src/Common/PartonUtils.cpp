#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Common/ConfigWrapper.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/FileUtils.h"
#include <PDFxTMDLib/Common/Logger.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>

namespace fs = std::filesystem;
namespace PDFxTMD
{
const auto &instance = LibraryBanner::Instance();
std::string StandardPDFNaming(const std::string &pdfName, int set)
{
    return pdfName + "_" + std::to_string(set) + ".dat";
}

std::vector<std::string> splitPaths(const std::string &paths)
{
    std::vector<std::string> result;
    size_t start = 0, end;
    while ((end = paths.find(':', start)) != std::string::npos)
    {
        result.emplace_back(paths.substr(start, end - start));
        start = end + 1;
    }
    result.emplace_back(paths.substr(start));
    return result;
}
bool hasWriteAccess(const std::string &path)
{
    try
    {
        // Get the file status
        std::filesystem::file_status file_status = std::filesystem::status(path);

        // Check if the file exists
        if (!std::filesystem::exists(file_status))
        {
            PDFxTMDLOG << "File does not exist." << std::endl;
            return false;
        }

        // Get the permissions
        std::filesystem::perms permissions = file_status.permissions();

        // Check if the write permission is set for the owner
        if ((permissions & std::filesystem::perms::owner_write) != std::filesystem::perms::none)
        {
            return true;
        }

        // Optionally, check for group and others write permissions
        if ((permissions & std::filesystem::perms::group_write) != std::filesystem::perms::none ||
            (permissions & std::filesystem::perms::others_write) != std::filesystem::perms::none)
        {
            return true;
        }

        return false;
    }
    catch (const std::filesystem::filesystem_error &e)
    {
#if defined(ENABLE_LOG) && (ENABLE_LOG == 1)
        std::cerr << "Filesystem error: " << e.what() << std::endl;
#endif
        return false;
    }
    catch (const std::exception &e)
    {
#if defined(ENABLE_LOG) && (ENABLE_LOG == 1)
        std::cerr << "Error: " << e.what() << std::endl;
#endif
        return false;
    }
}
std::vector<std::string> GetPDFxTMDPathsAsVector()
{
    auto notDefaultPaths = GetPDFxTMDPathsFromYaml();
    if (!FileUtils::Exists(DEFAULT_ENV_PATH))
    {
        if (FileUtils::HasUserAccess(FileUtils::ParentDir(DEFAULT_ENV_PATH)))
        {
            FileUtils::CreateDirs(DEFAULT_ENV_PATH);
        }
    }
    std::set<std::string> result;
    result.emplace(DEFAULT_ENV_PATH);
    std::string currentPath = std::filesystem::current_path().string();
    result.emplace(currentPath);
    for (auto &&notDefaultPath : notDefaultPaths)
    {
        if (FileUtils::HasUserAccess(FileUtils::ParentDir(notDefaultPath)))
        {
            if (!FileUtils::Exists(notDefaultPath))
                FileUtils::CreateDirs(notDefaultPath);
        }
        result.emplace(notDefaultPath);
    }
    return std::vector(result.begin(), result.end());
}

std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}
std::string to_str_zeropad(int val, size_t nchars)
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(static_cast<int>(nchars)) << val;
    return ss.str();
}
std::pair<std::optional<std::string>, ErrorType> StandardPDFSetPath(const std::string &pdfSetName,
                                                                    int set)
{
    auto pdfPaths = GetPDFxTMDPathsAsVector();
    if (pdfPaths.size() == 0)
    {
        return {"", ErrorType::FILE_NOT_FOUND};
    }
    fs::path pdfPath_;
    std::string memname = pdfSetName + "_" + to_str_zeropad(set) + STD_PDF_DATA_EXTENSION;
    for (auto &&pdfPath : pdfPaths)
    {
        pdfPath_ = pdfPath + FOLDER_SEP + pdfSetName + FOLDER_SEP + memname;
        if (!fs::exists(pdfPath_))
        {
            continue;
        }

        return {pdfPath_.string(), ErrorType::None};
    }
    return {"", ErrorType::FILE_NOT_FOUND};
}
std::pair<std::optional<std::string>, ErrorType> StandardInfoFilePath(const std::string &pdfSetName)
{
    auto pdfPaths = GetPDFxTMDPathsAsVector();
    if (pdfPaths.size() == 0)
    {
        return {"", ErrorType::FILE_NOT_FOUND};
    }
    fs::path pdfPath_;
    for (auto &&pdfPath : pdfPaths)
    {
        pdfPath_ =
            pdfPath + FOLDER_SEP + pdfSetName + FOLDER_SEP + pdfSetName + STD_PDF_INFO_EXTENSION;
        if (!fs::exists(pdfPath_))
        {
            continue;
        }

        return {pdfPath_.string(), ErrorType::None};
    }
    return {"", ErrorType::FILE_NOT_FOUND};
}

double _extrapolateLinear(double x, double xl, double xh, double yl, double yh)
{
    if (yl > 1e-3 && yh > 1e-3)
    {
        // If yl and yh are sufficiently positive, keep y positive
        // by extrapolating log(y).
        return exp(std::log(yl) + (std::log(x) - std::log(xl)) / (std::log(xh) - std::log(xl)) *
                                      (std::log(yh) - std::log(yl)));
    }
    else
    {
        // Otherwise just extrapolate y itself.
        return yl + (std::log(x) - std::log(xl)) / (std::log(xh) - std::log(xl)) * (yh - yl);
    }
}

std::vector<std::string> GetPDFxTMDPathsFromYaml()
{
    std::string rootPath;
#if defined(_WIN32)
    rootPath = "C:/ProgramData/PDFxTMDLib";
#else

    const char *homeDir = std::getenv("HOME");
    if (!homeDir)
    {
        return {};
    }
    rootPath = std::string(homeDir) + "/.PDFxTMDLib";
#endif
    std::string configFilePath = rootPath + "/config.yaml";
    if (!std::filesystem::exists(rootPath) && !std::filesystem::create_directories(rootPath))
    {
        throw std::runtime_error("[PDFxTMD] The program is not functional. It needs path" +
                                 rootPath + " directory to be created.");
    }

    PDFxTMD::ConfigWrapper config;

    if (!std::filesystem::exists(configFilePath) || std::filesystem::file_size(configFilePath) == 0)
    {

        std::ofstream ofs(configFilePath);
        // Make the default PDF-set location visible in the generated config.
        // GetPDFxTMDPathsAsVector() also adds this path internally, and its
        // std::set removes the duplicate.
        ofs << "paths:" << std::endl;
        ofs << "  - " << DEFAULT_ENV_PATH << std::endl;
        ofs.close();
        return {};
    }
    if (!config.loadFromFile(configFilePath, PDFxTMD::ConfigWrapper::Format::YAML))
    {
        std::cerr << "[PDFxTMD] Warning: ignoring custom paths from '" << configFilePath
                  << "'. The default PDF-set path and current working directory will be used."
                  << std::endl;
        return {};
    }

    auto [paths, error] = config.get<std::vector<std::string>>("paths");
    if (error != ErrorType::None || !paths.has_value())
    {
        std::cerr << "[PDFxTMD] Warning: 'paths' in '" << configFilePath
                  << "' must be a YAML sequence. The default PDF-set path and current working "
                     "directory will be used."
                  << std::endl;
        return {};
    }
    return *paths;
}

bool AddPathToEnvironment(const std::string &newPath)
{
    std::string rootPath;
#if defined(_WIN32)
    rootPath = "C:/ProgramData/PDFxTMDLib";
#else

    const char *homeDir = std::getenv("HOME");
    if (!homeDir)
    {
        return false;
    }
    rootPath = std::string(homeDir) + "/.PDFxTMDLib";
#endif
    auto updatedPaths = GetPDFxTMDPathsFromYaml();
    auto found_pathItr =
        std::find_if(updatedPaths.begin(), updatedPaths.end(),
                     [newPath](const std::string &path) { return newPath == path; });
    if (found_pathItr == updatedPaths.end())
    {
        PDFxTMD::ConfigWrapper config;
        updatedPaths.push_back(newPath);
        config.set("paths", updatedPaths);
        std::string configFilePath = rootPath + "/config.yaml";
        return config.saveToFile(configFilePath);
    }
    return true;
}
} // namespace PDFxTMD
