#define RYML_SINGLE_HDR_DEFINE_NOW
#include "PDFxTMDLib/external/rapidyaml/rapidyaml-0.9.0.hpp"
#include "PDFxTMDLib/Common/ConfigWrapper.h"
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace
{
[[noreturn]] void throwYamlError(const char *message, size_t length, ryml::Location location,
                                 void *)
{
    std::string error(message, length);
    while (!error.empty() && (error.back() == '\n' || error.back() == '\r'))
    {
        error.pop_back();
    }
    if (location)
    {
        error += " (line " + std::to_string(location.line) + ", column " +
                 std::to_string(location.col) + ")";
    }
    throw std::runtime_error(error);
}

ryml::Tree parseYaml(ryml::csubstr yaml)
{
    ryml::Callbacks callbacks;
    callbacks.m_error = &throwYamlError;
    ryml::Parser::handler_type eventHandler(callbacks);
    ryml::Parser parser(&eventHandler);
    return ryml::parse_in_arena(&parser, yaml);
}
} // namespace

namespace PDFxTMD
{
// Your saveToFile is correct, no changes needed here.
bool ConfigWrapper::saveToFile(const std::string &filename) const
{
    std::ofstream ofs(filename);
    if (!ofs.is_open())
    {
        return false;
    }
    try
    {
        ofs << data.tree;
        ofs.close();
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

bool ConfigWrapper::loadFromString(const std::string &data_string, Format format)
{
    if (format == Format::YAML)
    {
        try
        {
            // FIX: Use parse_in_arena to copy the data into the tree's own memory.
            // We can parse directly from the string without needing a separate buffer.
            data.tree = parseYaml(ryml::to_csubstr(data_string));

            if (!data.tree.rootref().is_map())
            {
                initializeEmptyYAML();
                return false;
            }

            data.format = format;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "[PDFxTMD] Warning: unable to parse YAML data: " << e.what()
                      << std::endl;
            initializeEmptyYAML();
            return false;
        }
    }
    return false;
}

bool ConfigWrapper::loadFromFile(const std::filesystem::path &filepath, Format format)
{
    if (format == Format::YAML)
    {
        std::ifstream ifs(filepath, std::ios::binary);
        if (!ifs.is_open())
        {
            return false;
        }

        std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)),
                                 std::istreambuf_iterator<char>());

        // No need to null-terminate when passing size, but it doesn't hurt.

        try
        {
            // FIX: Use parse_in_arena to copy the data into the tree's own memory.
            data.tree = parseYaml(ryml::csubstr(buffer.data(), buffer.size()));

            if (!data.tree.rootref().is_map())
            {
                initializeEmptyYAML();
                return false;
            }

            // NOTE: This block below is strange. It modifies the input file on a read
            // operation if it's empty, which is unexpected. Consider removing it.
            if (data.tree.rootref().num_children() == 0)
            {
                // This logic is questionable. You might want to remove it.
            }

            data.format = format;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "[PDFxTMD] Warning: unable to parse YAML file '" << filepath.string()
                      << "': " << e.what() << std::endl;
            initializeEmptyYAML();
            return false;
        }
    }
    return false;
}
} // namespace PDFxTMD
