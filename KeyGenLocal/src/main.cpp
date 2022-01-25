
#include <chrono>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>

#include "qrypt/qryptsecurity.h"
#include "qrypt/qryptsecurity_exceptions.h"

using namespace QryptSecurity;

const uint64_t KB = 1024;
const uint64_t MB = 1024 * KB;
const uint64_t GB = 1024 * MB;

std::vector<uint8_t> strToVector(std::string in) {
    std::vector<uint8_t> vec((uint8_t *)in.c_str(), (uint8_t *)in.c_str() + in.size());
    return vec;
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

void waitForCacheReady(IKeyGenLocalClient* pKeyGenClient) {
    CacheStatus status = {};
    do {
        status = pKeyGenClient->checkCacheStatus();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while(status.state != CacheState::CACHE_STATE_READY);        
}

int main(int argc, char **argv) {

    if(argc != 2) {
        printf("Incorrect usage. Provide token.");
    }
    std::string qryptToken = argv[1];

    // 1. Setup configurations
    LocationConfig locationConfig = {};
    locationConfig.id = "c67cf9e5-b88e-49ff-8b94-fd29914eb8ff";
    locationConfig.availableSize = 32 * MB;
    locationConfig.path = ".";
    std::vector<LocationConfig> locations;
    locations.push_back(locationConfig);

    CacheConfig cacheConfig = {};
    cacheConfig.deviceSecret = strToVector("Password123");
    cacheConfig.locations = locations;
    cacheConfig.maintenanceInterval = 1;
    cacheConfig.maxNumCachedBytes = 1 * MB;
    cacheConfig.minNumCachedBytes = 32 * KB;
    cacheConfig.rpsFQDN = "api-eus.qrypt.com";  

    // 2. Initialize key generation client
    std::unique_ptr<IKeyGenLocalClient> keyGenClient = IKeyGenLocalClient::create();
    keyGenClient->initializeAsync(qryptToken, cacheConfig);

    // 3. Wait for random to download
    waitForCacheReady(keyGenClient.get());

    // 4. Generate symmetric key
    std::vector<uint8_t> aesKey;
    try {
        aesKey = keyGenClient->genSymmetricKey(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256);
    }
    catch(QryptSecurityException &e) {
        printf("%s\n", e.what());
    }

    printf("AES Key: %s\n", convertByteVecToHexStr(aesKey).c_str());
}
