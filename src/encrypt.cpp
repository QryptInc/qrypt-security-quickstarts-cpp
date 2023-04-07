#include "common.h"
#include "encrypt.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

void encryptDecrypt(std::string operation,
                    std::istream& input_stream, std::istream& key_stream, std::ostream& output_stream,
                    std::string file_type, std::string aes_mode, std::string key_type) {

    if (operation != "encrypt" && operation != "decrypt") {
        throw std::invalid_argument("Invalid operation: \"" + operation + "\"");
    }
    if (key_type != "aes" && key_type != "otp") {
        throw std::invalid_argument("Invalid key type: \"" + key_type + "\"");
    }
    if (aes_mode != "ecb" && aes_mode != "ocb") {
        throw std::invalid_argument("Invalid aes mode: \"" + aes_mode + "\"");
    }
    if (file_type != "binary" && file_type != "bitmap") {
        throw std::invalid_argument("Invalid file type: \"" + file_type + "\"");
    }

    // Read inputs
    std::vector<uint8_t> header;
    std::vector<uint8_t> input(std::istreambuf_iterator<char>(input_stream), {});
    std::vector<uint8_t> key(std::istreambuf_iterator<char>(key_stream), {});
    // Preserve bmp header so it doesn't get decrypted
    if (file_type = "bitmap") {
        header = std::vector<uint8_t>(input.begin(), input.begin() + BMP_HEADER_SIZE);
        input = std::vector<uint8_t>(input.begin() + BMP_HEADER_SIZE, input.end());
    }
    // Convert key to binary if it is hexadecimal
    std::string key_string(key.begin(), key.end());
    if (key_string.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos) {
        key = hexStrToByteVec(key_string);
    }

    // Run cryptography operation
    std::vector<uint8_t> output;
    if (key_type == "aes") {
        if (key.size() != AESKeyLengthInBytes) {
            throw std::runtime_error("Provided AES key invalid. (File does not exist or key is the wrong size.)");
        }
        if (operation == "encrypt") {
            output = (aes_mode == "ecb") ? encryptAES256ECB(key, input) : encryptAES256OCB(key, input);
        }
        else { // operation == "decrypt"
            output = (aes_mode == "ecb") ? decryptAES256ECB(key, input) : decryptAES256OCB(key, input);
        }
    }
    else { // key_type == "otp"
        if (key.size() != ciphertext.size()) {
            throw std::runtime_error("Provided OTP invalid. (File does not exist or key is the wrong size.)");
        }
        output = xorVectors(key, input);
    }

    // Write output
    output.write((char *)&(header)[0], header.size());
    output.write((char *)&(output)[0], output.size());
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
