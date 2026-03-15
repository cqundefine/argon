#include <Compiler/AST.h>
#include <Compiler/Globals.h>
#include <Compiler/Lexer.h>
#include <Compiler/Parser.h>
#include <Shared/Utils/Stack.h>

#include <map>
#include <memory>
#include <string>

Parser::Parser(std::span<const Token> tokens)
    : m_tokens(tokens)
{
}

std::shared_ptr<ParsedFile> Parser::parse()
{
    std::vector<std::shared_ptr<VariableStatement>> variables;
    std::vector<std::shared_ptr<Function>> functions;
    std::vector<std::shared_ptr<Enum>> enums;
    while (m_tokens[m_index].type != TokenType::Eof) {
        const auto& token = m_tokens[m_index];

        switch (token.type) {
            using enum TokenType;
            case Let:
            case Var:
                m_index++;
                variables.push_back(parse_variable_statement());
                break;
            case Function:
                functions.push_back(parse_function());
                break;
            case Enum:
                enums.push_back(parse_enum());
                break;
            default:
                error("Unexpected token: {}", token);
        }
    }
    return std::make_shared<ParsedFile>(variables, functions, enums);
}

static const std::map<BinaryOperation, int> g_binary_precedence = {
    { BinaryOperation::Assign, 2 },
    { BinaryOperation::Add, 11 },
    { BinaryOperation::Subtract, 11 },
    { BinaryOperation::Multiply, 12 },
    { BinaryOperation::Divide, 12 },
    { BinaryOperation::Equals, 9 },
    { BinaryOperation::LessThan, 9 },
    { BinaryOperation::GreaterThan, 9 },
};

static const std::map<TokenType, BinaryOperation> g_binary_operations = {
    { TokenType::Equals, BinaryOperation::Assign },
    { TokenType::Plus, BinaryOperation::Add },
    { TokenType::Minus, BinaryOperation::Subtract },
    { TokenType::Asterisk, BinaryOperation::Multiply },
    { TokenType::Slash, BinaryOperation::Divide },
    { TokenType::DoubleEquals, BinaryOperation::Equals },
    { TokenType::LessThan, BinaryOperation::LessThan },
    { TokenType::GreaterThan, BinaryOperation::GreaterThan },
};

static const std::map<TokenType, UnaryOperation> g_unary_operations = {
    // { TokenType::Delete, UnaryOperation::Delete },
};

std::optional<UnaryOperation> Parser::parse_unary_operation()
{
    if (g_unary_operations.contains(m_tokens[m_index].type))
        return g_unary_operations.at(m_tokens[m_index++].type);
    return {};
}

std::optional<BinaryOperation> Parser::parse_binary_operation()
{
    if (g_binary_operations.contains(m_tokens[m_index].type))
        return g_binary_operations.at(m_tokens[m_index++].type);
    return {};
}

std::shared_ptr<Expression> Parser::parse_primary()
{
    auto maybe_unary = parse_unary_operation();

    const auto parse_element = [&]() -> std::shared_ptr<Expression> {
        const auto& token = m_tokens[m_index++];

        switch (token.type) {
            using enum TokenType;
            case LParen: {
                const auto expression = parse_expression();
                expect(TokenType::RParen);
                return expression;
            }
            case Number:
                return std::make_shared<ValueExpression>(std::to_string(token.number));
            case Identifier:
                if (m_tokens[m_index].type == LParen) {
                    expect(LParen);
                    std::vector<std::shared_ptr<Expression>> arguments;
                    if (m_tokens[m_index].type != RParen) {
                        arguments = collect_until<std::shared_ptr<Expression>>([&]() { return parse_expression(); }, [&](const auto&) {
                            if (m_tokens[m_index].type == Comma) {
                                expect(Comma);
                                return false;
                            }
                            return true; });
                    }
                    expect(RParen);
                    return std::make_shared<CallExpression>(token.string, arguments);
                }
                return std::make_shared<ValueExpression>(token.string);
            case StringLiteral:
                return std::make_shared<ValueExpression>('"' + token.string + "\"s");
            case True:
                return std::make_shared<ValueExpression>("true");
            case False:
                return std::make_shared<ValueExpression>("false");
            case Null:
                return std::make_shared<ValueExpression>("nullptr");
            default:
                error("Unexpected token: {}", token);
        }
    };

    auto expression = parse_element();
    // while (m_tokens[m_index].type == TokenType::Dot || m_tokens[m_index].type == TokenType::LBracket) {
    //     const auto type = m_tokens[m_index++].type;
    //     if (type == TokenType::Dot) {
    //         expression = std::make_shared<StaticMemberAccessExpression>(expression, expect(TokenType::Identifier).string);
    //     } else if (type == TokenType::LBracket) {
    //         expression = std::make_shared<DynamicMemberAccessExpression>(expression, parse_expression());
    //         expect(TokenType::RBracket);
    //     }
    // }

    if (maybe_unary) {
        expression = std::make_shared<UnaryExpression>(expression, *maybe_unary);
    }

    return expression;
}

