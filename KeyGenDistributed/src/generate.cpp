#include "common.h"
#include "generate.h"

using namespace QryptSecurity;
using KeyValuePair = std::tuple<std::string, std::string>;

// Generate a key using the Qrypt SDK
void generate(std::vector<KeyValuePair> unparsed_args) {
    auto args = parseGenerateArgs(unparsed_args);

    // 1. Create and initialize our keygen client
    ::QryptSecurity::setLogLevel(args.log_level);
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

        // 3. Write key, plus metadata for bob
        writeKeyToFile(args.key_filename, keyInit.key, args.key_format);
        writeToFile(args.metadata_filename, keyInit.metadata);

        // 4. Display success
        printf("\nKey successfully created in %s.\n\n", args.key_filename.c_str());

    }
    // Bob is the receiver
    else if (args.role == "bob") {
        // 2. Read metadata from alice
        std::vector<uint8_t> metadata = readFromFile(args.metadata_filename);

        // 3. Generate the key using the metadata
        std::vector<uint8_t> keySync = keyGenClient->genSync(metadata);

        // 4. Write out key for decryption
        writeKeyToFile(args.key_filename, keySync, args.key_format);

        // 5. Display success
        printf("\nKey successfully created in %s.\n\n", args.key_filename.c_str());
    }
}

GenerateArgs parseGenerateArgs(std::vector<KeyValuePair> unparsed_args) {
    std::string role, key_filename, metadata_filename, cacert_path;
    std::string token = demo_token;
    std::string key_type = "aes";
    size_t key_len = 32;
    std::string key_format = "hexstr";
    std::string metadata_format = "text";
    ::QryptSecurity::LogLevel log_level = ::QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_DISABLE;

    for(auto[arg_name, arg_value] : unparsed_args) {
        try {
            GenerateFlag flag = GenerateFlagsMap.at(arg_name);
            switch(flag){
                case GEN_FLAG_ROLE: 
                    role = arg_value;
                    break;
                case GEN_FLAG_KEY_FILENAME:
                    key_filename = arg_value;
                    break;
                case GEN_FLAG_META_FILENAME:
                    metadata_filename = arg_value;
                    break;
                case GEN_FLAG_KEY_TYPE:
                    key_type = arg_value;
                    break;
                case GEN_FLAG_KEY_LEN:
                    key_len = stoi(arg_value);
                    break;
                case GEN_FLAG_TOKEN:
                    token = arg_value;
                    break;
                case GEN_FLAG_KEY_FORMAT:
                    key_format = arg_value;
                    break;
                case GEN_FLAG_CACERT_PATH:
                    cacert_path = arg_value;
                    break;
                case GEN_FLAG_LOG_LEVEL_DISABLE:
                case GEN_FLAG_LOG_LEVEL_TRACE:
                case GEN_FLAG_LOG_LEVEL_DEBUG:
                case GEN_FLAG_LOG_LEVEL_INFO:
                case GEN_FLAG_LOG_LEVEL_WARNING:
                case GEN_FLAG_LOG_LEVEL_ERROR:
                    if(!arg_value.empty()) {
                        throw invalid_arg_exception("Invalid argument: " + arg_name + "=" + arg_value);
                    }
                    log_level = (::QryptSecurity::LogLevel)flag;
            }
        } catch (std::out_of_range& /*ex*/) {
            throw invalid_arg_exception("Invalid argument: " + arg_name);
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
    if (key_format != "hexstr" && key_format != "binary") {
        throw invalid_arg_exception("Invalid key-format: \"" + key_format + "\"");
    }

    return {
        role, key_filename, metadata_filename, token, key_type, key_len, key_format,
        log_level, cacert_path
    };
}

void writeKeyToFile(std::string key_filename, std::vector<uint8_t> key, std::string key_format) {
    if (key_format == "binary") {
        writeToFile(key_filename, key);
    }
    else {
        writeToFile(key_filename, byteVecToHexStr(key) + "\n");
    }
}