#pragma once
#include <cstdint>
#include <optional>
#include <string_view>

namespace RiverRaid
{
    // Manual entry accepts decimal, 0x-prefixed hex, or Atari-style $ hex.
    inline std::optional<uint16_t> ParseSeed(std::wstring_view text)
    {
        constexpr std::wstring_view whitespace = L" \t\r\n";
        const auto first = text.find_first_not_of(whitespace);
        if (first == text.npos) return std::nullopt;
        text = text.substr(first, text.find_last_not_of(whitespace) - first + 1);
        unsigned base = 10;
        if (text.starts_with(L"0x") || text.starts_with(L"0X")) { base = 16; text.remove_prefix(2); }
        else if (text.starts_with(L"$")) { base = 16; text.remove_prefix(1); }
        if (text.empty()) return std::nullopt;
        unsigned value = 0;
        for (wchar_t ch : text)
        {
            unsigned digit;
            if (ch >= L'0' && ch <= L'9') digit = ch - L'0';
            else if (ch >= L'a' && ch <= L'f') digit = ch - L'a' + 10;
            else if (ch >= L'A' && ch <= L'F') digit = ch - L'A' + 10;
            else return std::nullopt;
            if (digit >= base || value > (65535u - digit) / base) return std::nullopt;
            value = value * base + digit;
        }
        if (value == 0) return std::nullopt;
        return static_cast<uint16_t>(value);
    }
}
