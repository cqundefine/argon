#include <stddef.h>

class String
{
public:
    constexpr String(const char* data, size_t length)
        : m_data(data)
        , m_length(length)
        , m_owned(false)
    {
    }

    const char* data() const
    {
        return m_data;
    }

    size_t length() const
    {
        return m_length;
    }

private:
    const char* m_data;
    size_t m_length;

    bool m_owned;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuser-defined-literals"

[[nodiscard]] inline __attribute__((always_inline)) consteval String operator""s(const char* cstring, size_t length)
{
    return String(cstring, length);
}

#pragma clang diagnostic pop
