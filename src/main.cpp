#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {

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

    std::cout << contents << std::endl;


    return EXIT_SUCCESS;
}
