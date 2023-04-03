#ifndef COMMON_H
#define COMMON_H

#include "QryptSecurity/qryptsecurity_logging.h"
#include <exception>
#include <map>
#include <vector>
#include <string>

const uint32_t IVLengthInBytes = 12;
const uint32_t AESKeyLengthInBytes = 32;
const uint32_t AESKeyWithIVLengthInBytes = AESKeyLengthInBytes + IVLengthInBytes;
const uint32_t AETagSizeInBytes = 2;
constexpr int OPENSSL_SUCCESS = 1;
constexpr int BMP_HEADER_SIZE = 54;

struct BitmapData {
    std::vector<uint8_t> header;
    std::vector<uint8_t> body;
};

class invalid_arg_exception : public std::exception {
  private:
    std::string _ExceptionMsg;

  public:
    invalid_arg_exception(std::string message) {
        _ExceptionMsg = message;
    }
    ~invalid_arg_exception() = default;
    const char *what() const noexcept override { return _ExceptionMsg.c_str(); };
};

class CliArgument {
public:
    std::string flag_string;
    std::string value;
    CliArgument(std::string flag_string, std::string value) : flag_string(flag_string), value(value) {}; 
    CliArgument(std::string arg) {
        if(!(arg.find("--") == 0)) {
            throw invalid_arg_exception("Invalid argument: " + arg);
        }
        size_t delim_pos = arg.find("=");
        flag_string = arg.substr(0, delim_pos);
        if (arg.length() > delim_pos) {
            value = arg.substr(delim_pos + 1);
        } else {
            value = "";
        }
    }
    operator std::string() const { return (value.empty())? flag_string : flag_string + "=" + value;}
};

enum GenerateFlag {
    GEN_FLAG_LOG_LEVEL_DISABLE = ::QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_DISABLE,
    GEN_FLAG_LOG_LEVEL_ERROR = ::QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_ERROR,
    GEN_FLAG_LOG_LEVEL_WARNING = ::QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_WARNING,
    GEN_FLAG_LOG_LEVEL_INFO = ::QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_INFO,
    GEN_FLAG_LOG_LEVEL_DEBUG = ::QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_DEBUG,
    GEN_FLAG_LOG_LEVEL_TRACE = ::QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_TRACE,
    GEN_FLAG_ROLE,
    GEN_FLAG_KEY_FILENAME,
    GEN_FLAG_META_FILENAME,
    GEN_FLAG_KEY_TYPE,
    GEN_FLAG_KEY_LEN,
    GEN_FLAG_TOKEN,
    GEN_FLAG_FORMAT,
    GEN_FLAG_CACERT_PATH
};

static const std::map<std::string, GenerateFlag> GenerateFlagsMap = {
    {"--role", GEN_FLAG_ROLE},
    {"--key-filename", GEN_FLAG_KEY_FILENAME},
    {"--metadata-filename", GEN_FLAG_META_FILENAME},
    {"--key-type", GEN_FLAG_KEY_TYPE},
    {"--key-len", GEN_FLAG_KEY_LEN},
    {"--token", GEN_FLAG_TOKEN},
    {"--output-format", GEN_FLAG_FORMAT},
    {"--ca-cert", GEN_FLAG_CACERT_PATH},
    {"--log-level-disable", GEN_FLAG_LOG_LEVEL_DISABLE},
    {"--log-level-trace", GEN_FLAG_LOG_LEVEL_TRACE},
    {"--log-level-debug", GEN_FLAG_LOG_LEVEL_DEBUG},
    {"--log-level-info", GEN_FLAG_LOG_LEVEL_INFO},
    {"--log-level-warning", GEN_FLAG_LOG_LEVEL_WARNING},
    {"--log-level-error", GEN_FLAG_LOG_LEVEL_ERROR}
};

class GenerateArgs {
public:
    std::string role;
    std::string key_filename;
    std::string metadata_filename;
    std::string key_type;
    size_t key_len;
    std::string token;
    std::string output_format;
    std::string cacert_path;

    GenerateArgs(std::vector<CliArgument> unparsed_args);
    // std::string usage();
};

enum EncryptDecryptFlag {
    CRYPT_FLAG_GENERATE,
    CRYPT_FLAG_INPUT_FILENAME,
    CRYPT_FLAG_OUTPUT_FILENAME,
    CRYPT_FLAG_KEY_FILENAME,
    CRYPT_FLAG_META_FILENAME,
    CRYPT_FLAG_KEY_TYPE,
    CRYPT_FLAG_AES_MODE,
    CRYPT_FLAG_FILE_TYPE
};

static const std::map<std::string, EncryptDecryptFlag> EncryptDecryptFlagsMap = {
    {"--g", CRYPT_FLAG_GENERATE},
    {"--generate", CRYPT_FLAG_GENERATE},
    {"--input-filename", CRYPT_FLAG_INPUT_FILENAME},
    {"--output-filename", CRYPT_FLAG_OUTPUT_FILENAME},
    {"--key-filename", CRYPT_FLAG_KEY_FILENAME},
    {"--metadata-filename", CRYPT_FLAG_META_FILENAME},
    {"--key-type", CRYPT_FLAG_KEY_TYPE},
    {"--aes-mode", CRYPT_FLAG_AES_MODE},
    {"--file-type", CRYPT_FLAG_FILE_TYPE}
};

class EncryptDecryptArgs {
    bool generate;
    std::string input_filename;
    std::string output_filename;
    std::string key_filename;
    std::string metadata_filename;
    std::string key_type;
    std::string aes_mode;
    std::string file_type;

    GenerateArgs generate_args;

    EncryptDecryptArgs(std::vector<CliArgument> unparsed_args);
    // std::string usage();
};

void generate(GenerateArgs& args);

// void encrypt(EncryptDecryptArgs& args);

// void decrypt(EncryptDecryptArgs& args);

std::vector<uint8_t> readFromFile(const std::string filename);

std::vector<uint8_t> readFromHexFile(const std::string filename);

void writeToFile(const std::string filename, std::vector<uint8_t>& buffer);

void writeToFile(const std::string filename, std::string &buffer);

BitmapData readBitmap(const std::string filename);

void writeBitmap(const std::string filename, const BitmapData& bitmapData);

std::vector<uint8_t> xorVectors(const std::vector<uint8_t> otp, const std::vector<uint8_t> &data);

std::string convertByteVecToHexStr(std::vector<uint8_t> bytes);

size_t hexCharToInt(char input);

std::vector<uint8_t> hexStrToByteVec(std::string str);

std::string toUpper(std::string str);

#endif