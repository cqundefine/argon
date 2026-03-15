#include <Compiler/Globals.h>
#include <Compiler/Lexer.h>
#include <Shared/Utils/StringMap.h>
#include <Shared/Utils/Utils.h>

#include <map>
#include <string>

static const std::map<char, TokenType> g_basic_tokens = {
    { '=', TokenType::Equals },
    { ':', TokenType::Colon },
    { ';', TokenType::Semicolon },
    { '.', TokenType::Dot },
    { '+', TokenType::Plus },
    { '-', TokenType::Minus },
    { '*', TokenType::Asterisk },
    { '/', TokenType::Slash },
    { ',', TokenType::Comma },
    { '(', TokenType::LParen },
    { ')', TokenType::RParen },
    { '{', TokenType::LCurly },
    { '}', TokenType::RCurly },
    { '[', TokenType::LBracket },
    { ']', TokenType::RBracket },
    { '<', TokenType::LessThan },
    { '>', TokenType::GreaterThan },
};

static const std::map<char, std::map<char, TokenType>> g_double_tokens = {
    { '=', { { '=', TokenType::DoubleEquals } } },
    { '-', { { '>', TokenType::Arrow } } },
};

static const StringMap<TokenType> g_keyword_tokens = {
    { "var", TokenType::Var },
    { "let", TokenType::Let },
    { "function", TokenType::Function },
    { "return", TokenType::Return },
    { "if", TokenType::If },
    { "else", TokenType::Else },
    { "while", TokenType::While },
    { "true", TokenType::True },
    { "false", TokenType::False },
    { "null", TokenType::Null },
    { "enum", TokenType::Enum },
};

std::vector<Token> lex_file(std::span<const char> file)
{
    size_t index = 0;
    const auto skip_whitespace = [&]() -> void {
        while (index < file.size() && std::isspace(file[index]))
            index++;
    };

    const auto lex_token = [&]() -> Token {
        skip_whitespace();
        if (index >= file.size())
            return TokenType::Eof;

        if (file[index] == '"') {
            index++;
            std::string value;
            while (index < file.size() && file[index] != '"')
                value += file[index++];
            if (index >= file.size())
                error("Unterminated string literal");
            index++;
            return Token { TokenType::StringLiteral, value };
        }

        if (std::isdigit(file[index])) {
            uint64_t value = 0;

            if (file[index] == '0' && index + 1 < file.size()) {
                char prefix = file[index + 1];

                if (prefix == 'x' || prefix == 'X') {
                    index += 2;

                    if (index >= file.size() || !std::isxdigit(file[index]))
                        error("Empty hex literal");

                    while (index < file.size() && std::isxdigit(file[index]))
                        value = value * 16 + std::stoi(std::string(1, file[index++]), nullptr, 16);

                    return Token(value);
                }

                if (prefix == 'b' || prefix == 'B') {
                    index += 2;

                    if (index >= file.size() || (file[index] != '0' && file[index] != '1'))
                        error("Empty binary literal");

                    while (index < file.size() && (file[index] == '0' || file[index] == '1'))
                        value = value * 2 + (file[index++] - '0');

                    return Token(value);
                }

                if (prefix == 'o' || prefix == 'O') {
                    index += 2;

                    if (index >= file.size() || file[index] < '0' || file[index] > '7')
                        error("Empty octal literal");

                    while (index < file.size() && file[index] >= '0' && file[index] <= '7')
                        value = value * 8 + (file[index++] - '0');

                    return Token(value);
                }
            }

            while (index < file.size() && std::isdigit(file[index]))
                value = value * 10 + (file[index++] - '0');

            return Token(value);
        }

        if (std::isalpha(file[index]) || file[index] == '_') {
            std::string value;

            while (index < file.size() && (std::isalnum(file[index]) || file[index] == '_'))
                value += file[index++];

            if (g_keyword_tokens.contains(value))
                return Token { g_keyword_tokens.at(value) };

            return Token { TokenType::Identifier, value };
        }
        if (g_double_tokens.contains(file[index])) {
            if (index + 1 < file.size() && g_double_tokens.at(file[index]).contains(file[index + 1])) {
                return g_double_tokens.at(file[index++]).at(file[index++]);
            }
        }

        if (!g_basic_tokens.contains(file[index]))
            error("Unexpected char: {}", file[index]);

        return Token { g_basic_tokens.at(file[index++]) };
    };

    return collect_until<Token>(lex_token, [](const auto& token) { return token.type == TokenType::Eof; });
}
