//
// Created by Jamie on 27/01/2026.
//

#ifndef CYREXC_TOKENS_HPP
#define CYREXC_TOKENS_HPP

#include <string>

namespace cyrex
{
struct Token
{
    enum class Kind
    {
        // Structural keywords
        Function,
        Const,
        Var,
        If,
        Else,
        While,
        Do,
        Then,
        Return,

        // Signed types
        S8,
        S16,
        S32,
        S64,

        // Unsigned types
        U8,
        U16,
        U32,
        U64,

        // Elements
        Identifier,
        LiteralInteger,
        LiteralString,

        // Punctuation
        LeftParenthesis,
        RightParenthesis,
        LeftSquareBracket,
        RightSquareBracket,
        LeftCurlyBracket,
        RightCurlyBracket,
        Comma,
        Colon,
        Assign,

        // Bitwise logical operators
        Not,
        ExclusiveOr,
        Or,
        And,

        // Math operators
        Plus,
        Minus,
        Star,
        Slash,

        // Comparative operators
        LessThan,
        LessThanEqualTo,
        GreaterThan,
        GreaterThanEqualTo,
        EqualTo,
        NotEqualTo
    };

    explicit constexpr Token(const Kind kind, const std::string_view text) : kind(kind), text(text.begin(), text.end())
    {
    }

    Kind kind{};
    std::string text;
};

} // namespace cyrex

#endif //CYREXC_TOKENS_HPP
