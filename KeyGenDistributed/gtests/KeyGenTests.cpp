#include "QryptSecurity/qryptsecurity.h"

#include <gtest/gtest.h>

using namespace QryptSecurity;

static const uint64_t KB = 1024;
static const uint64_t MB = 1024 * 1024;

class KeyGenDistributedTest : public ::testing::Test {
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

TEST_F(KeyGenDistributedTest, InvalidKeyMode) {
    initialize();
    EXPECT_THROW(_AliceClient->genInit(SymmetricKeyMode::NUM_SYMMETRIC_KEY_MODES), InvalidArgument);
}

TEST_F(KeyGenDistributedTest, EmptyToken) {
    _Token = "";
    EXPECT_THROW(_AliceClient->initialize(_Token), InvalidArgument);
}

TEST_F(KeyGenDistributedTest, InvalidToken) {
    _Token = "xxxxx";
    initialize();
    EXPECT_THROW(_AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256), CannotDownload);
}

TEST_F(KeyGenDistributedTest, InvalidAESKeySize) {
    initialize();
    // AES256 key should be 32 bytes instead of 33 bytes
    EXPECT_THROW(_AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256, 33), InvalidArgument);
}

TEST_F(KeyGenDistributedTest, GenerateAESKey) {
    initialize();
    SymmetricKeyData aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256);
    std::vector<uint8_t> bobKey = _BobClient->genSync(aliceKey.metadata);
    EXPECT_EQ(aliceKey.key, bobKey);
}

TEST_F(KeyGenDistributedTest, GenerateOTP1Byte) {
    initialize();
    SymmetricKeyData aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, 1);
    std::vector<uint8_t> bobKey = _BobClient->genSync(aliceKey.metadata);
    EXPECT_EQ(aliceKey.key, bobKey);
}

TEST_F(KeyGenDistributedTest, GenerateOTP16Byte) {
    initialize();
    SymmetricKeyData aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, 16);
    std::vector<uint8_t> bobKey = _BobClient->genSync(aliceKey.metadata);
    EXPECT_EQ(aliceKey.key, bobKey);
}

TEST_F(KeyGenDistributedTest, GenerateOTP1KB) {
    initialize();
    SymmetricKeyData aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, KB);
    std::vector<uint8_t> bobKey = _BobClient->genSync(aliceKey.metadata);
    EXPECT_EQ(aliceKey.key, bobKey);
}

TEST_F(KeyGenDistributedTest, GenerateOTP32KB) {
    initialize();
    SymmetricKeyData aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, 32 * KB);
    std::vector<uint8_t> bobKey = _BobClient->genSync(aliceKey.metadata);
    EXPECT_EQ(aliceKey.key, bobKey);
}

TEST_F(KeyGenDistributedTest, KeySizeLowerLimit) {
    initialize();
    EXPECT_THROW(_AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, 0), InvalidArgument);
}

TEST_F(KeyGenDistributedTest, KeySizeUpperLimit) {
    initialize();
    EXPECT_THROW(_AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, 10 * MB + 1), InvalidArgument);
}

TEST_F(KeyGenDistributedTest, MetadataWithExtraPrefix) {
    initialize();
    SymmetricKeyData aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256);

    std::vector<uint8_t> extraBytes = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    aliceKey.metadata.insert(aliceKey.metadata.begin(), extraBytes.begin(), extraBytes.end() );

    EXPECT_THROW(_BobClient->genSync(aliceKey.metadata), DataCorrupted);
}

TEST_F(KeyGenDistributedTest, CorruptedMetadata) {
    initialize();
    SymmetricKeyData aliceKey = _AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_AES_256);

    std::vector<uint8_t> extraBytes = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    aliceKey.metadata.insert(aliceKey.metadata.begin() + 50, extraBytes.begin(), extraBytes.end() );

    EXPECT_THROW(_BobClient->genSync(aliceKey.metadata), DataCorrupted);
}
