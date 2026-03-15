#include <Compiler/Globals.h>
#include <Compiler/Lexer.h>
#include <Compiler/Parser.h>
#include <External/argparse.hpp>
#include <Shared/Utils/Utils.h>

#include <cassert>
#include <cctype>
#include <fstream>
#include <ios>
#include <print>
#include <string>
#include <unistd.h>

int main(int argc, char** argv)
{
    argparse::ArgumentParser program("njsc");

    program.add_argument("filename").help("input file");
    program.add_argument("-o").help("output file");
    // program.add_argument("-c").help("compile to object file").flag();
    // program.add_argument("-r", "--run").help("run executable").flag();
    // program.add_argument("--target").help("target triple");
    // program.add_argument("--disable-dce").help("disable dead code elimination").flag();

    // auto& optimize_debug_group = program.add_mutually_exclusive_group();
    // optimize_debug_group.add_argument("-O").help("optimize").flag();
    // optimize_debug_group.add_argument("-g").help("add debug information").flag();

    // auto& dumpGroup = program.add_mutually_exclusive_group();
    // dumpGroup.add_argument("--dump-tokens-before-preprocessor").help("dump tokens before preprocessor").flag();
    // dumpGroup.add_argument("--dump-tokens").help("dump tokens").flag();
    // dumpGroup.add_argument("--dump-ast").help("dump AST").flag();
    // dumpGroup.add_argument("--dump-ir").help("dump IR").flag();
    // dumpGroup.add_argument("--dump-asm").help("dump assembly").flag();

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        exit(1);
    }

    const auto file = read_file(program.get("filename"));
    const auto tokens = lex_file(file);

    for (const auto token : tokens)
        std::println("{}", token);

    auto parser = Parser(tokens);
    const auto parsed_file = parser.parse();
    parsed_file->dump();

    auto output = program.present("-o") ? program.get("-o") : "out";

    std::fstream f;
    f.open(output + ".cpp", std::ios::out);
    f << "#include <argon_runtime.h>\n\n";
    f << parsed_file->codegen();
    f << "\n";
    f.close();

    system((std::string("clang++ -std=c++23 -ISources/Runtime ") + output + ".cpp" + " -o " + output).c_str());
}
