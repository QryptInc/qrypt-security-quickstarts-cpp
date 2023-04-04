#include "common.h"
#include "generate.h"

#include <fstream>
#include <cstring>

using namespace QryptSecurity;

static const char* GeneralUsage = "Commands:\n"
    "    test        Run a set of tests for demonstrating key generation and NIST randomness.\n"
    "    generate    Init/sync an AES-256 key or one-time-pad using BLAST distributed key generation.\n"
    "    encrypt     Encrypt data using an AES-256 key or one-time-pad.\n"
    "    decrypt     Decrypt data using an AES-256 key or one-time-pad.\n";

void printUsage(std::string mode) {
    if (mode == "generate") {
        printf(GenerateUsage);
    } else if (mode == "encrypt" || mode == "decrypt") {

    } else {
        printf(GeneralUsage);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf(GeneralUsage);
        return 0;
    }

    std::string mode = *++argv; // first argument after the executable name
    if(mode == "--help") {
        printf(GeneralUsage);
        return 0;
    }

    bool help = false;
    std::vector<KeyValuePair> args;
    try {
        while(*++argv) {
            std::string arg = *argv;
            if (arg == "--help") {
                printUsage(mode);
                return 0;
            }
            args.push_back(tokenizeArg(arg));
        } 
        if (mode == "test") {

        }
        else if (mode == "generate") {
            generate(args);
        }
        else if (mode == "encrypt" || mode == "decrypt") {

        } else {
            printf(GeneralUsage);
            printf("\nERROR: Unrecognized command. See 'KeyGen --help'.\n");
            return 1;
        }
    }
    catch (invalid_arg_exception& ex) {
        printUsage(mode);
        printf("\nERROR: %s\n", ex.what());
        return 1;
    }
    catch (QryptSecurity::QryptSecurityException& ex) {
        printf("SDK ERROR: %s\n", ex.what());
        return 1;
    }
    return 0;
}
