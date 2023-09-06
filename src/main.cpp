#include <fstream>
#include <iostream>
#include <sstream>

#include "generator.hpp"

int main(int argc, char* argv[])
{

    if (argc != 2) {
        std::cerr << "Incorrect Usage. Correct Usage is .." << std::endl;
        std::cerr << "helix <file.he>" << std::endl;
        return EXIT_FAILURE;
    }
    std::string contents;
    {
        std::fstream input(argv[1], std::ios::in);
        std::stringstream content_stream;
        content_stream << input.rdbuf();
        contents = content_stream.str();
    }
    Tokenizer tokenizer(std::move(contents));
    auto tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));

    auto tree = parser.parse();

    if (!tree.has_value()) {
        std::cerr << "No exit statement found" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(tree.value());
    auto assembly = generator.generate();

    {
        std::fstream file("out.asm", std::ios::out);
        file << assembly;
    }

    system("nasm -f macho64 out.asm");
    system("ld -o out out.o -arch x86_64 -macosx_version_min 10.13 -L /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib -lSystem");

    return EXIT_SUCCESS;
}
