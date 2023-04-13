#include "QryptSecurity/qryptsecurity.h"
#include "QryptSecurity/qryptsecurity_exceptions.h"
#include "QryptSecurity/qryptsecurity_logging.h"

#include <iostream>
#include <thread>
#include <gtest/gtest.h>

using namespace QryptSecurity;

static const uint64_t KB = 1024;
static const uint64_t MB = 1024 * 1024;

static const char* green_pass = "\x1B[32mPASS\x1B[0m\n";
static const char* red_fail = "\x1B[31mFAIL\x1B[0m\n";

class KeyGenTest : public ::testing::Test {
  protected:
    std::unique_ptr<IKeyGenDistributedClient> _AliceClient = nullptr;
    std::unique_ptr<IKeyGenDistributedClient> _BobClient = nullptr;
    std::string _Token;

    void SetUp() override {
        _Token = std::string(std::getenv("QRYPT_TOKEN"));
        _AliceClient = IKeyGenDistributedClient::create();
        _AliceClient->initialize(_Token);
        _BobClient = IKeyGenDistributedClient::create();
        _BobClient->initialize(_Token);
    }

    void TearDown() override {
        _AliceClient = nullptr;
        _BobClient = nullptr;
    }
};

TEST_F(KeyGenTest, AES256) {
    const std::string gen_msg = "\tGenerating an AES256 key....";
    std::cout << gen_msg << std::string(gen_msg.length() + 7, '\b') << std::flush;
    SymmetricKeyData aliceKey;
    ASSERT_NO_THROW(
        aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256, 32)
    ) << gen_msg << red_fail;
    ASSERT_EQ(aliceKey.key.size(), 32) << gen_msg << red_fail;
    std::cout << gen_msg << green_pass;

    const std::string sync_msg = "\tReplicating an AES256 key...";
    std::cout << sync_msg << std::string(sync_msg.length() + 7, '\b') << std::flush;
    std::vector<uint8_t> bobKey;
    ASSERT_NO_THROW(
        bobKey = _BobClient->genSync(aliceKey.metadata)
    ) << sync_msg << red_fail;
    ASSERT_EQ(bobKey.size(), 32) << gen_msg << red_fail;
    std::cout << sync_msg << green_pass;

    const std::string cerify_msg = "\tVerifying keys match........";
    std::cout << cerify_msg << std::string(cerify_msg.length() + 7, '\b') << std::flush;
    ASSERT_EQ(
        std::string(aliceKey.key.begin(), aliceKey.key.end()), std::string(bobKey.begin(), bobKey.end())
    ) << cerify_msg << red_fail;
    std::cout << cerify_msg << green_pass;
}

TEST_F(KeyGenTest, OTP32) {
    const std::string gen_msg = "\tGenerating a 32B one-time-pad....";
    std::cout << gen_msg << std::string(gen_msg.length(), '\b');
    SymmetricKeyData aliceKey;
    ASSERT_NO_THROW(
        aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, 32)
    ) << gen_msg << red_fail;
    ASSERT_EQ(aliceKey.key.size(), 32) << gen_msg << red_fail;
    std::cout << gen_msg << green_pass;

    const std::string sync_msg = "\tReplicating a 32B one-time-pad...";
    std::cout << sync_msg << std::string(sync_msg.length(), '\b');
    std::vector<uint8_t> bobKey;
    ASSERT_NO_THROW(
        bobKey = _BobClient->genSync(aliceKey.metadata)
    ) << sync_msg << red_fail;
    ASSERT_EQ(bobKey.size(), 32) << gen_msg << red_fail;
    std::cout << sync_msg << green_pass;

    const std::string cerify_msg = "\tVerifying keys match.............";
    std::cout << cerify_msg << std::string(cerify_msg.length(), '\b');
    ASSERT_EQ(
        std::string(aliceKey.key.begin(), aliceKey.key.end()), std::string(bobKey.begin(), bobKey.end())
    ) << cerify_msg << red_fail;
    std::cout << cerify_msg << green_pass;
}

TEST_F(KeyGenTest, OTP1024) {
    const std::string gen_msg = "\tGenerating a 1KB one-time-pad....";
    std::cout << gen_msg << std::string(gen_msg.length(), '\b');
    SymmetricKeyData aliceKey;
    ASSERT_NO_THROW(
        aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, KB)
    ) << gen_msg << red_fail;
    ASSERT_EQ(aliceKey.key.size(), KB) << gen_msg << red_fail;
    std::cout << gen_msg << green_pass;

    const std::string sync_msg = "\tReplicating a 1KB one-time-pad...";
    std::cout << sync_msg << std::string(sync_msg.length(), '\b');
    std::vector<uint8_t> bobKey;
    ASSERT_NO_THROW(
        bobKey = _BobClient->genSync(aliceKey.metadata)
    ) << sync_msg << red_fail;
    ASSERT_EQ(bobKey.size(), KB) << gen_msg << red_fail;
    std::cout << sync_msg << green_pass;

    const std::string cerify_msg = "\tVerifying keys match.............";
    std::cout << cerify_msg << std::string(cerify_msg.length(), '\b');
    ASSERT_EQ(
        std::string(aliceKey.key.begin(), aliceKey.key.end()), std::string(bobKey.begin(), bobKey.end())
    ) << cerify_msg << red_fail;
    std::cout << cerify_msg << green_pass;
}

TEST_F(KeyGenTest, ValidateInputs) {
    // Note: 
    std::cout << "\tValidating input error handling." << std::endl;
    // Check for empty tokens
    auto temp_client = IKeyGenDistributedClient::create();
    EXPECT_THROW(temp_client->initialize(""), InvalidArgument);
    // Check for bad tokens
    temp_client = IKeyGenDistributedClient::create();
    temp_client->initialize("xxxx");
    EXPECT_THROW(temp_client->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256), CannotDownload);
    // Invalid AES Key size
    EXPECT_THROW(_AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256, 33), InvalidArgument);
    // OTP too large
    EXPECT_THROW(_AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, 10 * MB + 1), InvalidArgument);
    // Corrupted Metadata
    SymmetricKeyData aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256);
    std::vector<uint8_t> extraBytes = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    extraBytes.insert(extraBytes.end(), aliceKey.metadata.begin(), aliceKey.metadata.end() );
    EXPECT_THROW(_BobClient->genSync(extraBytes), DataCorrupted);
}