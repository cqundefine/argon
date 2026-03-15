#pragma once

#include <print>

template <class... Args>
[[noreturn]] void error(std::string_view msg, Args&&... args)
{
    std::println("Error: {}", std::vformat(msg, std::make_format_args(args...)));
    std::exit(1);
}
