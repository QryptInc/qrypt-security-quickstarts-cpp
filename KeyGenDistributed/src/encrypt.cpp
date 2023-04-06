#include "common.h"
#include "encrypt.h"
#include <fstream>
#include <iomanip>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

using KeyValuePair = std::tuple<std::string, std::string>;

void encrypt(std::string operation, std::vector<KeyValuePair> unparsed_args) {
    auto args = parseEncryptDecryptArgs(unparsed_args);

    BitmapData bitmapData = {};
    std::vector<uint8_t> cipherTextData;
    std::vector<uint8_t> plainTextData;

    // 1. Read key
    std::vector<uint8_t> key = readFromFile(args.key_filename);

    if (operation == "encrypt") {
        // 2. Read input file to encrypt
        if (args.file_type == "bitmap") {
            bitmapData = readBitmap(args.input_filename);
            plainTextData = bitmapData.body;
        }
        else if (args.file_type == "binary") {
            plainTextData = readFromFile(args.input_filename);
        }

        // 3. Encrypt file
        if (args.key_type == "aes") {
            if (key.size() != AESKeyLengthInBytes) {
                throw std::runtime_error("Provided AES key invalid. (File does not exist or key is the wrong size.)");
            }
            if (args.aes_mode == "ecb") {
                cipherTextData = encryptAES256ECB(key, plainTextData);
            }
            else if (args.aes_mode == "ocb") {
                cipherTextData = encryptAES256OCB(key, plainTextData);
            }
        }
        else if (args.key_type == "otp") {
            if (key.size() != plainTextData.size()) {
                throw std::runtime_error("Provided OTP invalid. (File does not exist or key is the wrong size.)");
            }
            cipherTextData = xorVectors(key, plainTextData);
        }

        // 4. Write out encrypted file
        if (args.file_type == "bitmap") {
            bitmapData.body = cipherTextData;
            writeBitmap(args.output_filename, bitmapData);
        }
        else if (args.file_type == "binary") {
            writeToFile(args.output_filename, cipherTextData);
        }
    }
    else if (operation == "decrypt") {
        // 2. Read in encrypted file
        if (args.file_type == "bitmap") {
            bitmapData = readBitmap(args.input_filename);
            cipherTextData = bitmapData.body;
        }
        else if (args.file_type == "binary") {
            cipherTextData = readFromFile(args.input_filename);
        }

        // 3. Decrypt file
        std::vector<uint8_t> plainTextData;
        if (args.key_type == "aes") {
            if (key.size() != AESKeyLengthInBytes) {
                throw std::runtime_error("Provided AES key invalid. (File does not exist or key is the wrong size.)");
            }
            if (args.aes_mode == "ecb") {
                plainTextData = decryptAES256ECB(key, cipherTextData);
            }
            else if (args.aes_mode == "ocb") {
                plainTextData = decryptAES256OCB(key, cipherTextData);
            }
        }
        else if (args.key_type == "otp") {
            if (key.size() != cipherTextData.size()) {
                throw std::runtime_error("Provided OTP invalid. (File does not exist or key is the wrong size.)");
            }
            plainTextData = xorVectors(key, cipherTextData);
        }

        // 4. Write out decrypted file
        if (args.file_type == "bitmap") {
            bitmapData.body = plainTextData;
            writeBitmap(args.output_filename, bitmapData);
        }
        else if (args.file_type == "binary") {
            writeToFile(args.output_filename, plainTextData);
        }
    }
}

