#pragma once

#include <Compiler/Globals.h>
#include <External/magic_enum.hpp>
#include <Shared/Utils/Utils.h>

#include <memory>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

struct Expression
{
    virtual void dump(const std::string& indent = "") const = 0;

    virtual std::string codegen() const = 0;
};

struct ValueExpression : public Expression
{
    std::string value;

    ValueExpression(std::string_view value)
        : value(value)
    {
    }

    void dump(const std::string& indent = "") const override
    {
        std::println("{}Value: {}", indent, value);
    }

    virtual std::string codegen() const override;
};

struct CallExpression : public Expression
{
    std::string function;
    std::vector<std::shared_ptr<Expression>> parameters;

    CallExpression(std::string_view function, std::vector<std::shared_ptr<Expression>> parameters)
        : function(function)
        , parameters(parameters)
    {
    }

    void dump(const std::string& indent = "") const override
    {
        std::println("{}Call: {}", indent, function);
        for (const auto& parameter : parameters)
            parameter->dump(indent + "  ");
    }

    virtual std::string codegen() const override;
};

enum class BinaryOperation
{
    Assign,
    Add,
    Subtract,
    Multiply,
    Divide,
    Equals,
    GreaterThan,
    LessThan,
};

template <>
struct std::formatter<BinaryOperation>
{
    constexpr auto parse(std::format_parse_context& context) { return std::cbegin(context); }

    auto format(const BinaryOperation& operation, std::format_context& context) const
    {
        return std::format_to(context.out(), "{}", magic_enum::enum_name(operation));
    }
};

struct BinaryExpression : public Expression
{
    std::shared_ptr<Expression> lhs;
    BinaryOperation operation;
    std::shared_ptr<Expression> rhs;

    BinaryExpression(std::shared_ptr<Expression> lhs, BinaryOperation operation, std::shared_ptr<Expression> rhs)
        : lhs(lhs)
        , operation(operation)
        , rhs(rhs)
    {
    }

    void dump(const std::string& indent = "") const override
    {
        std::println("{}Binary: {}", indent, operation);
        lhs->dump(indent + "  ");
        rhs->dump(indent + "  ");
    }

    virtual std::string codegen() const override;
};

enum class UnaryOperation
{
};

template <>
struct std::formatter<UnaryOperation>
{
    constexpr auto parse(std::format_parse_context& context) { return std::cbegin(context); }

    auto format(const UnaryOperation& operation, std::format_context& context) const
    {
        // return std::format_to(context.out(), "{}", magic_enum::enum_name(operation));
        return std::format_to(context.out(), "a");
    }
};

struct UnaryExpression : public Expression
{
    std::shared_ptr<Expression> value;
    UnaryOperation operation;
    std::shared_ptr<Expression> rhs;

    UnaryExpression(std::shared_ptr<Expression> value, UnaryOperation operation)
        : value(value)
        , operation(operation)
    {
    }

    void dump(const std::string& indent = "") const override
    {
        std::println("{}Unary: {}", indent, operation);
        value->dump(indent + "  ");
    }

    virtual std::string codegen() const override;
};

struct Statement
{
    virtual void dump(const std::string& indent = "") const = 0;
    virtual std::string codegen() const = 0;
};

struct Block : public Statement
{
    std::vector<std::shared_ptr<Statement>> statements;

    Block(std::vector<std::shared_ptr<Statement>> statements = {})
        : statements(statements)
    {
    }

    void dump(const std::string& indent = "") const override
    {
        std::println("{}Block", indent);
        for (auto& statement : statements)
            statement->dump(indent + "  ");
    }

    virtual std::string codegen() const override;
};

struct ExpressionStatement : public Statement
{
    std::shared_ptr<Expression> expression;

    ExpressionStatement(std::shared_ptr<Expression> expression)
        : expression(expression)
    {
    }

    void dump(const std::string& indent = "") const override
    {
        std::println("{}Expression", indent);
        expression->dump(indent + "  ");
    }

    virtual std::string codegen() const override;
};

struct VariableStatement : public Statement
{
    std::string name;
    std::string type;
    std::optional<std::shared_ptr<Expression>> default_value;

