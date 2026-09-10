#include "PDFxTMDLib/Common/FileUtils.h"
namespace PDFxTMD
{
bool FileUtils::HasUserAccess(const std::string &path)
{
    try
    {
        // Create a test file to verify write permissions
        auto testPath = std::filesystem::path(path) / ".write_test";
        std::ofstream file(testPath);
        bool hasAccess = file.is_open();
        file.close();
        if (std::filesystem::exists(testPath))
        {
            std::filesystem::remove(testPath);
        }
        return hasAccess;
    }
    catch (const std::filesystem::filesystem_error &)
    {
        return false;
    }
}

double FileUtils::FreeSize(const std::string &path)
{
    std::error_code ec;
    auto space = std::filesystem::space(path, ec);

    if (ec)
    {
        return -1.0; // Return negative value to indicate error
    }

    // Convert bytes to GB (1 GB = 1024^3 bytes)
    return static_cast<double>(space.available) / (1024 * 1024 * 1024);
}
bool FileUtils::Exists(const std::string &path)
{
    return std::filesystem::exists(path);
}

std::string FileUtils::ParentDir(const std::string &path)
{
    std::filesystem::path fsPath(path);
    return fsPath.parent_path().string();
}

bool FileUtils::CreateDirs(const std::string &path)
{
    std::error_code ec;
    return std::filesystem::create_directories(path, ec);
}
void FileUtils::readRaw(std::ifstream &in, void *ptr, size_t nbytes)
{
    in.read(reinterpret_cast<char *>(ptr), static_cast<std::streamsize>(nbytes));
    if (!in)
    {
        throw std::runtime_error("binary read failed");
    }
}
void FileUtils::writeRaw(std::ofstream &out, const void *ptr, size_t nbytes)
{
    out.write(reinterpret_cast<const char *>(ptr), static_cast<std::streamsize>(nbytes));
    if (!out)
    {
        throw std::runtime_error("binary write failed");
    }
}
uint16_t FileUtils::readU16(std::ifstream &in)
{
  uint16_t v = 0;
  readRaw(in, &v, sizeof(v));
  return v;
}
uint32_t FileUtils::readU32(std::ifstream &in)
{
    uint32_t v = 0;
    readRaw(in, &v, sizeof(v));
    return v;
}
void FileUtils::writeU16(std::ofstream &out, uint16_t v)
{
    writeRaw(out, &v, sizeof(v));
}
void FileUtils::writeU32(std::ofstream &out, uint32_t v)
{
    writeRaw(out, &v, sizeof(v));
}
uint64_t FileUtils::readU64(std::ifstream &in)
{
  uint64_t v = 0;
  readRaw(in, &v, sizeof(v));
  return v;
}
void FileUtils::writeU64(std::ofstream &out, uint64_t v)
{
    writeRaw(out, &v, sizeof(v));
}
} // namespace PDFxTMD