EncryptDecryptArgs parseEncryptDecryptArgs(std::vector<KeyValuePair> unparsed_args) {
    std::string input_filename, output_filename, key_filename;
    std::string key_type = "otp";
    std::string aes_mode = "ecb";
    std::string file_type = "binary";

    for(auto[arg_name, arg_value] : unparsed_args) {
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
    if (key_filename.empty() && !generate) {
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

std::vector<uint8_t> encryptAES256ECB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data) {
    int resCode = 0;

    // 1. Create context
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)> pEvpCtx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    if (pEvpCtx == nullptr) {
        throw std::runtime_error("EVP_CIPHER_CTX_new() returned NULL!");
    }

    // 2. Set engine EVP_aes_256_ecb: AES-256 Electronic Codebook
    resCode = EVP_EncryptInit_ex(pEvpCtx.get(), EVP_aes_256_ecb(), nullptr, aesKey.data(), nullptr); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_EncryptInit_ex() failed!");
    }

    // Over allocate vector for encrypted data to account for block size
    int blockSizeInBytes = EVP_CIPHER_CTX_block_size(pEvpCtx.get());
    std::vector<uint8_t> encryptedData(data.size() + blockSizeInBytes);

    // 3. Encrypt the plaintext
    int len;
    resCode = EVP_EncryptUpdate(pEvpCtx.get(), encryptedData.data(), &len, data.data(), data.size()); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_EncryptUpdate() failed!");
    }

    // Finalize encrypt
    resCode = EVP_EncryptFinal_ex(pEvpCtx.get(), encryptedData.data() + len, &len); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_EncryptFinal_ex() failed!");
    }

    return encryptedData;
}

std::vector<uint8_t> decryptAES256ECB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data) {
    int resCode = 0;

    // 1. Create context
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)> pEvpCtx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    if (pEvpCtx == nullptr) {
        throw std::runtime_error("EVP_CIPHER_CTX_new() returned NULL!");
    }

    // 2. Set engine EVP_aes_256_ecb: AES-256 Electronic Codebook
    resCode = EVP_DecryptInit_ex(pEvpCtx.get(), EVP_aes_256_ecb(), nullptr, aesKey.data(), nullptr); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_DecryptInit_ex() failed!");
    }

    // Allocate vector for decrypted data
    std::vector<uint8_t> decryptedData(data.size());

    // 3. Decrypt the ciphertext
    int len;
    resCode = EVP_DecryptUpdate(pEvpCtx.get(), decryptedData.data(), &len, data.data(), data.size()); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_DecryptUpdate() failed!");
    }

    // Finalize decrypt
    resCode = EVP_DecryptFinal_ex(pEvpCtx.get(), decryptedData.data() + len, &len); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_DecryptFinal_ex() failed!");
    }

    return decryptedData;
}

std::vector<uint8_t> encryptAES256OCB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data) {
    if (aesKey.size() != AESKeyLengthInBytes) {
        throw std::runtime_error("AES key is of the wrong size.");
    }

    // In practice IV shouldn't be a zero vector, but to keep this demo simple we will set it to zero vector
    std::vector<uint8_t> aesKeyWithIV(IVLengthInBytes, 0);
    aesKeyWithIV.insert(aesKeyWithIV.begin(), aesKey.begin(), aesKey.end());
    int resCode = 0;

    // 1. Create context
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)> pEvpCtx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    if (pEvpCtx == nullptr) {
        throw std::runtime_error("EVP_CIPHER_CTX_new() returned NULL!");
    }

    // 2. Set engine EVP_aes_256_ocb: AES-256 Offset Codebook
    resCode = EVP_EncryptInit_ex(pEvpCtx.get(), EVP_aes_256_ocb(), nullptr, nullptr, nullptr); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_EncryptInit_ex() failed!");
    }

    // Set Tag length
    resCode = EVP_CIPHER_CTX_ctrl(pEvpCtx.get(), EVP_CTRL_AEAD_SET_TAG, AETagSizeInBytes, nullptr); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_CIPHER_CTX_ctrl() EVP_CTRL_AEAD_SET_TAG failed!");
    }

    // Set IV length
    resCode = EVP_CIPHER_CTX_ctrl(pEvpCtx.get(), EVP_CTRL_AEAD_SET_IVLEN, IVLengthInBytes, nullptr); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_CIPHER_CTX_ctrl() EVP_CTRL_AEAD_SET_IVLEN failed!");
    }

    // Set AES key and IV
    resCode = EVP_EncryptInit_ex(pEvpCtx.get(), nullptr, nullptr, aesKeyWithIV.data(),
                                 aesKeyWithIV.data() + AESKeyLengthInBytes); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_EncryptInit_ex() failed!");
    }

    // Over allocate vector for encrypted data to account for block size
    int blockSizeInBytes = EVP_CIPHER_CTX_block_size(pEvpCtx.get());
    std::vector<uint8_t> encryptedData(data.size() + blockSizeInBytes);

    // 3. Encrypt the plaintext
    int len;
    resCode = EVP_EncryptUpdate(pEvpCtx.get(), encryptedData.data(), &len, data.data(), data.size()); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_EncryptUpdate() failed!");
    }
    int ciphertext_len = len;

    // Finalize encrypt
    resCode = EVP_EncryptFinal_ex(pEvpCtx.get(), encryptedData.data() + len, &len); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_EncryptFinal_ex() failed!");
    }
    ciphertext_len += len;

    // 4. Get tag
    std::vector<uint8_t> tag(AETagSizeInBytes);
    resCode = EVP_CIPHER_CTX_ctrl(pEvpCtx.get(), EVP_CTRL_AEAD_GET_TAG, AETagSizeInBytes, tag.data()); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_CIPHER_CTX_ctrl() EVP_CTRL_AEAD_GET_TAG failed!");
    }

    // 5. Resize vector for encrypted data and append tag
    encryptedData.resize(ciphertext_len);
    encryptedData.insert(encryptedData.end(), tag.begin(), tag.end());
    return encryptedData;
}

