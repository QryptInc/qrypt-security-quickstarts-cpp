#ifndef QRYPTCLI_H
#define QRYPTCLI_H

#include "QryptSecurity/qryptsecurity_logging.h"

#include <map>
#include <string>
#include <vector>

static const char* GeneralUsage = 
    "Commands:\n"
    "  generate    Initialize an AES-256 key or one-time-pad using BLAST distributed key generation.\n"
    "  replicate   Re-create an AES-256 key or one-time-pad from stored metadata, using BLAST distributed key generation.\n"
    "  encrypt     Encrypt data using an AES-256 key or one-time-pad.\n"
    "  decrypt     Decrypt data using an AES-256 key or one-time-pad.\n"
#ifdef ENABLE_TESTS
    "  test        Validate the Qrypt SDK using a set of end-to-end tests.\n"
#endif
    "\n";

static const char* GenerateUsage = 
    "Usage: qrypt generate [Optional Arguments]\n"
    "\n"
    "Initialize an AES-256 key or one-time-pad using BLAST distributed key generation. Also outputs a metadata file for\n"
    "replicating the same key at a different time/location."
    "\n"
    "Optional Arguments:\n"
    "  --help                          Display this message.\n"
    "  --token=<token>                 API token for portal.qrypt.com. Defaults to a demo token.\n"
    "  --key-filename=<filename>       Path of the key output file. Prints key to stdout if not set.\n"
    "  --metadata-filename=<filename>  Path of the metadata output file. Default \"./meta.dat\"\n"
    "  --key-type=<aes|otp>            Type of key to generate; AES-256 or one-time-pad. Default \"otp\".\n"
    "  --key-len=<byte_length>         Length of key to generate, if otp. Ignored for aes. Default 32.\n"
    "  --key-format=<hexstr|binary>    Key output format. Default \"hexstr\".\n"
    "                                  hexstr - key file will be in human-readable hex format.\n"
    "                                  binary - key file will be in binary format.\n"
    "  --log-level-<level>             Set logging level for the Qrypt SDK. Default \"disable\".\n"
    "  --ca-cert=<path>                Full or relative path to a public root ca-certificate (such as the one at\n"
    "                                  https://curl.se/docs/caextract.html) for TLS traffic with the Qrypt servers.\n"
    "                                  Use this option if the system does not have accessible root certificates or\n"
    "                                  if key generation persistently returns curl error 60 (ssl certificate problem).\n"
    "\n";

static const char* ReplicateUsage = 
    "Usage: qrypt replicate [Optional Arguments]\n"
    "\n"
    "Re-create an AES-256 key or one-time-pad from stored metadata, using BLAST distributed key generation."
    "\n"
    "Optional Arguments:\n"
    "  --help                          Display this message.\n"
    "  --token=<token>                 API token for portal.qrypt.com. Defaults to a demo token.\n"
    "  --key-filename=<filename>       Path of the key output file. Prints key to stdout if not set.\n"
    "  --metadata-filename=<filename>  Path of the metadata input file. Default \"./meta.dat\"\n"
    "  --key-format=<hexstr|binary>    Key output format. Default \"hexstr\".\n"
    "                                  hexstr - key file will be in human-readable hex format.\n"
    "                                  binary - key file will be in binary format.\n"
    "  --log-level-<level>             Set logging level for the Qrypt SDK. Default \"disable\".\n"
    "  --ca-cert=<path>                Full or relative path to a public root ca-certificate (such as the one at\n"
    "                                  https://curl.se/docs/caextract.html) for TLS traffic with the Qrypt servers.\n"
    "                                  Use this option if the system does not have accessible root certificates or\n"
    "                                  if key generation persistently returns curl error 60 (ssl certificate problem).\n"
    "\n";

enum KeygenFlag {
    KEYGEN_FLAG_LOG_LEVEL_DISABLE = (int)QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_DISABLE,
    KEYGEN_FLAG_LOG_LEVEL_ERROR = (int)QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_ERROR,
    KEYGEN_FLAG_LOG_LEVEL_WARNING = (int)QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_WARNING,
    KEYGEN_FLAG_LOG_LEVEL_INFO = (int)QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_INFO,
    KEYGEN_FLAG_LOG_LEVEL_DEBUG = (int)QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_DEBUG,
    KEYGEN_FLAG_LOG_LEVEL_TRACE = (int)QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_TRACE,
    KEYGEN_FLAG_KEY_FILENAME,
    KEYGEN_FLAG_META_FILENAME,
    KEYGEN_FLAG_KEY_TYPE,
    KEYGEN_FLAG_KEY_LEN,
    KEYGEN_FLAG_KEY_FORMAT,
    KEYGEN_FLAG_TOKEN,
    KEYGEN_FLAG_CACERT_PATH
};

