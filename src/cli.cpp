#include "cli.h"
#include "encrypt.h"
#include "keygen.h"

#include "QryptSecurity/qryptsecurity_exceptions.h"

#include <iostream>
#include <fstream>
#include <cstring>

void printUsage(std::string mode) {
    if (mode == "generate") {
        std::cout << GenerateUsage;
    } else if (mode == "replicate") {
        std::cout << ReplicateUsage;
    } else if (mode == "encrypt") {
        std::cout << EncryptUsage;
    } else if (mode == "decrypt") {
        std::cout << DecryptUsage;
    } else {
        std::cout << GeneralUsage;
    }
}

int main(int argc, const char* argv[]) {
    // Set mode and handle --help
    if (argc < 2) {
        std::cout << GeneralUsage;
        return 0;
    }
    std::string mode = *++argv;
    for(int i=0; i<argc-1; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printUsage(mode);
            return 0;
        }
    }

    try{
        // Run a set of demonstration tests and the NIST suite
        if (mode == "test") {

        }
        // Create a key using the QryptSecurity SDK
        else if (mode == "generate" || mode == "replicate") {
            // Parse and unpack cli arguments
            auto keygen_args = parseKeygenArgs(++argv);
            const auto& [
                key_filename, metadata_filename, token, key_type, key_len, key_format, log_level, cacert_path
            ] = keygen_args;

            // Open input/output streams
            auto metadata_io_flags = ((mode == "generate") ? std::ios::out : std::ios::in) | std::ios::binary;
            std::fstream metadata_file(metadata_filename, metadata_io_flags);
            if (!metadata_file.is_open()) {
                throw std::invalid_argument("Unable to open metadata file " + metadata_filename);
            }
            std::ofstream key_file;
            if (!key_filename.empty()) {
                key_file.open(key_filename, std::ios::out | std::ios::binary);
                if (!key_file.is_open()) {
                    throw std::invalid_argument("Unable to open key file " + key_filename);
                }
            } else {
                std::cout << "Writing key to stdout: " << std::endl;
            }
            std::ostream& key_out((!key_file.is_open())? std::cout : key_file);

            // Create key
            KeyGen keygen_client(token, key_type, key_len, key_format, log_level, cacert_path);
            if (mode == "generate") {
                keygen_client.generate(key_out, metadata_file);
            } else { // mode == "replicate"
                keygen_client.replicate(key_out, metadata_file);
            }

            if (key_file.is_open()) {
                std::cout << "Wrote key to file: " << key_filename << std::endl;
                key_file.close();
            } else {
                std::cout << std::endl; // formatting when printing key to stdout
            }
            std::cout << "Wrote metadata to file: " << metadata_filename << std::endl;
            metadata_file.close();
        }
        // Use a key to encrypt or decrypt a file
        else if (mode == "encrypt" || mode == "decrypt") {
            // Parse and unpack cli arguments
            auto encrypt_decrypt_args = parseEncryptDecryptArgs(++argv);
            const auto& [
                input_filename, output_filename, key_filename, key_type, aes_mode, file_type
            ] = encrypt_decrypt_args;

            std::ifstream input_file(input_filename, std::ios::in | std::ios::binary);
            std::ifstream key_file(key_filename, std::ios::in | std::ios::binary);
            std::ofstream output_file(output_filename, std::ios::out | std::ios::binary);

            encryptDecrypt(mode, input_file, key_file, output_file, file_type, aes_mode, key_type);

            input_file.close();
            output_file.close();
            key_file.close();
        // Unrecognized command
        } else {
            std::cout << GeneralUsage;
            std::cout << "\nERROR: Unrecognized command. See 'KeyGen --help'.\n\n";
            return 1;
        }
    }
    catch (std::invalid_argument& ex) {
        printUsage(mode);
        std::cout << "\nERROR: " << ex.what() << std::endl << std::endl;
        return 1;
    }
    catch (QryptSecurity::QryptSecurityException& ex) {
        std::cout << "\nSDK ERROR: " << ex.what() << std::endl << std::endl;
        return 1;
    }
    return 0;
}

// Tokenize arguments into key-value pairs
std::tuple<std::string, std::string> tokenizeArg(std::string arg) {
    std::string flag, value;
    if(!(arg.find("--") == 0)) {
        throw std::invalid_argument("Invalid argument: " + arg);
    }
    size_t delim_pos = arg.find("=");
    flag = arg.substr(0, delim_pos);
    if (arg.length() > delim_pos) {
        value = arg.substr(delim_pos + 1);
    }
    return {flag, value};
}

