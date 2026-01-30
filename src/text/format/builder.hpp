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
        m_data += code(c);
        return *this;
    }

    Builder& style(Style s)
    {
        m_data += code(s);
        return *this;
    }

    Builder& text(std::string_view t)
    {
        m_data += t;
        return *this;
    }

    Builder& reset()
    {
        m_data += code(Style::RESET);
        return *this;
    }

    Builder& space()
    {
        m_data += ' ';
        return *this;
    }

    [[nodiscard]] std::string build() const
    {
        return m_data;
    }

    operator std::string() const
    {
        return m_data;
    }

private:
    std::string m_data;
};
} // namespace cyrex::text::format
