#include "PDFxTMDLib/Common/typeUtils.h"
#include <limits>
#include <exception>
#include <string>
#include <stdexcept>

namespace PDFxTMD
{
uint32_t toU32(size_t v, const char *what)
{
    if (v > static_cast<size_t>(std::numeric_limits<uint32_t>::max()))
    {
        throw std::runtime_error(std::string("overflow converting ") + what + " to uint32");
    }
    return static_cast<uint32_t>(v);
}
}