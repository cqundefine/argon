#include <Compiler/AST.h>
#include <Compiler/Globals.h>
#include <Shared/Utils/TypeCasts.h>

#include <ranges>
#include <utility>

std::string ValueExpression::codegen() const
{
    return value;
}

std::string CallExpression::codegen() const
{
    const auto parameter_string = parameters | std::views::transform([](const auto& parameter) {
        return parameter->codegen();
    }) | std::views::join_with(std::string_view { "," })
        | std::ranges::to<std::string>();

    return std::format("{}({})", function, parameter_string);
}

std::string BinaryExpression::codegen() const
{
    const auto lhs_value = lhs->codegen();
    const auto rhs_value = rhs->codegen();

    switch (operation) {
        using enum BinaryOperation;
        case Add:
            return std::format("({} + {})", lhs_value, rhs_value);
        case Subtract:
            return std::format("({} - {})", lhs_value, rhs_value);
        case Multiply:
            return std::format("({} * {})", lhs_value, rhs_value);
        case Divide:
            return std::format("({} / {})", lhs_value, rhs_value);
        case Equals:
            return std::format("({} == {})", lhs_value, rhs_value);
        case LessThan:
            return std::format("({} < {})", lhs_value, rhs_value);
        case GreaterThan:
            return std::format("({} > {})", lhs_value, rhs_value);
        case Assign:
            return std::format("{} = {}", lhs_value, rhs_value);
    }

    std::unreachable();
}

std::string UnaryExpression::codegen() const
{
    switch (operation) {
        using enum UnaryOperation;
    }

    std::unreachable();
}

std::string Block::codegen() const
{
    return "{\n" + (statements | std::views::transform([](const auto& parameter) {
        return parameter->codegen();
    }) | std::views::join_with(std::string_view { "\n" })
               | std::ranges::to<std::string>())
        + "\n}\n";
}

std::string ExpressionStatement::codegen() const
{
    return expression->codegen() + ";";
}

std::string VariableStatement::codegen() const
{
    if (default_value)
        return std::format("{} {} = {};", type, name, (*default_value)->codegen());
    else
        return std::format("{} {};", type, name);
}

std::string ReturnStatement::codegen() const
{
    if (value)
        return std::format("return {};", (*value)->codegen());
    else
        return std::format("return;");
}

std::string IfStatement::codegen() const
{
    if (else_block) {
        return std::format("if ({}) {} else {}", condition->codegen(), then_block->codegen(), (*else_block)->codegen());
    } else {
        return std::format("if ({}) {}", condition->codegen(), then_block->codegen());
    }
}

std::string WhileStatement::codegen() const
{
    return std::format("while ({}) {}", condition->codegen(), block->codegen());
}

std::string Function::codegen() const
{
    const auto argument_string = arguments | std::views::transform([](const auto& argument) {
        return std::format("{} {}", argument.type, argument.name);
    }) | std::views::join_with(std::string_view { ", " })
        | std::ranges::to<std::string>();

    return std::format("{} {}({}) {}", return_type, name, argument_string, block->codegen());
}

std::string Enum::codegen() const
{
    const auto values_string = values | std::views::join_with(std::string_view { ",\n" }) | std::ranges::to<std::string>();
    return std::format("enum class {} {{\n {} }};\n", name, values_string);
}

std::string ParsedFile::codegen() const
{
    auto output = variables | std::views::transform([](const auto& variable) {
        return variable->codegen();
    }) | std::views::join_with(std::string_view { "\n\n" })
        | std::ranges::to<std::string>();

    output += functions | std::views::transform([](const auto& function) {
        return function->codegen();
    }) | std::views::join_with(std::string_view { "\n\n" })
        | std::ranges::to<std::string>();

    output += enums | std::views::transform([](const auto& function) {
        return function->codegen();
    }) | std::views::join_with(std::string_view { "\n\n" })
        | std::ranges::to<std::string>();

    return output;
}
