#include "common.h"
#include "keygen.h"

using KeyValuePair = std::tuple<std::string, std::string>;

KeyGen::KeyGen(std::string token, std::string key_type, size_t key_len, uint32_t key_ttl, std::string key_format, 
               QryptSecurity::LogLevel log_level, std::string cacert_path) :
               key_type(key_type), key_len(key_len), key_ttl(key_ttl), key_format(key_format) {

    if (key_type != "aes" && key_type != "otp") {
        throw std::invalid_argument("Invalid key type: \"" + key_type + "\"");
    }
    if (key_format != "hexstr" && key_format != "binary") {
        throw std::invalid_argument("Invalid key format: \"" + key_format + "\"");
    }

    QryptSecurity::setLogLevel(log_level);
    // Create and initialize a client from the QryptSecurity SDK
    sdk_client = QryptSecurity::IKeyGenDistributedClient::create();
    if (cacert_path.empty()) {
        sdk_client->initialize(token);
    }
    else {
        // 
        sdk_client->initialize(token, cacert_path);
    }
}

// Generate a key using the Qrypt SDK
void KeyGen::generate(std::ostream& key_out, std::ostream& meta_out) {

    // Generate the key and metadata
    QryptSecurity::SymmetricKeyData key_and_metadata = {};
    if (key_type == "aes") {
        key_and_metadata = sdk_client->genInit(QryptSecurity::AES_256_SIZE, QryptSecurity::KeyConfiguration(key_ttl));
    }
    else if (key_type == "otp") {
        key_and_metadata = sdk_client->genInit(key_len, QryptSecurity::KeyConfiguration(key_ttl));
    }

    // Output key in either hexadecimal or binary format
    if (key_format == "hexstr") {
        key_out << byteVecToHexStr(key_and_metadata.key);
    } 
    else  {
        key_out.write((char *)&(key_and_metadata.key)[0], key_and_metadata.key.size());
    }

    // Output metadata
    meta_out.write((char *)&(key_and_metadata.metadata)[0], key_and_metadata.metadata.size());
}

void KeyGen::replicate(std::ostream& key_out, std::istream& meta_in) {

    // Read metadata
    std::vector<uint8_t> metadata(std::istreambuf_iterator<char>(meta_in), {});

    // Replicate the key using the retrieved metadata
    std::vector<uint8_t> key = sdk_client->genSync(metadata);

    // Output key in either hexadecimal or binary format
    if (key_format == "hexstr") {
        key_out << byteVecToHexStr(key);
    } 
    else  {
        key_out.write((char *)&(key)[0], key.size());
    }
}