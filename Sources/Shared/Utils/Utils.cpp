#include <Shared/Utils/Utils.h>

#include <fstream>
#include <print>

std::string join(std::span<const std::string> values, std::string_view separator)
{
    if (values.empty())
        return {};

    std::string out = values.front();
    for (size_t i = 1; i < values.size(); ++i) {
        out += separator;
        out += values[i];
    }
    return out;
}

std::vector<char> read_file(const std::filesystem::path& filename)
{
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::println(stderr, "Can't open file: {}", filename.string());
        std::exit(1);
    }

    auto length { std::filesystem::file_size(filename) };
    std::vector<char> result(length);
    file.read(std::bit_cast<char*>(result.data()), static_cast<long>(length));

    return result;
}