KeygenArgs parseKeygenArgs(const char** unparsed_args) {
    std::string key_filename, token, cacert_path;
    std::string metadata_filename = "meta.dat";
    std::string key_type = "aes";
    size_t key_len = 32;
    std::string key_format = "hexstr";
    std::string metadata_format = "text";
    ::QryptSecurity::LogLevel log_level = ::QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_DISABLE;

    while(*unparsed_args) {
        auto[arg_name, arg_value] = tokenizeArg(*unparsed_args++);
        try {
            KeygenFlag flag = KeygenFlagsMap.at(arg_name);
            switch(flag) {
                case KEYGEN_FLAG_KEY_FILENAME:
                    key_filename = arg_value;
                    break;
                case KEYGEN_FLAG_META_FILENAME:
                    metadata_filename = arg_value;
                    break;
                case KEYGEN_FLAG_KEY_TYPE:
                    key_type = arg_value;
                    break;
                case KEYGEN_FLAG_KEY_LEN:
                    key_len = stoi(arg_value);
                    break;
                case KEYGEN_FLAG_TOKEN:
                    token = arg_value;
                    break;
                case KEYGEN_FLAG_KEY_FORMAT:
                    key_format = arg_value;
                    break;
                case KEYGEN_FLAG_CACERT_PATH:
                    cacert_path = arg_value;
                    break;
                case KEYGEN_FLAG_LOG_LEVEL_DISABLE:
                case KEYGEN_FLAG_LOG_LEVEL_TRACE:
                case KEYGEN_FLAG_LOG_LEVEL_DEBUG:
                case KEYGEN_FLAG_LOG_LEVEL_INFO:
                case KEYGEN_FLAG_LOG_LEVEL_WARNING:
                case KEYGEN_FLAG_LOG_LEVEL_ERROR:
                    if(!arg_value.empty()) {
                        throw std::invalid_argument("Invalid argument: " + arg_name + "=" + arg_value);
                    }
                    log_level = (::QryptSecurity::LogLevel)flag;
            }
        } catch (std::out_of_range& /*ex*/) {
            throw std::invalid_argument("Invalid argument: " + arg_name);
        }
    }

    if (key_filename.empty() && key_format == "binary") {
        throw std::invalid_argument("Cannot output key with key-type \"binary\" to stdout!");
    }
    if (key_type != "aes" && key_type != "otp") {
        throw std::invalid_argument("Invalid key-type: \"" + key_type + "\"");
    }
    if (key_format != "hexstr" && key_format != "binary") {
        throw std::invalid_argument("Invalid key-format: \"" + key_format + "\"");
    }

    return { key_filename, metadata_filename, token, key_type, key_len, key_format, log_level, cacert_path };
}

EncryptDecryptArgs parseEncryptDecryptArgs(const char** unparsed_args) {
    std::string input_filename, output_filename, key_filename;
    std::string key_type = "otp";
    std::string aes_mode = "ecb";
    std::string file_type = "binary";

    while(*unparsed_args) {
        auto[arg_name, arg_value] = tokenizeArg(*unparsed_args++);
        try {
            switch(EncryptDecryptFlagsMap.at(arg_name)){
                case CRYPT_FLAG_INPUT_FILENAME:
                    input_filename = arg_value;
                    break;
                case CRYPT_FLAG_OUTPUT_FILENAME:
                    output_filename = arg_value;
                    break;
                case CRYPT_FLAG_KEY_FILENAME:
                    key_filename = arg_value;
                    break;
                case CRYPT_FLAG_KEY_TYPE:
                    key_type = arg_value;
                    break;
                case CRYPT_FLAG_AES_MODE:
                    aes_mode = arg_value;
                    break;
                case CRYPT_FLAG_FILE_TYPE:
                    file_type = arg_value;
            }
        } catch (std::out_of_range& /*ex*/) {
            throw std::invalid_argument("Invalid argument: " + std::string(arg_name));
        }
    }
    if (input_filename.empty()) {
        throw std::invalid_argument("Missing input-filename");
    }
    if (output_filename.empty()) {
        throw std::invalid_argument("Missing output-filename");
    }
    if (key_filename.empty()) {
        throw std::invalid_argument("Missing key-filename");
    }
    if (key_type != "aes" && key_type != "otp") {
        throw std::invalid_argument("Invalid key-type: \"" + key_type + "\"");
    }
    if (aes_mode != "ecb" && aes_mode != "ocb") {
        throw std::invalid_argument("Invalid aes-mode: \"" + aes_mode + "\"");
    }
    if (file_type != "binary" && file_type != "bitmap") {
        throw std::invalid_argument("Invalid file-type: \"" + file_type + "\"");
    }

    return {input_filename, output_filename, key_filename, key_type, aes_mode, file_type};
}