#include "common.h"
#include <fstream>
#include <iomanip>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

std::vector<uint8_t> encryptAES256(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data);
std::vector<uint8_t> decryptAES256(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data);

std::string getUsage() {
    std::string usage = 
    "\n"
    "EncryptTool\n"
    "=====================\n"
    "\n"
    "Exercises encryption or decryption.\n"
    "\n"
    "Options\n"
    "-------\n"
    "--op=<encrypt|decrypt>         Set operation to encryption or decryption.\n"
    "\n"
    "--key-type=<aes|otp>           Set the encryption/decryption type.\n"
    "                               aes - AES-256 key with length 32 bytes.\n"
    "                               otp - One time pad.\n"
    "\n"
    "--key-filename=<filename>      Key to use for encryption or decryption.\n"
    "\n"
    "--random-format=<hexstr|vector> Set the input format of the key.\n"
    "                                hexstr - key will be in hex format.\n"
    "                                vector - key will be in binary format.\n"
    "                                Defaults to hexstr format.\n"
    "\n"
    "--file-type=<binary|bitmap>    Set the input/output file type.\n"
    "                               binary - File data will be used as a big binary blob.\n"
    "                               bitmap - bmp image file. Bitmap header data will be preserved.\n"
    "\n"
    "--input-filename=<filename>    The input file.\n"
    "\n"
    "--output-filename=<filename>   The output file.\n"
    "\n"
    "--help                         Display help.\n"
    "\n"
    "";

    return usage;
}

void displayUsage() {
    std::string usage = getUsage();
    printf("%s", usage.c_str());
}

int main(int argc, char **argv) {
    
    std::string operation, keyFilename, inputFilename, outputFilename;
    std::string keyType = "aes";
    std::string randomFormat = "hexstr";
    std::string fileType = "binary";
    std::string setOperationFlag = "--op=";
    std::string setKeyTypeFlag = "--key-type=";
    std::string setKeyFilenameFlag = "--key-filename=";
    std::string setRandomFormatFlag = "--random-format=";
    std::string setFileTypeFlag = "--file-type=";
    std::string setInputFilenameFlag = "--input-filename=";
    std::string setOutputFilenameFlag = "--output-filename=";

    // Parse command line parameters
    while(*++argv) {
        std::string argument(*argv);

        if (argument.find(setOperationFlag) == 0) {            
            operation = argument.substr(setOperationFlag.size());
        }
        else if (argument.find(setKeyTypeFlag) == 0) {
            keyType = argument.substr(setKeyTypeFlag.size());
        }
        else if (argument.find(setRandomFormatFlag) == 0) {
            randomFormat = argument.substr(setRandomFormatFlag.size());
        }
        else if (argument.find(setKeyFilenameFlag) == 0) {
            keyFilename = argument.substr(setKeyFilenameFlag.size());
        }
        else if (argument.find(setFileTypeFlag) == 0) {
            fileType = argument.substr(setFileTypeFlag.size());
        }
        else if (argument.find(setInputFilenameFlag) == 0) {
            inputFilename = argument.substr(setInputFilenameFlag.size());
        }
        else if (argument.find(setOutputFilenameFlag) == 0) {
            outputFilename = argument.substr(setOutputFilenameFlag.size());
        }
        else if ((argument == "-h") || (argument == "--help")) {
            displayUsage();
            return 0;
        }
        else {
            printf("Invalid parameter: %s\n", argument.c_str());
            displayUsage();
            return 1;
        }
    }

    // Validate arguments
    if (keyFilename.empty()) {
        printf("Missing key filename.\n");
        displayUsage();
        return 1;        
    }
    if (inputFilename.empty()) {
        printf("Missing input filename.\n");
        displayUsage();
        return 1;        
    }
    if (outputFilename.empty()) {
        printf("Missing output filename.\n");
        displayUsage();
        return 1;        
    }
    if (keyType != "aes" && keyType != "otp") {
        printf("Invalid key type.\n");
        displayUsage();
        return 1;
    }
    if (fileType != "binary" && fileType != "bitmap") {
        printf("Invalid file type.\n");
        displayUsage();
        return 1;
    }
    if (randomFormat != "hexstr" && randomFormat != "vector") {
        printf("Invalid random format.\n");
        displayUsage();
        return 1;
    }
    
    printf("\nCalling up EncryptTool to %s %s in %s format using the %s key file %s and generate %s.\n",
            operation.c_str(), inputFilename.c_str(), fileType.c_str(), toUpper(keyType).c_str(), keyFilename.c_str(), outputFilename.c_str());

    BitmapData bitmapData = {};
    std::vector<uint8_t> cipherTextData;
    std::vector<uint8_t> plainTextData;

    // 1. Read key
    std::vector<uint8_t> key;
    if (keyFormat == "vector") {
        key = readFromFile(keyFilename);
    }
    else {
        key = readFromHexFile(keyFilename);
    }

    if (operation == "encrypt") {
        // 2. Read input file to encrypt
        if (fileType == "bitmap") {
            bitmapData = readBitmap(inputFilename);
            plainTextData = bitmapData.body;
        }
        else if (fileType == "binary") {
            plainTextData = readFromFile(inputFilename);
        }

        // 3. Encrypt file
        if (keyType == "aes") {
            cipherTextData = encryptAES256(key, plainTextData);
        }
        else if (keyType == "otp") {
            cipherTextData = xorVectors(key, plainTextData);
        }

        // 4. Write out encrypted file
        if (fileType == "bitmap") {
            bitmapData.body = cipherTextData;
            writeBitmap(outputFilename, bitmapData);            
        }
        else if (fileType == "binary") {
            writeToFile(outputFilename, cipherTextData);
        }
    }
    else if (operation == "decrypt") {
        // 2. Read in encrypted file      
        if (fileType == "bitmap") {
            bitmapData = readBitmap(inputFilename);
            cipherTextData = bitmapData.body;
        }
        else if (fileType == "binary") {
            cipherTextData = readFromFile(inputFilename);
        }

        // 3. Decrypt file
        std::vector<uint8_t> plainTextData;
        if (keyType == "aes") {        
            plainTextData = decryptAES256(key, cipherTextData);
        }
        else if (keyType == "otp") {
            plainTextData = xorVectors(key, cipherTextData);
        }

        // 4. Write out decrypted file
        if (fileType == "bitmap") {
            bitmapData.body = plainTextData;
            writeBitmap(outputFilename, bitmapData);
        }
        else if (fileType == "binary") {
            writeToFile(outputFilename, plainTextData);
        }
    }
    else {
        displayUsage();
        return 1;
    }
}

std::vector<uint8_t> encryptAES256(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data) {
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

std::vector<uint8_t> decryptAES256(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data) {
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
