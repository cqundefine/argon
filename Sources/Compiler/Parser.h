#pragma once

#include <Compiler/AST.h>
#include <Compiler/Lexer.h>

#include <memory>
#include <optional>
#include <span>

class Parser
{
public:
    Parser(std::span<const Token> tokens);

    std::shared_ptr<ParsedFile> parse();

private:
    std::optional<UnaryOperation> parse_unary_operation();
    std::optional<BinaryOperation> parse_binary_operation();
    std::shared_ptr<Expression> parse_primary();
    std::shared_ptr<Expression> parse_expression();
    std::shared_ptr<Statement> parse_statement();
    std::shared_ptr<Block> parse_block(bool allow_single_statement, bool toplevel = false);
    std::shared_ptr<VariableStatement> parse_variable_statement();
    std::shared_ptr<Function> parse_function();
    std::shared_ptr<Enum> parse_enum();

    Token expect(TokenType type);

    std::span<const Token> m_tokens;
    size_t m_index { 0 };
};