static const std::map<std::string, KeygenFlag> KeygenFlagsMap = {
    {"--key-filename", KEYGEN_FLAG_KEY_FILENAME},
    {"--metadata-filename", KEYGEN_FLAG_META_FILENAME},
    {"--token", KEYGEN_FLAG_TOKEN},
    {"--key-type", KEYGEN_FLAG_KEY_TYPE},
    {"--key-len", KEYGEN_FLAG_KEY_LEN},
    {"--key-format", KEYGEN_FLAG_KEY_FORMAT},
    {"--ca-cert", KEYGEN_FLAG_CACERT_PATH},
    {"--log-level-disable", KEYGEN_FLAG_LOG_LEVEL_DISABLE},
    {"--log-level-trace", KEYGEN_FLAG_LOG_LEVEL_TRACE},
    {"--log-level-debug", KEYGEN_FLAG_LOG_LEVEL_DEBUG},
    {"--log-level-info", KEYGEN_FLAG_LOG_LEVEL_INFO},
    {"--log-level-warning", KEYGEN_FLAG_LOG_LEVEL_WARNING},
    {"--log-level-error", KEYGEN_FLAG_LOG_LEVEL_ERROR}
};

struct KeygenArgs {
    std::string key_filename;
    std::string metadata_filename;
    std::string token;
    std::string key_type;
    size_t key_len;
    std::string key_format;
    ::QryptSecurity::LogLevel log_level;
    std::string cacert_path;
};
KeygenArgs parseKeygenArgs(char** unparsed_args);

static const char* EncryptUsage = 
    "Usage: qrypt encrypt --input-filename=<file> --key-filename=<file> --output-filename=<file> [Optional Args]\n"
    "\n"
    "Encrypt data using an AES-256 key or one-time-pad.\n"
    "\n"
    "Required Arguments:\n"
    "  --input-filename=<filename>     Plaintext input file.\n"
    "  --output-filename=<filename>    Encrypted output file.\n"
    "  --key-filename=<filename>       Key input file.\n"
    "Optional Arguments:\n"
    "  --help                          Display this message.\n"
    "  --key-type=<aes|otp>            Key type used for the encryption operation; AES-256 or one-time-pad. Default otp.\n"
    "  --aes-mode=<ecb|ocb>            (Ignored with --key-type=otp) Set the AES encryption mode. Default ocb.\n"
    "                                  ecb - Legacy ECB algorithm with known vulnerabilities. Useful for demo purposes.\n"
    "                                  ocb - Standard OCB algorithm.\n"
    "  --file-type=<binary|bitmap>     If \"bitmap\", preserve .bmp header for visual demonstration. Default \"binary\".\n"
    "\n";

static const char* DecryptUsage = 
    "Usage: qrypt decrypt --input-filename=<file> --key-filename=<file> --output-filename=<file> [Optional Args]\n"
    "\n"
    "Decrypt data using an AES-256 key or one-time-pad.\n"
    "\n"
    "Required Arguments:\n"
    "  --input-filename=<filename>     Encrypted input file.\n"
    "  --output-filename=<filename>    Decrypted output file.\n"
    "  --key-filename=<filename>       Key input file.\n"
    "Optional Arguments:\n"
    "  --help                          Display this message.\n"
    "  --key-type=<aes|otp>            Key type used for the encryption operation; AES-256 or one-time-pad. Default otp.\n"
    "  --aes-mode=<ecb|ocb>            (Ignored with --key-type=otp) Set the AES encryption mode. Default ocb.\n"
    "                                  ecb - Legacy ECB algorithm with known vulnerabilities. Useful for demo purposes.\n"
    "                                  ocb - Standard OCB algorithm.\n"
    "  --file-type=<binary|bitmap>     If \"bitmap\", preserve .bmp header for visual demonstration. Default \"binary\".\n"
    "\n";

enum EncryptDecryptFlag {
    CRYPT_FLAG_INPUT_FILENAME,
    CRYPT_FLAG_OUTPUT_FILENAME,
    CRYPT_FLAG_KEY_FILENAME,
    CRYPT_FLAG_KEY_TYPE,
    CRYPT_FLAG_AES_MODE,
    CRYPT_FLAG_FILE_TYPE
};

static const std::map<std::string, EncryptDecryptFlag> EncryptDecryptFlagsMap = {
    {"--input-filename", CRYPT_FLAG_INPUT_FILENAME},
    {"--output-filename", CRYPT_FLAG_OUTPUT_FILENAME},
    {"--key-filename", CRYPT_FLAG_KEY_FILENAME},
    {"--key-type", CRYPT_FLAG_KEY_TYPE},
    {"--aes-mode", CRYPT_FLAG_AES_MODE},
    {"--file-type", CRYPT_FLAG_FILE_TYPE}
};

struct EncryptDecryptArgs {
    std::string input_filename;
    std::string output_filename;
    std::string key_filename;
    std::string key_type;
    std::string aes_mode;
    std::string file_type;
};
EncryptDecryptArgs parseEncryptDecryptArgs(char** unparsed_args);

#ifdef ENABLE_TESTS
static const char* TestUsage = 
    "Usage: qrypt test [Optional Arguments]\n"
    "\n"
    "Run a suite of tests for validating and demonstrating the capabilities of the QryptSecurity SDK and Qrypt EaaS."
    "\n"
    "Optional Arguments:\n"
    "  --help                          Display this message.\n"
    "  --token=<token>                 API token for portal.qrypt.com. Defaults to a demo token.\n"
    "\n";
#endif

#endif /* QRYPTCLI_H */
