#include "QryptSecurity/qryptsecurity.h"
#include "QryptSecurity/qryptsecurity_exceptions.h"
#include "QryptSecurity/qryptsecurity_logging.h"

#include "common.h"

#include <filesystem>
#include <iostream>
#include <thread>
#include <gtest/gtest.h>

using namespace QryptSecurity;

static const uint64_t KB = 1024;
static const uint64_t MB = 1024 * 1024;

static const std::string green_pass = "\x1B[32mPASS\x1B[0m";
static const std::string red_fail = "\x1B[31mFAIL\x1B[0m";
static const std::string gray_text = "\x1B[90m";
static const std::string white_text = "\x1B[0m";

/*
    Validation tests

    A test suite intended to simultaneously validate and demonstrate the capabilities of the Qrypt SDK
    Places a high empasis on output readability, as this will be run OnAttach with the devcontainer
 */

class KeyGenTest : public ::testing::Test {
  protected:
    std::unique_ptr<IKeyGenDistributedClient> _AliceClient = nullptr;
    std::unique_ptr<IKeyGenDistributedClient> _BobClient = nullptr;

    void SetUp() override {
        _AliceClient = IKeyGenDistributedClient::create();
        _AliceClient->initialize(sdk_token);
        _BobClient = IKeyGenDistributedClient::create();
        _BobClient->initialize(sdk_token);
        std::cout << white_text;
    }

    void TearDown() override {
        _AliceClient = nullptr;
        _BobClient = nullptr;
    }
};

TEST_F(KeyGenTest, AES256) {
    const std::string gen_msg = white_text + "Generating an AES256 key..........";
    std::cout << gen_msg << std::flush; // Case message
    std::cout << std::string(gen_msg.length(), '\b') << gray_text; // Return cursor to top so gtest error messages can overwrite case message
    SymmetricKeyData aliceKey;
    ASSERT_NO_THROW(
        aliceKey = _AliceClient->genInit(AES_256_SIZE)
    ) << gen_msg << red_fail; // Re-print case message as a gtest diagnostic message in the event of a failure
    ASSERT_EQ(aliceKey.key.size(), 32) << gen_msg << red_fail;
    std::cout << gen_msg << green_pass << std::endl; // Print success

    const std::string sync_msg = white_text + "Replicating the AES256 key........";
    std::cout << sync_msg << std::flush;
    std::cout << std::string(sync_msg.length(), '\b') << gray_text;
    std::vector<uint8_t> bobKey;
    ASSERT_NO_THROW(
        bobKey = _BobClient->genSync(aliceKey.metadata)
    ) << sync_msg << red_fail;
    ASSERT_EQ(bobKey.size(), 32) << sync_msg << red_fail;
    std::cout << sync_msg << green_pass << std::endl;

    const std::string certify_msg = white_text + "Verifying keys match..............";
    std::cout << certify_msg << std::flush;
    std::cout << std::string(certify_msg.length(), '\b') << gray_text;
    ASSERT_EQ(
        byteVecToHexStr(aliceKey.key), byteVecToHexStr(bobKey)
    ) << certify_msg << red_fail;
    std::cout << certify_msg << green_pass << std::endl;
}

TEST_F(KeyGenTest, OTP1KB) {
    const std::string gen_msg = white_text + "Generating a 1KB one-time-pad......";
    std::cout << gen_msg << std::flush;
    std::cout << std::string(gen_msg.length(), '\b') << gray_text;
    SymmetricKeyData aliceKey;
    ASSERT_NO_THROW(
        aliceKey = _AliceClient->genInit(KB)
    ) << gen_msg << red_fail;
    ASSERT_EQ(aliceKey.key.size(), KB) << gen_msg << red_fail;
    std::cout << gen_msg << green_pass << std::endl;

    const std::string sync_msg = white_text + "Replicating the 1KB one-time-pad...";
    std::cout << sync_msg << std::flush;
    std::cout << std::string(sync_msg.length(), '\b') << gray_text;
    std::vector<uint8_t> bobKey;
    ASSERT_NO_THROW(
        bobKey = _BobClient->genSync(aliceKey.metadata)
    ) << sync_msg << red_fail;
    ASSERT_EQ(bobKey.size(), KB) << sync_msg << red_fail;
    std::cout << sync_msg << green_pass << std::endl;

    const std::string certify_msg = white_text + "Verifying keys match...............";
    std::cout << certify_msg << std::flush;
    std::cout << std::string(certify_msg.length(), '\b') << gray_text;
    ASSERT_EQ(
        byteVecToHexStr(aliceKey.key), byteVecToHexStr(bobKey)
    ) << certify_msg << red_fail;
    std::cout << certify_msg << green_pass << std::endl;
}

