#pragma once

#include <External/magic_enum.hpp>

#include <format>
#include <span>
#include <string>
#include <string_view>
#include <vector>

enum class TokenType
{
    Invalid,

    Identifier,
    Number,
    StringLiteral,

    Equals,
    Colon,
    Semicolon,
    Dot,
    Plus,
    Minus,
    Asterisk,
    Slash,
    Comma,
    LParen,
    RParen,
    LCurly,
    RCurly,
    LBracket,
    RBracket,
    LessThan,
    GreaterThan,
    DoubleEquals,
    Arrow,

    Var,
    Let,
    Function,
    Return,
    If,
    Else,
    While,
    True,
    False,
    Null,
    Enum,

    Eof,
};

template <>
struct std::formatter<TokenType>
{
    constexpr auto parse(std::format_parse_context& context) { return std::cbegin(context); }

    auto format(const TokenType& type, std::format_context& context) const
    {
        return std::format_to(context.out(), "{}", magic_enum::enum_name(type));
    }
};

struct Token
{
    TokenType type;

    std::string string {};
    uint64_t number {};

    Token()
        : type(TokenType::Invalid)
    {
    }

    Token(TokenType type)
        : type(type)
    {
    }

    Token(uint64_t number)
        : type(TokenType::Number)
        , number(number)
    {
    }

    Token(TokenType type, std::string_view string)
        : type(type)
        , string(string)
    {
    }
};

template <>
struct std::formatter<Token>
{
    constexpr auto parse(std::format_parse_context& context) { return std::cbegin(context); }

    auto format(const Token& token, std::format_context& context) const
    {
        if (token.type == TokenType::Identifier)
            return std::format_to(context.out(), "Identifier: {}", token.string);

        if (token.type == TokenType::Number)
            return std::format_to(context.out(), "Number: {}", token.number);

        return std::format_to(context.out(), "{}", token.type);
    }
};

std::vector<Token> lex_file(std::span<const char> file);
