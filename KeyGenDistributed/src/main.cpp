#include "QryptSecurity/qryptsecurity.h"
#include "QryptSecurity/qryptsecurity_exceptions.h"
#include "QryptSecurity/qryptsecurity_logging.h"
#include "common.h"

#include <fstream>
#include <cstring>

using namespace QryptSecurity;

int main(int argc, char** argv) {
    if (argc < 2) {
        // print general usage
        return 1;
    }
    std::string mode = *++argv; // first argument after the executable name
    std::vector<CliArgument> unparsed_args;
    try {
        while(*++argv) {
            CliArgument argument(*argv);
            if (argument.flag_string == "help") {
                // print usage
                return 0;
            }
            unparsed_args.push_back(argument);
        } 
        if (mode == "test") {

        }
        else if (mode == "generate") {
            GenerateArgs generate_args(unparsed_args);
            generate(generate_args);
        }
        else if (mode == "encrypt" || mode == "decrypt") {

        } else {

        }
    }
    catch (invalid_arg_exception& ex) {
        printf("\nError: %s\n", ex.what());
        // print usage
        return 1;
    }
    catch (QryptSecurity::QryptSecurityException& ex) {
        printf("\nSDK Error: %s\n", ex.what());
        return 1;
    }
    return 0;
}
