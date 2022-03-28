
#include "qrypt/qryptsecurity.h"
#include "qrypt/qryptsecurity_exceptions.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

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
    "   % KeyGenDistributed --user=alice --token=<EaaS token> --key-type=<aes|otp> [--otp-len=<desired OTP length>] --metadata-filename=<metadata filename>\n"
    "\n"
    "2. Run as bob, which will read in the metadata file to generate the shared key.\n"
    "\n"
    "   % KeyGenDistributed --user=bob --metadata-filename=<metadata filename>\n"
    "\n"
    "Options:\n"
    "--user=<alice|bob>             Set the user to either alice or bob.\n"
    "\n"
    "--token=<EaaS token>           A Qrypt EaaS token. Retrieve from the Qrypt Portal (http://portal.qrypt.com).\n"
    "                               Tip: Assign token to an environment variable and pass in the environment variable.\n"
    "\n"
    "--key-type=<aes|otp>           Set to the type of key you would like to produce.\n"
    "                               aes - A 32 byte AES key.\n"
    "                               otp - A OTP pad. Use the otp_len parameter to set the length of the OTP.\n"
    "\n"
    "--otp_len=<length>             If otp is specified for --key-type, then this will be the desried length of the OTP.\n"
    "\n"
    "--metadata-filename=<filename> The filename for the metadata file to be created or consumed.\n"
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
    
    std::string user, token, keyType, metadataFilename;
    int otpLen = 0;
    std::string setUserFlag = "--user=";
    std::string setTokenFlag = "--token=";
    std::string setKeyTypeFlag = "--key-type=";
    std::string setOTPLenFlag = "--otp-len=";
    std::string setMetadataFilenameFlag = "--metadata-filename=";

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
    if (keyType == "otp" && otpLen == 0) {
        printf("Invalid OTP length.\n");
        displayUsage();
        return 1;        
    }

    // Create and initialize our keygen client
    auto keyGenClient = IKeyGenDistributedClient::create();
    keyGenClient->initialize(token);
    
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
        printf("Alice - Key: %s", key.c_str());

        // 3. Write out metadata for bob
        std::ofstream output(metadataFilename, std::ios::out | std::ios::binary);
        output.write((char*)&keyInit.metadata[0], keyInit.metadata.size());
        output.close();
    }
    // Bob is the receiver
    else if (user == "bob") {
        // 2. Read in metadata
        std::ifstream input(metadataFilename, std::ios::binary);
        std::vector<unsigned char> metadata(std::istreambuf_iterator<char>(input), {});
        input.close();

        // 3. Generate the key using the metadata
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

