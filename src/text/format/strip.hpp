#pragma once
#include "color.hpp"

#include <string>
#include <string_view>

namespace cyrex::text::format
{
inline std::string strip(std::string_view text)
{
    std::string out;

    for (size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == Code && i + 1 < text.size())
        {
            ++i;
            continue;
        }
        out.push_back(text[i]);
    }

    return out;
}
} // namespace cyrex::text::format
