#include <unistd.h>

using int8 = __INT8_TYPE__;
using int16 = __INT16_TYPE__;
using int32 = __INT32_TYPE__;
using int64 = __INT64_TYPE__;

using uint8 = __UINT8_TYPE__;
using uint16 = __UINT16_TYPE__;
using uint32 = __UINT32_TYPE__;
using uint64 = __UINT64_TYPE__;

#include "String.h"

inline void print(String string)
{
    write(STDOUT_FILENO, string.data(), string.length());
}
