#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "compressor.h"

[[noreturn]] void usage(const char *progname)
{
    std::cerr << "A small tool helping you read & write SuConfig file easily."
              << std::endl;
    std::cerr << "Usage: " << progname
              << " {-c | -d} <config file> [<output file>]"
              << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    std::string read_filename, write_filename;
    bool decompress = false;

    if (argc < 3)
        usage(argv[0]);

    read_filename = argv[2];

    if (!strcmp(argv[1], "-c")) {
        write_filename = argc > 3 ? argv[3] : read_filename + "_compressed";
        decompress = false;

    } else if (!strcmp(argv[1], "-d")) {
        write_filename = argc > 3 ? argv[3] : read_filename + "_decompressed";
        decompress = true;

    } else
        usage(argv[0]);

    std::ifstream ifs(read_filename);
    std::ofstream ofs(write_filename);

    if (!ifs) {
        std::cerr << "Could not open file " << read_filename << std::endl;
        return EXIT_FAILURE;
    }

    if (!ofs) {
        std::cerr << "Could not open file " << write_filename << std::endl;
        return EXIT_FAILURE;
    }

    // yes, the codes are copied from suconfigfile.cpp
    if (decompress) {
        ifs.seekg(0, std::ios::end);
        unsigned orig_size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        unsigned char *ibuf = new unsigned char[orig_size + 1];

        for (unsigned i = 0; !ifs.eof(); i++)
            ibuf[i] = ~ifs.get();

        ifs.close();
        unsigned char *obuf = nullptr;
        unsigned decompress_size = Decompress(ibuf, obuf, orig_size, 0);
        obuf = new unsigned char[decompress_size];
        Decompress(ibuf, obuf, orig_size, decompress_size);

        if (!ofs.write(reinterpret_cast<const char *>(obuf), decompress_size)) {
            std::cerr << "ERROR: write file " << write_filename << " failed."
                      << std::endl;
            delete[] ibuf;
            delete[] obuf;
            ibuf = obuf = nullptr;
            return EXIT_FAILURE;
        }

        delete[] ibuf;
        delete[] obuf;
        ibuf = obuf = nullptr;
        return EXIT_SUCCESS;
    }

    ifs.seekg(0, std::ios::end);
    unsigned orig_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    unsigned char *ibuf = new unsigned char[orig_size + 1];
    ifs.read(reinterpret_cast<char *>(ibuf), orig_size);
    ifs.close();
    unsigned char *obuf = nullptr;
    unsigned comp_size = Compress(ibuf, obuf, orig_size, 0);
    obuf = new unsigned char[comp_size];
    Compress(ibuf, obuf, orig_size, comp_size);
    // *INDENT-OFF*
    std::for_each(obuf, obuf + comp_size + 1, [](unsigned char &i) { i = ~i; });
    // *INDENT-ON*

    if (!ofs.write(reinterpret_cast<const char *>(obuf), comp_size)) {
        std::cerr << "ERROR: write file " << write_filename << " failed."
                  << std::endl;
        delete[] ibuf;
        delete[] obuf;
        ibuf = obuf = nullptr;
        return false;
    }

    delete[] ibuf;
    delete[] obuf;
    ibuf = obuf = nullptr;
    return EXIT_SUCCESS;
}
