#include <iostream>
#include <fstream>

static void interface(std::string in_filename, std::string out_filename, std::ifstream &in, std::ofstream &out, int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " " << in_filename << " " << out_filename << std::endl;
        exit(1);
    }

    in = std::ifstream(argv[1], std::ios::binary);
    if (!in.is_open()) {
        std::cout << "Could not open input file" << std::endl;
        exit(1);
    }

    out = std::ofstream(argv[2], std::ios::binary);
    if (!out.is_open()) {
        std::cout << "Could not open output file" << std::endl;
        exit(1);
    }
}
