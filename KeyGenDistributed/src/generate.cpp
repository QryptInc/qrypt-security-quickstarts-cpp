#include "QryptSecurity/qryptsecurity.h"
#include "QryptSecurity/qryptsecurity_exceptions.h"
#include "QryptSecurity/qryptsecurity_logging.h"
#include "common.h"

#include <fstream>
#include <cstring>

using namespace QryptSecurity;

void generate(GenerateArgs& args) {
    // 1. Create and initialize our keygen client
    auto keyGenClient = IKeyGenDistributedClient::create();
    if (args.cacert_path.empty()) {
        keyGenClient->initialize(args.token);
    }
    else {
        keyGenClient->initialize(args.token, args.cacert_path);
    }
    
    // Alice is the sender
    if (args.role == "alice") {
        // 2. Generate the key and metadata
        SymmetricKeyData keyInit = {};
        if (args.key_type == "aes") {
            keyInit = keyGenClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256);
        }
        else if (args.key_type == "otp") {
            keyInit = keyGenClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, args.key_len);
        }

        // 3. Write out metadata for bob and key for encryption
        writeToFile(args.metadata_filename, keyInit.metadata);
        if (args.output_format == "vector") {
            writeToFile(args.key_filename, keyInit.key);
        }
        else {
            std::string key = convertByteVecToHexStr(keyInit.key) + "\n";
            writeToFile(args.key_filename, key);
        }

        // 4. Display success
        printf("\nKey successfully created in %s.\n\n", args.key_filename.c_str());

    }
    // Bob is the receiver
    else if (args.role == "bob") {
        // 2. Read in metadata from alice
        std::vector<uint8_t> metadata = readFromFile(args.metadata_filename);

        // 3. Generate the key using the metadata
        std::vector<uint8_t> keySync = keyGenClient->genSync(metadata);

        // 4. Write out key for decryption
        if (args.output_format == "vector") {
            writeToFile(args.key_filename, keySync);
        }
        else {
            std::string key = convertByteVecToHexStr(keySync) + "\n";
            writeToFile(args.key_filename, key);
        }

        // 5. Display success
        printf("\nKey successfully created in %s.\n\n", args.key_filename.c_str());
    }
}

const std::string demo_token = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6IjEzNThlMjk0YTc0NjQ5MTc5ZGU4ZTY0YTlhNjI3YjA"
    "zIn0.eyJleHAiOjE3MDI0MTkyMDAsIm5iZiI6MTY3MDg4MzIwMCwiaXNzIjoiQVVUSCIsImlhdCI6MTY3MDg4MzIwMCwiZ3JwcyI6WyJQVUIiLCJQV"
    "UIiXSwiYXVkIjpbIlFERUEiLCJSUFMiXSwicmxzIjpbIlFERVVTUiIsIlJORFVTUiJdLCJjaWQiOiJzTXZNZWNGTWMwYkFWZ0YxSjNhSmsiLCJkdmM"
    "iOiIwNDk2NmNiZTVkYTQ0ZmQ2ODYzMTRhMTE1ODllMjllMiIsImp0aSI6IjUyOGY2Y2EyYzBiNzQ3NDBiNGIzMmZjYTI0ZDljM2U1IiwidHlwIjozf"
    "Q.c-9-bfaXMKLDf_WhdhNPj0GOrCsSi7-mW-0gBd0Dse4mYiTwm5iTyp6ezsjOmQz6R3WXnaHE5BBX9lYEM6KZ1Q";

