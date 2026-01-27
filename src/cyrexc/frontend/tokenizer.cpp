#include "tokenizer.hpp"

#include <frontend/reserved_words.hpp>
#include <util/cmap.hpp>

namespace
{
constexpr auto reservedWords = cyrex::makeReservedWords();
constexpr auto reservedPunctuation = cyrex::makeReservedPunctuation();

} // namespace

std::vector<cyrex::Token> cyrex::tokenize(const std::string& sourceCode)
{
    std::vector<Token> tokens;

    // TODO:
    std::size_t line{1};
    std::size_t column{1};

    auto start = sourceCode.begin();

    while (start != sourceCode.end())
    {
        const auto isDigit = [](const unsigned char ch)
        {
            return isdigit(ch);
        };
        const auto isIdentifierBeginning = [](const unsigned char ch)
        {
            return isalpha(ch) || ch == '_';
        };
        const auto isIdentifierEnding = [isIdentifierBeginning, isDigit](const unsigned char ch)
        {
            return isIdentifierBeginning(ch) || isdigit(ch);
        };

        const auto isSpace = [](const unsigned char ch)
        {
            return isspace(ch);
        };
        const auto isPunct = [](const unsigned char ch)
        {
            return ispunct(ch);
        };
        const auto isSpeechMark = [](const unsigned char ch)
        {
            return ch == '\"';
        };
        const auto findOrDefault = [](const auto& map, const auto& key, const auto& orElse)
        {
            const auto it = map.find(key);
            if (it != map.end())
            {
                return it->second;
            }
            return orElse;
        };

        // skip whitespace
        start = std::find_if_not(start, sourceCode.end(), isSpace);
        if (start == sourceCode.end())
            break;

        // scan numbers
        if (isDigit(*start))
        {
            auto end = std::find_if_not(start, sourceCode.end(), isDigit);
            tokens.emplace_back(Token::Kind::LiteralInteger, std::string{start, end});
            start = end;
            continue;
        }

        // scan a word
        if (isIdentifierBeginning(*start))
        {
            const auto end = std::find_if_not(start, sourceCode.end(), isIdentifierEnding);
            const auto word = std::string{start, end};
            const auto kind = findOrDefault(reservedWords, word, Token::Kind::Identifier);
            auto& tok = tokens.emplace_back(kind, std::string{start, end});
            // may be a keyword
            start = end;
            continue;
        }

        if (isSpeechMark(*start))
        {
            start = std::next(start);
            const auto end = std::find_if(start, sourceCode.end(), isSpeechMark);
            if (end == sourceCode.end())
            {
                throw LexicalError("Unterminated string", line, column);
            }
            tokens.emplace_back(Token::Kind::LiteralString, std::string{start, end});
            start = std::next(end);
            continue;
        }

        if (isPunct(*start))
        {
            std::string bestMatch;
            for (auto len = 1; len <= sourceCode.end() - start; ++len)
            {
                const std::string candidate(start, start + len);
                if (reservedPunctuation.find(candidate) != reservedPunctuation.data.end())
                    bestMatch = candidate;
            }
            if (bestMatch.empty())
                goto unexpected; // NOLINT(*-avoid-goto)
            tokens.emplace_back(reservedPunctuation.at(bestMatch), bestMatch);
            start += bestMatch.size();
            continue;
        }

    unexpected:
        start = std::next(start);
    }

    return tokens;
}