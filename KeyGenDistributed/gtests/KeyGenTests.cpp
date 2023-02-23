#include "QryptSecurity/qryptsecurity.h"
#include "QryptSecurity/qryptsecurity_exceptions.h"
#include "QryptSecurity/qryptsecurity_logging.h"

#include <gtest/gtest.h>

using namespace QryptSecurity;

static const uint64_t KB = 1024;
static const uint64_t MB = 1024 * 1024;

void populate_X(blast_multi_stream *X,
                size_t target_size,
                size_t num_in_memory_blocks,
                size_t num_in_storage_blocks,
                bool use_kb) {

    // Just being safe
    zero_bytes(X, sizeof(blast_multi_stream));

    size_t num_blocks               = num_in_memory_blocks + num_in_storage_blocks;
    size_t in_memory_block_counter  = 0;
    size_t in_storage_block_counter = 0;
    X->blast_streams                = (blast_stream *)alloc_bytes(num_blocks * sizeof(blast_stream));
    X->blast_streams_len            = num_blocks;
    X->beta                         = 0.5;

    // Load up our streams
    for (size_t i = 0; i < num_blocks; i++) {
        char *file_location      = NULL;
        size_t file_location_len = 0;
        // Sizing request
        get_test_filename(target_size, i, file_location, &file_location_len, use_kb);
        file_location = (char *)alloc_bytes(file_location_len);
        // Set file name
        get_test_filename(target_size, i, file_location, &file_location_len, use_kb);

        // Open file for processing
        FILE *file = fopen(file_location, "rb");
        if (file == NULL) {
            printf("Unable to open file: %s\n", file_location);
            continue;
        }
        fseek(file, 0L, SEEK_END);
        size_t random_file_len = ftell(file);
        fseek(file, 0L, SEEK_SET);

        if (in_memory_block_counter < num_in_memory_blocks) {
            // Read in file if in-memory stream
            uint8_t *buffer = (uint8_t *)alloc_bytes(random_file_len);
            fread(buffer, 1, random_file_len, file);

            X->blast_streams[i].type     = BLAST_MEMORY_STREAM;
            X->blast_streams[i].location = (void *)buffer;
            in_memory_block_counter++;

            // Clean-up
            free_bytes(file_location, file_location_len);
        } else {
            // Just record file info if file stream
            X->blast_streams[i].type     = BLAST_FILE_STREAM;
            X->blast_streams[i].location = (void *)file_location;
            in_storage_block_counter++;
        }

        // Grab our lengths
        X->blast_streams[i].stream_len = random_file_len;
        X->n += random_file_len;

        // Done with file
        fclose(file);
    }
}

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
    EXPECT_THROW(_AliceClient->genInit(SymmetricKeyMode::SYMMETRIC_KEY_MODE_OTP, 15), InvalidArgument);
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
