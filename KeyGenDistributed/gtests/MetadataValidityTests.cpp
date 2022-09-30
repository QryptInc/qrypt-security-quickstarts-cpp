#include "QryptSecurity/qryptsecurity.h"
#include "QryptSecurity/qryptsecurity_exceptions.h"
#include "QryptSecurity/qryptsecurity_logging.h"

#include <gtest/gtest.h>
#include <thread>

// The current max pool validity period. A fresh pool will be shredded no later than this amount of time.
static constexpr int MAX_TIME_TO_DECRYPT_IN_MIN = 60;
static constexpr int ERROR_TOLERANCE_IN_MIN = 2;
static constexpr int SECONDS_IN_ONE_MIN = 60;

using namespace QryptSecurity;

class MetadataValidityTest : public ::testing::Test {
  protected:
    std::unique_ptr<IKeyGenDistributedClient> _AliceClient = nullptr;
    std::unique_ptr<IKeyGenDistributedClient> _BobClient = nullptr;
    std::string _Token;

    void SetUp() override {
        _AliceClient = IKeyGenDistributedClient::create();
        _BobClient = IKeyGenDistributedClient::create();
        _Token = std::string(std::getenv("QRYPT_TOKEN"));
    }

    void TearDown() override {
        _AliceClient = nullptr;
        _BobClient = nullptr;
    }

    void initialize() {
        _AliceClient->initialize(_Token);
        _BobClient->initialize(_Token);
    }
};

TEST_F(MetadataValidityTest, MetadataExpired) {
    initialize();
    SymmetricKeyData aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256);
    std::vector<uint8_t> bobKey = _BobClient->genSync(aliceKey.metadata);
    EXPECT_EQ(aliceKey.key, bobKey);

    for (int min = MAX_TIME_TO_DECRYPT_IN_MIN + ERROR_TOLERANCE_IN_MIN; min > 0; --min) {
        printf("Counting down: %d minutes\n", min);

        for (int sec = SECONDS_IN_ONE_MIN; sec > 0; --sec) {
            std::cout << ".";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        try {
            std::vector<uint8_t> bobKey = _BobClient->genSync(aliceKey.metadata);
            EXPECT_EQ(aliceKey.key, bobKey);
            printf("\nRecovered Bob's key from the metadata.\n");
        } catch (QryptSecurityException &e) {            
            printf("\nQryptSecurity exception caught: %s\n", e.what());
            break;
        } catch (std::exception &e) {
            printf("\nstd exception caught: %s\n", e.what());
            break;
        } catch (...) {
            printf("\nOther exception caught\n");
            break;
        }        
    }

    try {
        std::vector<uint8_t> bobKey = _BobClient->genSync(aliceKey.metadata);
        EXPECT_NE(aliceKey.key, bobKey);
    } catch (...) {
    }

    printf("\nVerified that the metadata is no longer valid\n");
}
