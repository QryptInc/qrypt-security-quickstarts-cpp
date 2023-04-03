#include "common.h"
#include <fstream>
#include <iomanip>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

EncryptDecryptArgs::EncryptDecryptArgs(std::vector<CliArgument> unparsed_args) {
    // Set defaults
    generate = false;
    key_type = "otp";
    aes_mode = "ecb";
    file_type = "binary";

    std::vector<CliArgument> gen_args;
    for(CliArgument arg : unparsed_args) {
        try {
            switch(EncryptDecryptFlagsMap.at(arg.flag_string)){
                case CRYPT_FLAG_GENERATE:
                    if(!arg.value.empty()) throw invalid_arg_exception("Invalid argument: " + std::string(arg));
                    generate = true;
                    break;
                case CRYPT_FLAG_INPUT_FILENAME:
                    input_filename = arg.value;
                    break;
                case CRYPT_FLAG_OUTPUT_FILENAME:
                    output_filename = arg.value;
                    break;
                case CRYPT_FLAG_KEY_FILENAME:
                    key_filename = arg.value;
                case CRYPT_FLAG_META_FILENAME:
                    metadata_filename = arg.value;
                    break;
                case CRYPT_FLAG_KEY_TYPE:
                    key_type = arg.value;
                    break;
                case CRYPT_FLAG_AES_MODE:
                    aes_mode = arg.value;
                    break;
                case CRYPT_FLAG_FILE_TYPE:
                    file_type = arg.value;
                    break;
                default:
                    throw invalid_arg_exception("Invalid argument: " + std::string(arg));
            }
        } catch (std::out_of_range& /*ex*/) {
            // Push_back generate args instead of throwing an error, in case --generate is specified
            if (GenerateFlagsMap.find(arg.flag_string) == GenerateFlagsMap.end()) {
                throw invalid_arg_exception("Invalid argument: " + std::string(arg));
            }
            gen_args.push_back(arg);
        }
    }

    if (!generate && !gen_args.empty()) {
        throw invalid_arg_exception("Argument " + std::string(gen_args.front()) + " is for use with \"--generate\" only!");
    }
    if (input_filename.empty()) {
        throw invalid_arg_exception("Missing input-filename");
    }
    if (output_filename.empty()) {
        throw invalid_arg_exception("Missing output-filename");
    }
    if (key_filename.empty() && !generate) {
        throw invalid_arg_exception("Missing key-filename");
    }
    if (metadata_filename.empty()) {
        throw invalid_arg_exception("Missing metadata-filename");
    }
    if (key_type != "aes" && key_type != "otp") {
        throw invalid_arg_exception("Invalid key-type: \"" + key_type + "\"");
    }
    if (aes_mode != "ecb" && aes_mode != "ocb") {
        throw invalid_arg_exception("Invalid aes-mode: \"" + aes_mode + "\"");
    }
    if (key_type != "aes" && key_type != "otp") {
        throw invalid_arg_exception("Invalid key-type: \"" + key_type + "\"");
    }
    if (file_type != "binary" && file_type != "bitmap") {
        throw invalid_arg_exception("Invalid file-type: \"" + file_type + "\"");
    }

    if (generate) {
        gen_args.push_back(CliArgument("--key-filename", key_filename));
        gen_args.push_back(CliArgument("--metadata-filename", metadata_filename));
        generate_args = GenerateArgs(gen_args);
    }
}

std::vector<uint8_t> encryptAES256ECB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data);
std::vector<uint8_t> decryptAES256ECB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data);
std::vector<uint8_t> encryptAES256OCB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data);
std::vector<uint8_t> decryptAES256OCB(const std::vector<uint8_t> aesKey, const std::vector<uint8_t> &data);

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
    "--op=<encrypt|decrypt>             Set operation to encryption or decryption.\n"
    "\n"
    "--key-type=<aes|otp>               Set the encryption/decryption type.\n"
    "                                   aes - AES-256 key with length 32 bytes.\n"
    "                                   otp - One time pad.\n"
    "\n"
    "--aes-mode=<ecb|ocb>               Set the AES encryption/decryption mode (ignored if OTP is selected).\n"
    "                                   ecb - AES-256 key with length 32 bytes to be used for ECB mode (known to be insecure).\n"
    "                                   ocb - AES-256 key with length 32 bytes to be used for OCB mode.\n"
    "                                   Defaults to ocb mode.\n"
    "\n"
    "--key-filename=<filename>          Key to use for encryption or decryption.\n"
    "\n"
    "--random-format=<hexstr|vector>    Set the input format of the key.\n"
    "                                   hexstr - key will be in hex format.\n"
    "                                   vector - key will be in binary format.\n"
    "                                   Defaults to hexstr format.\n"
    "\n"
    "--file-type=<binary|bitmap>        Set the input/output file type.\n"
    "                                   binary - File data will be used as a big binary blob.\n"
    "                                   bitmap - bmp image file. Bitmap header data will be preserved.\n"
    "\n"
    "--input-filename=<filename>        The input file.\n"
    "\n"
    "--output-filename=<filename>       The output file.\n"
    "\n"
    "--help                             Display help.\n"
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
    std::string aesMode = "ocb";
    std::string randomFormat = "hexstr";
    std::string fileType = "binary";
    std::string setOperationFlag = "--op=";
    std::string setKeyTypeFlag = "--key-type=";
    std::string setAESModeFlag = "--aes-mode=";
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
        else if (argument.find(setAESModeFlag) == 0) {
            aesMode = argument.substr(setAESModeFlag.size());
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
    if (aesMode != "ocb" && aesMode != "ecb") {
        printf("Invalid aes mode.\n");
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

    try {
        // 1. Read key
        std::vector<uint8_t> key;
        if (randomFormat == "vector") {
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
                if (key.size() != AESKeyLengthInBytes) {
                    throw std::runtime_error("Provided AES key invalid. (File does not exist or key is the wrong size.)");
                }
                if (aesMode == "ecb") {
                    cipherTextData = encryptAES256ECB(key, plainTextData);
                }
                else if (aesMode == "ocb") {
                    cipherTextData = encryptAES256OCB(key, plainTextData);
                }
            }
            else if (keyType == "otp") {
                if (key.size() != plainTextData.size()) {
                    throw std::runtime_error("Provided OTP invalid. (File does not exist or key is the wrong size.)");
                }
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
                if (key.size() != AESKeyLengthInBytes) {
                    throw std::runtime_error("Provided AES key invalid. (File does not exist or key is the wrong size.)");
                }
                if (aesMode == "ecb") {
                    plainTextData = decryptAES256ECB(key, cipherTextData);
                }
                else if (aesMode == "ocb") {
                    plainTextData = decryptAES256OCB(key, cipherTextData);
                }
            }
            else if (keyType == "otp") {
                if (key.size() != cipherTextData.size()) {
                    throw std::runtime_error("Provided OTP invalid. (File does not exist or key is the wrong size.)");
                }
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
    } catch (std::runtime_error &ex) {
        printf("\nFailure: %s\n", ex.what());
        return 1;
    }

    printf("\nSuccess!\n");
    return 0;
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