GenerateArgs::GenerateArgs(std::vector<CliArgument> unparsed_args) {
    // Set defaults
    key_type = "otp";
    key_len = 32;
    output_format = "hexstr";
    token = demo_token;
    ::QryptSecurity::setLogLevel(::QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_DISABLE);

    for(CliArgument arg : unparsed_args) {
        try {
            GenerateFlag flag_value = GenerateFlagsMap.at(arg.flag_string);
            switch(flag_value){
                case GEN_FLAG_ROLE: 
                    role = arg.value;
                    break;
                case GEN_FLAG_KEY_FILENAME:
                    key_filename = arg.value;
                    break;
                case GEN_FLAG_META_FILENAME:
                    metadata_filename = arg.value;
                    break;
                case GEN_FLAG_KEY_TYPE:
                    key_type = arg.value;
                    break;
                case GEN_FLAG_KEY_LEN:
                    key_len = stoi(arg.value);
                    break;
                case GEN_FLAG_TOKEN:
                    token = arg.value;
                    break;
                case GEN_FLAG_FORMAT:
                    output_format = arg.value;
                    break;
                case GEN_FLAG_CACERT_PATH:
                    cacert_path = arg.value;
                    break;
                case GEN_FLAG_LOG_LEVEL_DISABLE:
                case GEN_FLAG_LOG_LEVEL_TRACE:
                case GEN_FLAG_LOG_LEVEL_DEBUG:
                case GEN_FLAG_LOG_LEVEL_INFO:
                case GEN_FLAG_LOG_LEVEL_WARNING:
                case GEN_FLAG_LOG_LEVEL_ERROR:
                    if(!arg.value.empty()) throw invalid_arg_exception("Invalid argument: " + std::string(arg));
                    ::QryptSecurity::setLogLevel((::QryptSecurity::LogLevel)flag_value);
                    break;
                default:
                    throw invalid_arg_exception("Invalid argument: " + std::string(arg));
            }
        } catch (std::out_of_range& /*ex*/) {
            throw invalid_arg_exception("Invalid argument: " + std::string(arg));
        }
    }

    if (role.empty()) {
        throw invalid_arg_exception("Missing role");
    }
    if (role != "alice" && role != "bob") {
        throw invalid_arg_exception("Invalid role: \"" + role + "\"");
    }
    if (key_filename.empty()) {
        throw invalid_arg_exception("Missing key-filename");
    }
    if (metadata_filename.empty()) {
        throw invalid_arg_exception("Missing metadata-filename");
    }
    if (key_type != "aes" && key_type != "otp") {
        throw invalid_arg_exception("Invalid key-type: \"" + key_type + "\"");
    }
    if (output_format != "hexstr" && output_format != "binary") {
        throw invalid_arg_exception("Invalid output-format: \"" + output_format + "\"");
    }
}

std::string getUsage() {
    std::string usage = 
    "\n"
    "KeyGenDistributed\n"
    "=====================\n"
    "\n"
    "Exercises distributed key generation.\n"
    "\n"
    "Options\n"
    "-------\n"
    "--user=<alice|bob>              Set the user to either alice or bob.\n"
    "\n"
    "--token=<token>                 Qrypt token retrieved from Qrypt portal (https://portal.qrypt.com).\n"
    "                                Make sure the token has the BLAST scope.\n"
    "\n"
    "--key-type=<aes|otp>            Set to the type of key you would like to produce.\n"
    "                                aes - AES-256 key with length 32 bytes.\n"
    "                                otp - One time pad with length provided by the otp-len parameter.\n"
    "\n"
    "--otp-len=<length>              The length of the one time pad (in bytes) if key type is otp.\n"
    "\n"
    "--metadata-filename=<filename>  The filename for the metadata file to be created or consumed.\n"
    "\n"
    "--key-filename=<filename>       The filename to save the generated key.\n"
    "\n"
    "--random-format=<hexstr|vector> Set the output format of the key you would like to produce.\n"
    "                                hexstr - key will be output in hex format.\n"
    "                                vector - key will be ouput in binary format.\n"
    "                                Defaults to hexstr format.\n"
    "\n"
    "--ca-cert=<path>                Full or relative path to a public root ca-certificate (such as the one available\n"
    "                                at https://curl.se/docs/caextract.html) for TLS traffic with the Qrypt servers.\n"
    "                                Use this option if the system does not have accessible root certificates or\n"
    "                                if key generation persistently returns curl error 60 (ssl certificate problem).\n"
    "\n"
    "--log_level_<level>             Set logging level.\n" 
    "                                Defaults to --log_level_disable\n"
    "\n"
    "--help                          Display help.\n"
    "\n"
    "";

    return usage;
}

void displayUsage() {
    std::string usage = getUsage();
    printf("%s", usage.c_str());
}