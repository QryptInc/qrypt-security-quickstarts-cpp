#include "common.h"
#include "keygen.h"
#include <curl/curl.h>

using KeyValuePair = std::tuple<std::string, std::string>;

static const char* FLASK_PORT = "5000";
static long curlConnectionTimeout = 10L;

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

static size_t curlWriteCallback(char* data, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(data, totalSize);
    return totalSize;
}

void uploadFileToCodespace(const std::string& filename, const std::string& codespaceName) {
    std::string url = "https://" + codespaceName + "-" + FLASK_PORT +  ".preview.app.github.dev/upload";

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, curlConnectionTimeout);

        // Set the multipart/form-data request
        curl_mime* mime = curl_mime_init(curl);
        curl_mimepart* part = curl_mime_addpart(mime);
        curl_mime_name(part, "file");
        curl_mime_filedata(part, filename.c_str());

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        std::string serverResponse;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &serverResponse);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::cout << "File uploaded successfully to the remote codespace at " << serverResponse << std::endl;
        } else {
            throw std::runtime_error("Error: " + std::string(curl_easy_strerror(res)));
        }

        curl_mime_free(mime);
        curl_easy_cleanup(curl);
    } else {
        throw std::runtime_error("Failed to initialize libcurl");
    }
}
