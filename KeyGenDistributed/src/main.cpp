
#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string.h>
#include <vector>

#include "qrypt/qryptsecurity.h"
#include "qrypt/qryptsecurity_exceptions.h"

using namespace QryptSecurity;

std::string getUsage() {
    std::string usage = 
    "\n"
    "KeyGenDistributed\n"
    "=================\n"
    "\n"
    "Exercises distributed key generation.\n"
    "\n"
    "Usage\n"
    "-----\n"
    "1. Run as alice to generate the shared key and a metadata file to be sent to bob.\n"
    "\n"
    "   % KeyGenDistributed --user=alice --token=<EaaS token> --key-type=<aes|otp> [--otp-len=<desired OTP length>] --filename=<metadata filename>\n"
    "\n"
    "2. Run as bob, which will read in the metadata file to generate the shared key.\n"
    "\n"
    "   % KeyGenDistributed --user=bob --filename=<metadata filename>\n"
    "\n"
    "Options:\n"
    "--user=<alice|bob>      Set the user to either alice or bob.\n"
    "\n"
    "--token=<EaaS token>    A Qrypt EaaS token. Retrieve from the Qrypt Portal (http://portal.qrypt.com).\n"
    "                        Tip: Assign token to an environment variable and pass in the environment variable.\n"
    "\n"
    "--key-type=<aes|otp>    Set to the type of key you would like to produce.\n"
    "                        aes - A 32 byte AES key.\n"
    "                        otp - A OTP pad. Use the otp_len parameter to set the length of the OTP.\n"
    "\n"
    "--otp_len=<length>      If otp is specified for --key-type, then this will be the desried length of the OTP.\n"
    "\n"
    "--filename=<filename>   The filename for the metadata file to be created or consumed.\n"
    "\n"
    "--help                  Display help.\n"
    "\n"
    "";

    return usage;
}

void displayUsage() {
    std::string usage = getUsage();
    printf("%s", usage.c_str());
}

std::string convertByteVecToHexStr(std::vector<uint8_t> bytes) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto byte : bytes) {
        int byteValue = byte;
        oss << std::setw(2) << byteValue;
    }
    std::string result = oss.str();
    return result;
}

int main(int argc, char **argv) {

    // TODO: Change to prod url
    std::string qdeaFQDN = "https://qdea-directory.stage.qrypt.com";
    
    std::string setUserFlag = "--user=";
    std::string user = "";
    std::string setTokenFlag = "--token=";
    std::string token = "";
    std::string setKeyTypeFlag = "--key-type=";
    std::string keyType = "";
    std::string setOTPLenFlag = "--otp-len=";
    int otpLen = 0;
    std::string setFilenameFlag = "--filename=";
    std::string filename = "";

    // Parse command line parameters
    while(*++argv) {
       
        if (std::string(*argv).find(setUserFlag) == 0) {
            std::string argument = std::string(*argv);
            user = argument.substr(setUserFlag.size(), argument.size() - setUserFlag.size());
        }
        else if (std::string(*argv).find(setTokenFlag) == 0) {
            std::string argument = std::string(*argv);
            token = argument.substr(setTokenFlag.size(), argument.size() - setTokenFlag.size());
        }
        else if (std::string(*argv).find(setKeyTypeFlag) == 0) {
            std::string argument = std::string(*argv);
            keyType = argument.substr(setKeyTypeFlag.size(), argument.size() - setKeyTypeFlag.size());
        }
        else if (std::string(*argv).find(setOTPLenFlag) == 0) {
            std::string argument = std::string(*argv);
            otpLen = std::stoi(argument.substr(setOTPLenFlag.size(), argument.size() - setOTPLenFlag.size()));
        }
        else if (std::string(*argv).find(setFilenameFlag) == 0) {
            std::string argument = std::string(*argv);
            filename = argument.substr(setFilenameFlag.size(), argument.size() - setFilenameFlag.size());
        }
        // Display help
        else if (!strcmp(*argv, "-h") || !strcmp(*argv, "--help")) {
            displayUsage();
            return 0;
        }
        // Invalid param
        else {
            displayUsage();
            return 1;
        }
    }

    // Validate arguments
    if (token.empty()) {
        displayUsage();
        return 1;
    }
    if (filename.empty()) {
        displayUsage();
        return 1;        
    }
    if (keyType == "otp" && otpLen == 0) {
        displayUsage();
        return 1;        
    }

    // Create and initialize our keygen client
    KeyAgreementConfig keyAgreementConfig = {};
    keyAgreementConfig.qdeaFQDN = qdeaFQDN;
    auto keyGenClient = IKeyGenDistributedClient::create();
    keyGenClient->initialize(token, keyAgreementConfig);
    
    // Are we generating the shared key and metadata?
    if (user == "alice") {
        // What type of key do we want to create?
        SymmetricKeyData keyInit = {};
        if (keyType == "aes") {
            // Create the aes key and metadata
            auto symmetricKeyMode = QryptSecurity::SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256;
            keyInit = keyGenClient->genInit(symmetricKeyMode);
        }
        else if (keyType == "otp") {
            // Create the OTP and metadata
            auto symmetricKeyMode = QryptSecurity::SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP;
            keyInit = keyGenClient->genInit(symmetricKeyMode, otpLen);
        }

        // Display the shared key
        std::string key = convertByteVecToHexStr(keyInit.key);
        printf("Alice - Key: %s", key.c_str());

        // Write out metadata to be used by bob
        std::ofstream output(filename, std::ios::out | std::ios::binary);
        output.write((char*)&keyInit.metadata[0], keyInit.metadata.size());
        output.close();
    }
    // Are we recreating the shared key using the metadata?
    else if (user == "bob") {
        // Read in the metadata
        std::ifstream input(filename, std::ios::binary);
        std::vector<unsigned char> metadata(std::istreambuf_iterator<char>(input), {});
        input.close();

        // Recreate our shared key using the metadata
        std::vector<uint8_t> keySync = keyGenClient->genSync(metadata);

        // Display our shared key
        std::string key = convertByteVecToHexStr(keySync);
        printf("Bob - Key: %s", key.c_str());
    }
    else {
        displayUsage();
        return 1;
    }
}

