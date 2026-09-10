#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace PDFxTMD
{
class FileUtils
{
  public:
    static bool HasUserAccess(const std::string &path);
    static double FreeSize(const std::string &path);
    static bool Exists(const std::string &path);
    static std::string ParentDir(const std::string &path);
    static bool CreateDirs(const std::string &path);
    static void readRaw(std::ifstream &in, void *ptr, size_t nbytes);
    static void writeRaw(std::ofstream &out, const void *ptr, size_t nbytes);
    static void writeU32(std::ofstream& out, uint32_t v);
    static void writeU16(std::ofstream &out, uint16_t v);
    static uint32_t readU32(std::ifstream &in);
    static uint16_t readU16(std::ifstream &in);
    static uint64_t readU64(std::ifstream &in);
    static void writeU64(std::ofstream &out, uint64_t v);
};
} // namespace PDFxTMD