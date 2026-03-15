#pragma once

#include <cassert>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

#define ALWAYS_INLINE __attribute__((always_inline)) inline

template <typename From, typename To>
using CopyConst = std::conditional_t<std::is_const_v<From>, std::add_const_t<To>, std::remove_const_t<To>>;

template <typename T, typename... U>
concept IsAnyOf = (std::same_as<T, U> || ...);

template <typename T, typename Generator, typename Predicate>
std::vector<T> collect_until(Generator generator, Predicate predicate)
{
    std::vector<T> values;
    while (true) {
        auto value = generator();
        values.push_back(value);
        if (predicate(value))
            break;
    }
    return values;
}

std::string join(std::span<const std::string> values, std::string_view separator);

std::vector<char> read_file(const std::filesystem::path& filename);
