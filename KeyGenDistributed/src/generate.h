#ifndef GENERATE_H
#define GENERATE_H

#include "QryptSecurity/qryptsecurity.h"
#include "QryptSecurity/qryptsecurity_exceptions.h"
#include "QryptSecurity/qryptsecurity_logging.h"

#include <map>
#include <string>
#include <stdexcept>
#include <vector>

const std::string demo_token = "abcd";

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
    GEN_FLAG_KEY_FORMAT,
    GEN_FLAG_TOKEN,
    GEN_FLAG_CACERT_PATH
};

static const std::map<std::string, GenerateFlag> GenerateFlagsMap = {
    {"--role", GEN_FLAG_ROLE},
    {"--key-filename", GEN_FLAG_KEY_FILENAME},
    {"--metadata-filename", GEN_FLAG_META_FILENAME},
    {"--token", GEN_FLAG_TOKEN},
    {"--key-type", GEN_FLAG_KEY_TYPE},
    {"--key-len", GEN_FLAG_KEY_LEN},
    {"--key-format", GEN_FLAG_KEY_FORMAT},
    {"--ca-cert", GEN_FLAG_CACERT_PATH},
    {"--log-level-disable", GEN_FLAG_LOG_LEVEL_DISABLE},
    {"--log-level-trace", GEN_FLAG_LOG_LEVEL_TRACE},
    {"--log-level-debug", GEN_FLAG_LOG_LEVEL_DEBUG},
    {"--log-level-info", GEN_FLAG_LOG_LEVEL_INFO},
    {"--log-level-warning", GEN_FLAG_LOG_LEVEL_WARNING},
    {"--log-level-error", GEN_FLAG_LOG_LEVEL_ERROR}
};

static const char* GenerateUsage = 
    "Usage: KeyGen generate ARGS\n"
    "\n"
    "Init/sync an AES-256 key or one-time-pad using BLAST distributed key generation.\n"
    "\n"
    "ARGS:\n"
    "    --help                          Display this message.\n"
    "    --user=<alice|bob>              Initialize key as alice (sender) or synchronize as bob (reciever).\n"
    "    --key-filename=<filename>       Path of the key output file.\n"
    "    --metadata-filename=<filename>  Path of the metadata output (alice) or input (bob) file.\n"
    "    --token=<token>                 (Optional) API token for portal.qrypt.com. Defaults to a demo token.\n"
    "    --key-type=<aes|otp>            (Optional) Type of key to generate; AES-256 or one-time-pad. Default \"otp\".\n"
    "    --key-len=<byte_length>         (Optional) Length of key to generate, if otp. Ignored for aes. Default 32.\n"
    "    --key-format=<hexstr|binary>    (Optional) Key out format. Default \"hexstr\".\n"
    "                                    hexstr - key file will be in human-readable hex format.\n"
    "                                    binary - key file will be in binary format.\n"
    "    --log-level-<level>             (Optional) Set logging level for the Qrypt SDK. Default \"disable\".\n"
    "    --ca-cert=<path>                (Optional) Full or relative path to a public root ca-certificate (such as the one \n"
    "                                    at https://curl.se/docs/caextract.html) for TLS traffic with the Qrypt servers.\n"
    "                                    Use this option if the system does not have accessible root certificates or\n"
    "                                    if key generation persistently returns curl error 60 (ssl certificate problem).\n";

struct GenerateArgs {
    std::string role;
    std::string key_filename;
    std::string metadata_filename;
    std::string token;
    std::string key_type;
    size_t key_len;
    std::string key_format;
    ::QryptSecurity::LogLevel log_level;
    std::string cacert_path;
};
using KeyValuePair = std::tuple<std::string, std::string>;
GenerateArgs parseGenerateArgs(std::vector<KeyValuePair> unparsed_args);

void writeKeyToFile(std::string key_filename, std::vector<uint8_t> key, std::string key_format);

#endif /* GENERATE_H */