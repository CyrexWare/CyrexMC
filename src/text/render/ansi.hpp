#pragma once
#include "text/format/color.hpp"

#include <string>
#include <string_view>
#include <unordered_map>

namespace cyrex::text::render
{
inline std::string ansi(std::string_view text)
{
    static const std::unordered_map<char, std::string>
        map{{'0', "\033[30m"},
            {'1', "\033[34m"},
            {'2', "\033[32m"},
            {'3', "\033[36m"},
            {'4', "\033[31m"},
            {'5', "\033[35m"},
            {'6', "\033[33m"},
            {'7', "\033[37m"},
            {'8', "\033[90m"},
            {'9', "\033[94m"},
            {'a', "\033[92m"},
            {'b', "\033[96m"},
            {'c', "\033[91m"},
            {'d', "\033[95m"},
            {'e', "\033[93m"},
            {'f', "\033[97m"},
            {'l', "\033[1m"},
            {'o', "\033[3m"},
            {'r', "\033[0m"}};

    std::string out;

    for (size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == Code && i + 1 < text.size())
        {
            if (auto it = map.find(text[i + 1]); it != map.end())
                out += it->second;
            ++i;
            continue;
        }
        out += text[i];
    }

    out += "\033[0m";
    return out;
}
} // namespace cyrex::text::render
