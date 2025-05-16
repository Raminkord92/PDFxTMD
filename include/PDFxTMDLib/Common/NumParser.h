#pragma once
#include <cstring>
#include <string_view>
#include <cstdlib>
#include <cerrno>
#include <type_traits>

namespace PDFxTMD
{

class NumParser
{
  public:
    explicit NumParser(std::string_view input) noexcept
        : _current(input.data()), _end(input.data() + input.size())
    {
    }

    void reset(std::string_view input) noexcept
    {
        _current = input.data();
        _end     = input.data() + input.size();
    }

    template <typename T>
    bool operator>>(T &value) noexcept
    {
        skipSpaces();
        return parseNumber(value);
    }

    [[nodiscard]] bool hasMore() const noexcept
    {
        return _current < _end;
    }

  private:
    const char *_current;
    const char *_end;

    void skipSpaces() noexcept
    {
        while (_current < _end && *_current == ' ')
            ++_current;
    }

    template <typename T>
    bool parseNumber(T &value) noexcept
    {
        char *next = nullptr;
        errno = 0;

        if constexpr (std::is_integral_v<T>) {
            // parse as signed long long, then cast
            long long tmp = std::strtoll(_current, &next, /*base=*/10);
            if (_current == next || errno == ERANGE)
                return false;
            value = static_cast<T>(tmp);
        }
        else if constexpr (std::is_unsigned_v<T>) {
            unsigned long long tmp = std::strtoull(_current, &next, /*base=*/10);
            if (_current == next || errno == ERANGE)
                return false;
            value = static_cast<T>(tmp);
        }
        else if constexpr (std::is_floating_point_v<T>) {
            long double tmp = std::strtold(_current, &next);
            if (_current == next || errno == ERANGE)
                return false;
            value = static_cast<T>(tmp);
        }
        else {
            static_assert(!sizeof(T*), "parseNumber only supports integral or floating types");
        }

        // advance past what we just consumed:
        _current = next;
        return true;
    }
};

} // namespace PDFxTMD
