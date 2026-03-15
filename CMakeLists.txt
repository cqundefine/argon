cmake_minimum_required(VERSION 3.25)

project(Argon)

set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wno-deprecated-declarations -Wno-return-type-c-linkage)
include_directories(Sources)

set(SHARED_SOURCES
    Sources/Shared/Utils/Utils.cpp
)

add_executable(argon
    Sources/Compiler/Codegen.cpp
    Sources/Compiler/Lexer.cpp
    Sources/Compiler/main.cpp
    Sources/Compiler/Parser.cpp

    ${SHARED_SOURCES}
)

add_library(argon_runtime STATIC
    ${SHARED_SOURCES}
)

target_compile_options(argon_runtime PRIVATE -fsanitize=address)
