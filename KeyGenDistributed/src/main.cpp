#include "QryptSecurity/qryptsecurity.h"
#include "QryptSecurity/qryptsecurity_exceptions.h"
#include "QryptSecurity/qryptsecurity_logging.h"
#include "common.h"

#include <fstream>
#include <cstring>

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
    "--user=<alice|bob>              Set the user to either alice or bob.\n"
    "\n"
    "--token=<token>                 Qrypt token retrieved from Qrypt portal (https://portal.qrypt.com).\n"
    "                                Make sure the token has the BLAST scope.\n"
    "\n"
    "--key-type=<aes|otp>            Set to the type of key you would like to produce.\n"
    "                                aes - AES-256 key with length 32 bytes.\n"
    "                                otp - One time pad with length provided by the otp-len parameter.\n"
    "\n"
    "--otp-len=<length>              The length of the one time pad (in bytes) if key type is otp.\n"
    "\n"
    "--metadata-filename=<filename>  The filename for the metadata file to be created or consumed.\n"
    "\n"
    "--key-filename=<filename>       The filename to save the generated key.\n"
    "\n"
    "--random-format=<hexstr|vector> Set the output format of the key you would like to produce.\n"
    "                                hexstr - key will be output in hex format.\n"
    "                                vector - key will be ouput in binary format.\n"
    "                                Defaults to hexstr format.\n"
    "\n"
    "--ca-cert=<path>                Full or relative path to a public root ca-certificate (such as the one available\n"
    "                                at https://curl.se/docs/caextract.html) for TLS traffic with the Qrypt servers.\n"
    "                                Use this option if the system does not have accessible root certificates or\n"
    "                                if key generation persistently returns curl error 60 (ssl certificate problem).\n"
    "\n"
    "--enable_file_logging           Enable file logging. This will disable console logging.\n"
    "                                Defaults to file logging disabled.\n"
    "\n"
    "--log_level_<level>             Set logging level.\n" 
    "                                Defaults to --log_level_disable\n"
    "\n"
    "--help                          Display help.\n"
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
    std::string randomFormat = "hexstr";
    int otpLen = 0;
    std::string setUserFlag = "--user=";
    std::string setTokenFlag = "--token=";
    std::string setKeyTypeFlag = "--key-type=";
    std::string setOTPLenFlag = "--otp-len=";
    std::string setMetadataFilenameFlag = "--metadata-filename=";
    std::string setKeyFilenameFlag = "--key-filename=";
    std::string setRandomFormatFlag = "--random-format=";
    std::string setCaCertFlag = "--ca-cert=";

    // Set default log level
    ::QryptSecurity::logging::getLogWriter()->setLogLevel(::QryptSecurity::logging::LogLevel::QRYPTLIB_LOG_LEVEL_DISABLE);

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
        else if (argument.find(setRandomFormatFlag) == 0) {
            randomFormat = argument.substr(setRandomFormatFlag.size());
        }
        else if (argument.find(setCaCertFlag) == 0) {
            cacertPath = argument.substr(setCaCertFlag.size());
        }
        else if (!strcmp(*argv, "--enable_file_logging")) {
            ::QryptSecurity::logging::getLogWriter()->enableFileLogging("qryptlib.log");
        }
        else if (!strcmp(*argv, "--log_level_trace")) {
            ::QryptSecurity::logging::getLogWriter()->setLogLevel(::QryptSecurity::logging::LogLevel::QRYPTLIB_LOG_LEVEL_TRACE);
        }
        else if (!strcmp(*argv, "--log_level_debug")) {
            ::QryptSecurity::logging::getLogWriter()->setLogLevel(::QryptSecurity::logging::LogLevel::QRYPTLIB_LOG_LEVEL_DEBUG);
        }
        else if (!strcmp(*argv, "--log_level_info")) {
            ::QryptSecurity::logging::getLogWriter()->setLogLevel(::QryptSecurity::logging::LogLevel::QRYPTLIB_LOG_LEVEL_INFO);
        }
        else if (!strcmp(*argv, "--log_level_warn")) {
            ::QryptSecurity::logging::getLogWriter()->setLogLevel(::QryptSecurity::logging::LogLevel::QRYPTLIB_LOG_LEVEL_WARNING);
        }
        else if (!strcmp(*argv, "--log_level_error")) {
            ::QryptSecurity::logging::getLogWriter()->setLogLevel(::QryptSecurity::logging::LogLevel::QRYPTLIB_LOG_LEVEL_ERROR);
        }
        else if (!strcmp(*argv, "--log_level_disable")) {
            ::QryptSecurity::logging::getLogWriter()->setLogLevel(::QryptSecurity::logging::LogLevel::QRYPTLIB_LOG_LEVEL_DISABLE);
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
    if (randomFormat != "hexstr" && randomFormat != "vector") {
        printf("Invalid random format.\n");
        displayUsage();
        return 1;
    }
    
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

            // 3. Write out metadata for bob and key for encryption
            writeToFile(metadataFilename, keyInit.metadata);
            if (randomFormat == "vector") {
                writeToFile(keyFilename, keyInit.key);
            }
            else {
                std::string key = convertByteVecToHexStr(keyInit.key);
                writeToFile(keyFilename, key);
            }

            // 4. Display success
            printf("\nKey successfully created in %s.\n\n", keyFilename.c_str());

        }
        // Bob is the receiver
        else if (user == "bob") {
            // 2. Read in metadata from alice
            std::vector<uint8_t> metadata = readFromFile(metadataFilename);

            // 3. Generate the key using the metadata
            std::vector<uint8_t> keySync = keyGenClient->genSync(metadata);

            // 4. Write out key for decryption
            if (randomFormat == "vector") {
                writeToFile(keyFilename, keySync);
            }
            else {
                std::string key = convertByteVecToHexStr(keySync);
                writeToFile(keyFilename, key);
            }

            // 5. Display success
            printf("\nKey successfully created in %s.\n\n", keyFilename.c_str());
        }
        else {
            displayUsage();
            return 1;
        }
    } catch(QryptSecurityException &e) {
        printf("Error: %s\n", e.what());
    }
}

