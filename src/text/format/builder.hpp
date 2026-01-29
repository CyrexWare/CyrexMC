#pragma once
#include "color.hpp"

#include <string>
#include <string_view>

namespace cyrex::text::format
{
class Builder
{
public:
    Builder& color(Color c)
    {
        data += code(c);
        return *this;
    }

    Builder& style(Style s)
    {
        data += code(s);
        return *this;
    }

    Builder& text(std::string_view t)
    {
        data += t;
        return *this;
    }

    Builder& reset()
    {
        data += code(Style::RESET);
        return *this;
    }

    Builder& space()
    {
        data += ' ';
        return *this;
    }

    std::string build() const
    {
        return data;
    }

    operator std::string() const
    {
        return data;
    }

private:
    std::string data;
};
} // namespace cyrex::text::format