TEST_F(KeyGenTest, OTP1MB) {
    const std::string gen_msg = white_text + "Generating a 1MB one-time-pad......";
    std::cout << gen_msg << std::flush;
    std::cout << std::string(gen_msg.length(), '\b') << gray_text;
    SymmetricKeyData aliceKey;
    ASSERT_NO_THROW(
        aliceKey = _AliceClient->genInit(MB)
    ) << gen_msg << red_fail;
    ASSERT_EQ(aliceKey.key.size(), MB) << gen_msg << red_fail;
    std::cout << gen_msg << green_pass << std::endl;

    const std::string sync_msg = white_text + "Replicating the 1MB one-time-pad...";
    std::cout << sync_msg << std::flush;
    std::cout << std::string(sync_msg.length(), '\b') << gray_text;
    std::vector<uint8_t> bobKey;
    ASSERT_NO_THROW(
        bobKey = _BobClient->genSync(aliceKey.metadata)
    ) << sync_msg << red_fail;
    ASSERT_EQ(bobKey.size(), MB) << sync_msg << red_fail;
    std::cout << sync_msg << green_pass << std::endl;

    const std::string certify_msg = white_text + "Verifying keys match...............";
    std::cout << certify_msg << std::flush;
    std::cout << std::string(certify_msg.length(), '\b') << gray_text;
    if(byteVecToHexStr(aliceKey.key) != byteVecToHexStr(bobKey)) {
        FAIL() << "Generated/Replicated keys do not match! (Too large to print diff)" << std::endl << certify_msg << red_fail;
    }
    std::cout << certify_msg << green_pass << std::endl;
}

TEST_F(KeyGenTest, CustomTTL) {
    KeyConfiguration keyConfig = {};
    keyConfig.ttl = 5;
    SymmetricKeyData aliceKey;

    const std::string gen_msg = white_text + "Generating an AES256 key with a 5 second TTL......";
    std::cout << gen_msg << std::flush;
    std::cout << std::string(gen_msg.length(), '\b') << gray_text;
    ASSERT_NO_THROW(
        aliceKey = _AliceClient->genInit(AES_256_SIZE, keyConfig)
    ) << gen_msg << red_fail;
    std::cout << gen_msg << green_pass << std::endl;

    const std::string sync_msg_1 = white_text + "Replicating the AES256 key immediately............";
    std::cout << sync_msg_1 << std::flush;
    std::cout << std::string(sync_msg_1.length(), '\b') << gray_text;
    std::vector<uint8_t> bobKey;
    ASSERT_NO_THROW(
        bobKey = _BobClient->genSync(aliceKey.metadata)
    ) << sync_msg_1 << red_fail;
    std::cout << sync_msg_1 << green_pass << std::endl;

    const std::string certify_msg = white_text + "Verifying keys match..............................";
    std::cout << certify_msg << std::flush;
    std::cout << std::string(certify_msg.length(), '\b') << gray_text;
    ASSERT_EQ(
        byteVecToHexStr(aliceKey.key), byteVecToHexStr(bobKey)
    ) << certify_msg << red_fail;
    std::cout << certify_msg << green_pass << std::endl;

    for (int sec = keyConfig.ttl; sec > 0; --sec) {
        printf("Counting down: %d seconds\n", sec);
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::seconds(1));  
    }

    const std::string sync_msg_2 = white_text + "Verifying key replication fails after 5 seconds...";
    std::cout << sync_msg_2 << std::flush;
    std::cout << std::string(sync_msg_2.length(), '\b') << gray_text;
    ASSERT_THROW(
        bobKey = _BobClient->genSync(aliceKey.metadata), QryptSecurityException
    ) << sync_msg_2 << red_fail;
    std::cout << sync_msg_2 << green_pass << std::endl;
}

TEST(EaaSTest, VerifyNISTSuccess) {
    std::cout << gray_text << "NIST Statistical Test Suite for Random Number Generators (Special Publication 800-22 Rev 1a)" << std::endl;
    std::string case_msg = white_text + "Verifying randomness of Qrypt Entropy stream...";
    std::cout << case_msg << std::flush;
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    std::filesystem::path cwd = std::filesystem::current_path();
    if (cwd.filename() == "build") {
        cwd /= ".."; // Navigate to project root if we're in the build directory
    }
    std::string cmd = "python3 " + std::string((cwd / "test") / "parse_nist_api.py");
    int fail = std::system(cmd.c_str());
    std::string script_stdout = testing::internal::GetCapturedStdout();
    std::string script_stderr = testing::internal::GetCapturedStderr();
    if (fail || script_stderr.length() > 0) {
        std::cout << std::string(case_msg.length(), '\b') << gray_text;
        FAIL() << script_stderr << case_msg << red_fail;
    }
     std::cout << green_pass << std::endl;
}