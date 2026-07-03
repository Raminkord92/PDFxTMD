#pragma once
#include <iomanip>
#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <filesystem>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "PDFxTMDLib/Common/LibraryBanner.h"

#define FOLDER_SEP "/"
#define STD_PDF_INFO_EXTENSION ".info"
#define STD_PDF_DATA_EXTENSION ".dat"
#define DEFAULT_TOTAL_PDFS 13
#define DEFULT_NUM_FLAVORS 5
#define DOWN_DEFAULT_MASS 0.005
#define UP_DEFAULT_MASS 0.002
#define STRANGE_DEFAULT_MASS 0.10
#define CHARM_DEFAULT_MASS 1.29
#define BOTTOM_DEFAULT_MASS 4.19
#define TOP_DEFAULT_MASS 172.9
#define SQR(x) ((x) * (x))
#define NO_REQUESTED_CONFIDENCE_LEVEL -1
namespace PDFxTMD
{
// https://www.modernescpp.com/index.php/from-variadic-templates-to-fold-expressions/
template <typename... Args> bool all(Args... args)
{
    return (... && args);
}
enum class PhaseSpaceComponent
{
    X,
    Kt2,
    Q2
};

enum class ErrorType
{
    None,                    // No error
    CONFIG_KeyNotFound,      // The key does not exist
    CONFIG_ConversionFailed, // Conversion to the desired type
                             // failed,
    FILE_NOT_FOUND = 10,
};
enum class OrderQCD
{
    LO,
    NLO,
    N2LO,
    N3LO,
    N4LO
};
enum PartonFlavor
{
    tbar = -6,
    bbar = -5,
    cbar = -4,
    sbar = -3,
    dbar = -1,
    ubar = -2,
    gNS = 0, // non-standard g
    u = 2,
    d = 1,
    s = 3,
    c = 4,
    b = 5,
    t = 6,
    g = 21,
    photon = 22,
    // Non stadard ew partciles
    z0 = 100,
    wplus,
    wminus,
    higgs
};
constexpr std::array<PartonFlavor, DEFAULT_TOTAL_PDFS> standardPartonFlavors = {
    PartonFlavor::tbar, PartonFlavor::bbar, PartonFlavor::cbar, PartonFlavor::sbar,
    PartonFlavor::ubar, PartonFlavor::dbar, PartonFlavor::gNS,  PartonFlavor::d,
    PartonFlavor::u,    PartonFlavor::s,    PartonFlavor::c,    PartonFlavor::b,
    PartonFlavor::t};

std::vector<std::string> splitPaths(const std::string &paths);
bool hasWriteAccess(const std::string &path);
double _extrapolateLinear(double x, double xl, double xh, double yl, double yh);
std::string StandardPDFNaming(const std::string &pdfName, int set);
std::vector<std::string> GetPDFxTMDPathsAsVector();
std::vector<std::string> GetPDFxTMDPathsFromYaml();
bool AddPathToEnvironment(const std::string &newPath);
std::vector<std::string> split(const std::string &str, char delimiter);
/// Format an integer @a val as a zero-padded string of length
/// @a nchars
std::string to_str_zeropad(int val, size_t nchars = 4);
std::pair<std::optional<std::string>, ErrorType> StandardInfoFilePath(
    const std::string &pdfSetName);
std::pair<std::optional<std::string>, ErrorType> StandardPDFSetPath(const std::string &pdfSetName,
                                                                    int set);
// taken from the lhapdf
inline size_t indexbelow(double value, const std::vector<double> &knots)
{
    size_t i = std::upper_bound(knots.begin(), knots.end(), value) - knots.begin();
    if (i == knots.size())
        i -= 1; // can't return the last knot index
    i -= 1;     // step back to get the knot <= x behaviour
    return i;
}
/// Check if a number is in a range (closed-open) (from lhapdf)
inline int in_range(double x, double low, double high)
{
    return x >= low && x < high;
}
} // namespace PDFxTMD
