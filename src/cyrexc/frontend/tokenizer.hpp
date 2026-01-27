#pragma once
#include <frontend/tokens.hpp>
#include <stdexcept>
#include <vector>

namespace cyrex
{

struct LexicalError : std::runtime_error
{
    std::string message;
    std::size_t line{};
    std::size_t column{};

    explicit constexpr LexicalError(const std::string_view message, const std::size_t line, const std::size_t column) :
        std::runtime_error({message.begin(), message.end()}),
        line(line),
        column(column)
    {
    }
};

// Throws: LexicalError
std::vector<Token> tokenize(const std::string& sourceCode);
} // namespace cyrex