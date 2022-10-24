#include "QryptSecurity/qryptsecurity.h"
#include "QryptSecurity/qryptsecurity_exceptions.h"
#include "QryptSecurity/qryptsecurity_logging.h"
#include "common.h"

#include <fstream>

using namespace QryptSecurity;

std::string getUsage() {
    std::string usage = 
    "\n"
    "KeyGenDistributed\n"
    "=====================\n"
    "\n"
    "Exercises distributed key generation.\n"
    "\n"
    "Options\n"
    "-------\n"
    "--user=<alice|bob>             Set the user to either alice or bob.\n"
    "\n"
    "--token=<token>                Qrypt token retrieved from Qrypt portal (https://portal.qrypt.com).\n"
    "                               Make sure the token has the BLAST scope.\n"
    "\n"
    "--key-type=<aes|otp>           Set to the type of key you would like to produce.\n"
    "                               aes - AES-256 key with length 32 bytes.\n"
    "                               otp - One time pad with length provided by the otp-len parameter.\n"
    "\n"
    "--otp-len=<length>             The length of the one time pad (in bytes) if key type is otp.\n"
    "\n"
    "--metadata-filename=<filename> The filename for the metadata file to be created or consumed.\n"
    "\n"
    "--key-filename=<filename>      The filename to save the generated key.\n"
    "\n"
    "--ca-cert=<path>               Full or relative path to a public root ca-certificate (such as the one available\n"
    "                               at https://curl.se/docs/caextract.html) for TLS traffic with the Qrypt servers.\n"
    "                               Use this option if the system does not have accessible root certificates or\n"
    "                               if key generation persistently returns curl error 60 (ssl certificate problem).\n"
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
    std::string user, token, metadataFilename, keyFilename, cacertPath;
    std::string keyType = "aes";
    int otpLen = 0;
    std::string setUserFlag = "--user=";
    std::string setTokenFlag = "--token=";
    std::string setKeyTypeFlag = "--key-type=";
    std::string setOTPLenFlag = "--otp-len=";
    std::string setMetadataFilenameFlag = "--metadata-filename=";
    std::string setKeyFilenameFlag = "--key-filename=";
    std::string setCaCertFlag = "--ca-cert=";

    // Parse command line parameters
    while(*++argv) {
        std::string argument(*argv);

        if (argument.find(setUserFlag) == 0) {            
            user = argument.substr(setUserFlag.size());
        }
        else if (argument.find(setTokenFlag) == 0) {
            token = argument.substr(setTokenFlag.size());
        }
        else if (argument.find(setKeyTypeFlag) == 0) {
            keyType = argument.substr(setKeyTypeFlag.size());
        }
        else if (argument.find(setOTPLenFlag) == 0) {
            otpLen = std::stoi(argument.substr(setOTPLenFlag.size()));
        }
        else if (argument.find(setMetadataFilenameFlag) == 0) {
            metadataFilename = argument.substr(setMetadataFilenameFlag.size());
        }
        else if (argument.find(setKeyFilenameFlag) == 0) {
            keyFilename = argument.substr(setKeyFilenameFlag.size());
        }
        else if (argument.find(setCaCertFlag) == 0) {
            cacertPath = argument.substr(setCaCertFlag.size());
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
    if (token.empty()) {
        printf("Missing token.\n");
        displayUsage();
        return 1;
    }
    if (metadataFilename.empty()) {
        printf("Missing metadata filename.\n");
        displayUsage();
        return 1;
    }
    if (keyFilename.empty()) {
        printf("Missing key filename.\n");
        displayUsage();
        return 1;
    }
    if (keyType == "otp" && otpLen == 0) {
        printf("Invalid OTP length.\n");
        displayUsage();
        return 1;
    }

    // Enable QryptSecurity logging
    logging::getLogWriter()->setLogLevel(logging::LogLevel::QRYPTLIB_LOG_LEVEL_INFO);

    try {
        // 1. Create and initialize our keygen client
        auto keyGenClient = IKeyGenDistributedClient::create();
        if (cacertPath.empty()) {
            keyGenClient->initialize(token);
        }
        else {
            keyGenClient->initialize(token, cacertPath);
        }
        
        // Alice is the sender
        if (user == "alice") {
            // 2. Generate the key and metadata
            SymmetricKeyData keyInit = {};
            if (keyType == "aes") {
                keyInit = keyGenClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256);
            }
            else if (keyType == "otp") {
                keyInit = keyGenClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, otpLen);
            }

            // Display the shared key
            std::string key = convertByteVecToHexStr(keyInit.key);
            printf("\nAlice - Key: %s\n\n", key.c_str());

            // 3. Write out metadata for bob and key for encryption
            writeToFile(metadataFilename, keyInit.metadata);
            writeToFile(keyFilename, keyInit.key);
        }
        // Bob is the receiver
        else if (user == "bob") {
            // 2. Read in metadata from alice
            std::vector<uint8_t> metadata = readFromFile(metadataFilename);

            // 3. Generate the key using the metadata
            std::vector<uint8_t> keySync = keyGenClient->genSync(metadata);

            // Display our shared key
            std::string key = convertByteVecToHexStr(keySync);
            printf("\nBob - Key: %s\n\n", key.c_str());

            // 4. Write out key for decryption
            writeToFile(keyFilename, keySync);
        }
        else {
            displayUsage();
            return 1;
        }
    } catch(QryptSecurityException &e) {
        printf("Error: %s\n", e.what());
    }
}

