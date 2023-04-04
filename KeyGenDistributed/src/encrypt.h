#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

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

static const char* EncryptUsage = 
    "Usage: KeyGen encrypt ARGS\n"
    "\n"
    "Encrypt data using an AES-256 key or one-time-pad.\n"
    "\n"
    "ARGS:\n"
    "    --help                          Display this message.\n"
    "    --key-type=<aes|otp>            Key type used for the encryption operation; AES-256 or one-time-pad. Default otp.\n"
    "    --aes-mode=<ecb|ocb>            (Ignored with --key-type=otp) Set the AES encryption mode. Default ocb.\n"
    "                                    ecb - Legacy ECB algorithm with known vulnerabilities. Useful for demo purposes.\n"
    "                                    ocb - Standard OCB algorithm.\n"
    "    --input-filename=<filename>     Plaintext input file.\n"
    "    --output-filename=<filename>    Encrypted output file.\n"
    "    --key-filename=<filename>       Key input file.\n"
    "    --file-type=<binary|bitmap>     (Optional) If \"bitmap\", preserve .bmp header for visual demonstration. Default binary.\n";

static const char* DecryptUsage = 
    "Usage: KeyGen decrypt ARGS\n"
    "\n"
    "Decrypt data using an AES-256 key or one-time-pad.\n"
    "\n"
    "ARGS:\n"
    "    --help                          Display this message.\n"
    "    --key-type=<aes|otp>            Key type used for the decryption operation; AES-256 or one-time-pad. Default otp.\n"
    "    --aes-mode=<ecb|ocb>            (Ignored with --key-type=otp) Set the AES decryption mode. Default ocb.\n"
    "                                    ecb - Legacy ECB algorithm with known vulnerabilities. Useful for demo purposes.\n"
    "                                    ocb - Standard OCB algorithm.\n"
    "    --input-filename=<filename>     Encrypted input file.\n"
    "    --output-filename=<filename>    Decrypted output file.\n"
    "    --key-filename=<filename>       Key input file.\n"
    "    --file-type=<binary|bitmap>     (Optional) If \"bitmap\", preserve .bmp header for visual demonstration. Default binary.\n";

struct EncryptDecryptArgs {
    std::string input_filename;
    std::string output_filename;
    std::string key_filename;
    std::string key_type;
    std::string aes_mode;
    std::string file_type;
};
using KeyValuePair = std::tuple<std::string, std::string>;
EncryptDecryptArgs parseEncryptDecryptArgs(std::vector<KeyValuePair> unparsed_args);

std::vector<uint8_t> encryptAES256ECB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data);
std::vector<uint8_t> decryptAES256ECB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data);
std::vector<uint8_t> encryptAES256OCB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data);
std::vector<uint8_t> decryptAES256OCB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data);

#endif /* ENCRYPT_H */