    VariableStatement(std::string_view name, std::string_view type, std::optional<std::shared_ptr<Expression>> default_value)
        : name(name)
        , type(type)
        , default_value(default_value)
    {
    }

    void dump(const std::string& indent = "") const override
    {
        std::println("{}Variable Declaration: {} ({})", indent, name, type);
        if (default_value)
            (*default_value)->dump(indent + "  ");
    }

    virtual std::string codegen() const override;
};

struct ReturnStatement : public Statement
{
    std::optional<std::shared_ptr<Expression>> value;

    ReturnStatement(std::optional<std::shared_ptr<Expression>> value)
        : value(value)
    {
    }

    void dump(const std::string& indent = "") const override
    {
        std::println("{}Return", indent);
        if (value)
            (*value)->dump(indent + "  ");
    }

    virtual std::string codegen() const override;
};

struct IfStatement : public Statement
{
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Block> then_block;
    std::optional<std::shared_ptr<Block>> else_block;

    IfStatement(std::shared_ptr<Expression> condition, std::shared_ptr<Block> then_block, std::optional<std::shared_ptr<Block>> else_block)
        : condition(condition)
        , then_block(then_block)
        , else_block(else_block)
    {
    }

    void dump(const std::string& indent = "") const override
    {
        std::println("{}If", indent);
        condition->dump(indent + "  ");
        then_block->dump(indent + "  ");
        if (else_block) {
            (*else_block)->dump(indent + "  ");
        }
    }

    virtual std::string codegen() const override;
};

struct WhileStatement : public Statement
{
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Block> block;

    WhileStatement(std::shared_ptr<Expression> condition, std::shared_ptr<Block> block)
        : condition(condition)
        , block(block)
    {
    }

    void dump(const std::string& indent = "") const override
    {
        std::println("{}While", indent);
        condition->dump(indent + "  ");
        block->dump(indent + "  ");
    }

    virtual std::string codegen() const override;
};

struct Function
{

    struct Argument
    {
        std::string name;
        std::string type;
    };

    std::string name;
    std::string return_type;
    std::vector<Argument> arguments;
    std::shared_ptr<Block> block;

    Function(std::string_view name, std::string return_type, std::vector<Argument> arguments, std::shared_ptr<Block> block)
        : name(name)
        , return_type(return_type)
        , arguments(arguments)
        , block(block)
    {
    }

    void dump(const std::string& indent = "") const
    {
        const auto arguments_string = arguments | std::views::transform([](const auto& argument) {
            return std::format("{} {}", argument.type, argument.name);
        }) | std::views::join_with(std::string_view { ", " })
            | std::ranges::to<std::string>();

        std::println("{}Function: {} ({})", indent, name, arguments_string);
        block->dump(indent + "  ");
    }

    std::string codegen() const;
};

struct Enum
{
    std::string name;
    std::vector<std::string> values;

    Enum(std::string_view name, std::vector<std::string> values)
        : name(name)
        , values(values)
    {
    }

    void dump(const std::string& indent = "") const
    {
        std::println("{}Enum: {}", indent, name);
        for (const auto& value : values)
            std::println("{}{}", indent + "  ", value);
    }

    std::string codegen() const;
};

struct ParsedFile
{
    std::vector<std::shared_ptr<VariableStatement>> variables;
    std::vector<std::shared_ptr<Function>> functions;
    std::vector<std::shared_ptr<Enum>> enums;

    ParsedFile(std::vector<std::shared_ptr<VariableStatement>> variables, std::vector<std::shared_ptr<Function>> functions, std::vector<std::shared_ptr<Enum>> enums)
        : variables(variables)
        , functions(functions)
        , enums(enums)
    {
    }

    void dump(const std::string& indent = "") const
    {
        std::println("{}ParsedFile", indent);
        for (const auto variable : variables)
            variable->dump(indent + "  ");
        for (const auto function : functions)
            function->dump(indent + "  ");
        for (const auto enu : enums)
            enu->dump(indent + "  ");
    }

    std::string codegen() const;
};