std::shared_ptr<Expression> Parser::parse_expression()
{
    struct OperationInfo
    {
        BinaryOperation operation;
        int precedence;
    };

    Stack<std::shared_ptr<Expression>> expression_stack;
    Stack<OperationInfo> operation_stack;
    int last_precedence = INT32_MAX;

    const auto left_side = parse_primary();
    expression_stack.push(left_side);

    while (true) {
        const auto operation = parse_binary_operation();
        if (!operation)
            break;

        const auto precedence = g_binary_precedence.at(*operation);

        const auto right_side = parse_primary();

        while (precedence <= last_precedence && expression_stack.size() > 1) {
            const auto right_side = expression_stack.pop();
            const auto operation = operation_stack.pop();

            last_precedence = operation.precedence;

            if (last_precedence < precedence) {
                operation_stack.push(operation);
                expression_stack.push(right_side);
                break;
            }

            const auto left_size = expression_stack.pop();

            expression_stack.push(std::make_shared<BinaryExpression>(left_size, operation.operation, right_side));
        }

        operation_stack.push({ *operation, precedence });
        expression_stack.push(right_side);

        last_precedence = precedence;
    }

    while (expression_stack.size() != 1) {
        auto right_side = expression_stack.pop();
        auto operation = operation_stack.pop();
        auto light_side = expression_stack.pop();

        expression_stack.push(std::make_shared<BinaryExpression>(light_side, operation.operation, right_side));
    }

    return expression_stack.pop();
}

std::shared_ptr<Statement> Parser::parse_statement()
{
    const auto& token = m_tokens[m_index++];

    switch (token.type) {
        using enum TokenType;
        case Let:
        case Var:
            return parse_variable_statement();
        case Return: {
            std::shared_ptr<Statement> statement;
            if (m_tokens[m_index].type != Semicolon)
                statement = std::make_shared<ReturnStatement>(parse_expression());
            else
                statement = std::make_shared<ReturnStatement>(std::optional<std::shared_ptr<Expression>> {});
            expect(Semicolon);
            return statement;
        }
        case If: {
            const auto condition = parse_expression();

            const auto then_block = parse_block(true);
            std::optional<std::shared_ptr<Block>> else_block;
            if (m_tokens[m_index].type == Else) {
                m_index++;
                else_block = parse_block(true);
            }
            return std::make_shared<IfStatement>(condition, then_block, else_block);
        }
        case While: {
            const auto condition = parse_expression();

            const auto block = parse_block(true);
            return std::make_shared<WhileStatement>(condition, block);
        }
        case LCurly:
            m_index--;
            return parse_block(false);
        default:
            m_index--;
            const auto statement = std::make_shared<ExpressionStatement>(parse_expression());
            expect(Semicolon);
            return statement;
    }
}

std::shared_ptr<Block> Parser::parse_block(bool allow_single_statement, bool toplevel)
{
    static size_t depth = 0;

    depth++;
    if (depth > 128) {
        error("Blocks are nested too deep");
    }

    if (!toplevel) {
        if (allow_single_statement && m_tokens[m_index].type != TokenType::LCurly) {
            return std::make_shared<Block>(std::vector<std::shared_ptr<Statement>> { parse_statement() });
        }

        expect(TokenType::LCurly);
    }

    const auto block = std::make_shared<Block>(collect_until<std::shared_ptr<Statement>>(std::bind_front(&Parser::parse_statement, this), [&](const auto&) {
        return m_tokens[m_index].type == (toplevel ? TokenType::Eof : TokenType::RCurly);
    }));

    if (!toplevel)
        expect(TokenType::RCurly);

    return block;
}

std::shared_ptr<VariableStatement> Parser::parse_variable_statement()
{
    const auto name = expect(TokenType::Identifier).string;
    expect(TokenType::Colon);
    const auto type = expect(TokenType::Identifier).string;
    std::optional<std::shared_ptr<Expression>> default_value {};
    if (m_tokens[m_index].type == TokenType::Equals) {
        m_index++;
        default_value = parse_expression();
    }
    expect(TokenType::Semicolon);
    return std::make_shared<VariableStatement>(name, type, default_value);
}

std::shared_ptr<Function> Parser::parse_function()
{
    expect(TokenType::Function);

    const auto name = expect(TokenType::Identifier).string;
    expect(TokenType::LParen);

    std::vector<Function::Argument> arguments;
    if (m_tokens[m_index].type != TokenType::RParen) {
        arguments = collect_until<Function::Argument>(
            [&]() {
                auto name = expect(TokenType::Identifier).string;
                expect(TokenType::Colon);
                auto type = expect(TokenType::Identifier).string;
                return Function::Argument { name, type };
            },
            [&](const auto&) {
                if (m_index < m_tokens.size() && m_tokens[m_index].type == TokenType::Comma) {
                    expect(TokenType::Comma);
                    return false;
                }
                return true;
            });
    }

    expect(TokenType::RParen);
    expect(TokenType::Arrow);
    const auto return_type = expect(TokenType::Identifier).string;
    const auto block = parse_block(false);
    return std::make_shared<Function>(name, return_type, arguments, block);
}

std::shared_ptr<Enum> Parser::parse_enum()
{
    expect(TokenType::Enum);
    const auto name = expect(TokenType::Identifier).string;
    expect(TokenType::LCurly);

    std::vector<std::string> values;
    if (m_tokens[m_index].type != TokenType::RParen) {
        values = collect_until<std::string>([&]() { return expect(TokenType::Identifier).string; }, [&](const auto&) {
                        if (m_tokens[m_index].type == TokenType::Comma) {
                            expect(TokenType::Comma);
                            if (m_tokens[m_index].type == TokenType::Identifier)
                                return false;
                        }
                        return true; });
    }

    expect(TokenType::RCurly);
    return std::make_shared<Enum>(name, values);
}

Token Parser::expect(TokenType type)
{
    auto token = m_tokens[m_index++];
    if (token.type != type)
        error("Unexpected token: {}, expected: {}", token, type);
    return token;
}