std::vector<uint8_t> decryptAES256OCB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data) {
    if (aesKey.size() != AESKeyLengthInBytes) {
        throw std::runtime_error("AES key is of the wrong size.");
    }

    std::vector<uint8_t> aesKeyWithIV(IVLengthInBytes, 0);
    aesKeyWithIV.insert(aesKeyWithIV.begin(), aesKey.begin(), aesKey.end());
    int resCode = 0;

    // 1. Create context
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)> pEvpCtx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    if (pEvpCtx == nullptr) {
        throw std::runtime_error("EVP_CIPHER_CTX_new() returned NULL!");
    }

    // 2. Set engine EVP_aes_256_ocb: AES-256 Offset Codebook
    resCode = EVP_DecryptInit_ex(pEvpCtx.get(), EVP_aes_256_ocb(), nullptr, nullptr, nullptr); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_DecryptInit_ex() failed!");
    }

    // Set Tag length
    resCode = EVP_CIPHER_CTX_ctrl(pEvpCtx.get(), EVP_CTRL_AEAD_SET_TAG, AETagSizeInBytes, nullptr); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_CIPHER_CTX_ctrl() EVP_CTRL_AEAD_SET_TAG failed!");
    }

    // Set IV length
    resCode = EVP_CIPHER_CTX_ctrl(pEvpCtx.get(), EVP_CTRL_AEAD_SET_IVLEN, IVLengthInBytes, nullptr); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_CIPHER_CTX_ctrl() EVP_CTRL_AEAD_SET_IVLEN failed!");
    }

    // Set AES key and IV
    resCode = EVP_DecryptInit_ex(pEvpCtx.get(), nullptr, nullptr, aesKeyWithIV.data(),
                                 aesKeyWithIV.data() + AESKeyLengthInBytes); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_DecryptInit_ex() failed!");
    }

    // 3. Set tag
    std::vector<uint8_t> tag(data.end() - AETagSizeInBytes, data.end());
    resCode = EVP_CIPHER_CTX_ctrl(pEvpCtx.get(), EVP_CTRL_AEAD_SET_TAG, AETagSizeInBytes, tag.data()); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_CIPHER_CTX_ctrl() EVP_CTRL_AEAD_SET_TAG failed!");
    }

    // Allocate vector for decrypted data
    int ciphertext_len = data.size() - AETagSizeInBytes;
    std::vector<uint8_t> decryptedData(ciphertext_len);

    // 4. Decrypt the ciphertext
    int len;
    resCode = EVP_DecryptUpdate(pEvpCtx.get(), decryptedData.data(), &len, data.data(), ciphertext_len); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_DecryptUpdate() failed!");
    }
    int plaintext_len = len;

    // Finalize decrypt
    resCode = EVP_DecryptFinal_ex(pEvpCtx.get(), decryptedData.data() + len, &len); // NOLINT
    if (resCode != OPENSSL_SUCCESS) {
        throw std::runtime_error("EVP_DecryptFinal_ex() failed!");
    }
    plaintext_len += len;

    // Resize vector for decrypted data
    decryptedData.resize(plaintext_len);
    return decryptedData;
}
