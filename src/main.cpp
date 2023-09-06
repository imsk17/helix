#include <fstream>
#include <iostream>
#include <sstream>
#include "tokenization.hpp"

std::vector<Token> tokenize(const std::string& str)
{

}
std::string tokens_to_asm(const std::vector<Token>& tokens) {
    std::stringstream output;
    output << "global _main\n_main:\n";
    for (int i = 0; i < tokens.size(); i++) {
        const Token& token = tokens.at(i);
        if (token.type == TokenType::exit) {
            if (i+1 <tokens.size() && tokens.at(i+1).type == TokenType::int_lit) {
                if (i+2 <tokens.size() && tokens.at(i+2).type == TokenType::semi) {
                    // Syscall for Exit Code on Mac OS
                    output << "    mov rax, 0x2000001\n";
                    output << "    mov rdi, " <<  tokens.at(i+1).value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }
    return output.str();
}
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

    std::string asmb = tokens_to_asm(tokens);

    {
        std::fstream file("out.asm", std::ios::out);
        file << asmb;
    }

    system("nasm -f macho64 out.asm");
    system("ld -o out out.o -arch x86_64 -macosx_version_min 10.13 -L /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib -lSystem");

    return EXIT_SUCCESS;
}
