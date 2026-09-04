#pragma once
#include <cstdlib>
#include <iostream>
#include <string>

namespace PDFxTMD
{
class LibraryBanner
{
  public:
    static LibraryBanner &Instance()
    {
        static LibraryBanner instance;
        return instance;
    }

    LibraryBanner(const LibraryBanner &) = delete;
    LibraryBanner &operator=(const LibraryBanner &) = delete;

  private:
    LibraryBanner()
    {
        printBanner();
    }

    static std::string configFilePath()
    {
#if defined(_WIN32)
        return "C:/ProgramData/PDFxTMDLib/config.yaml";
#else
        const char *homeDir = std::getenv("HOME");
        return homeDir ? std::string(homeDir) + "/.PDFxTMDLib/config.yaml"
                       : "~/.PDFxTMDLib/config.yaml";
#endif
    }

    void printBanner() const
    {
        std::string banner = R"(
============================================================
          Welcome to the PDFxTMDLib Library
============================================================
Please cite the following paper when using this library:

Title: PDFxTMDLib: A High-Performance C++ Library for Collinear and Transverse Momentum Dependent Parton Distribution Functions
Authors: R. Kord Valeshabadi, S. Rezaie
Source: https://arxiv.org/abs/2412.16680
)";
        std::cout << banner << '\n'
                  << "Configuration file: " << configFilePath() << '\n'
                  << "Add custom PDF-set directories to the YAML 'paths' list in this file.\n\n"
                  << "Thank you for using our library!\n"
                  << "============================================================\n"
                  << std::endl;
    }
};
} // namespace PDFxTMD
