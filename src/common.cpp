#include "common.h"

#include <exception>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <curl/curl.h>

static const char* FLASK_PORT = "5000";
static long curlConnectionTimeout = 10L;

namespace {
const char* _demo_token = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6ImM1NDM3NTlhMjhiMjRmYzQ4NGY1OWIyNjBmNTMwZjA2In0.e"
    "yJleHAiOjE3NDcxNjU2MDUsIm5iZiI6MTcxNTYyOTYwNSwiaXNzIjoiQVVUSCIsImlhdCI6MTcxNTYyOTYwNSwiZ3JwcyI6WyJQVUIiXSwiYXVkIjpb"
    "IlJQUyIsIlFERUEiXSwicmxzIjpbIlJORFVTUiIsIlFERVVTUiJdLCJjaWQiOiJZdkN4bklscmlOYk8wRXpKWXBKUWwiLCJkdmMiOiJiYjZhMjU5YWQ"
    "4ODk0MGE1YjMyZTU0YWJiYWUxZWQ0MyIsImp0aSI6Ijc3Nzc1NzM4Y2Y0MTRmMTVhOTlmZmRhZDg2NzgyMDNjIiwidHlwIjozfQ.Ka0GFmvbfmrPdnL"
    "fE1D6hV9oE8y9zvBZB0OKDDA8FiuTDWC5Pv3mn8VLH9hz6iNr96brYB0BdGirewqRWkYqMA";
}
std::string sdk_token = _demo_token;

std::vector<uint8_t> xorVectors(const std::vector<uint8_t> otp, const std::vector<uint8_t> &data) {
    std::vector<uint8_t> result;
    if(otp.size() != data.size()) {
        throw std::runtime_error("One time pad size does not match data size.");
    }
    for(int i = 0; i < otp.size(); i++) {
        result.push_back(otp[i] ^ data[i]);
    }
    return result;
}

std::string byteVecToHexStr(std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto byte : bytes) {
        int byteValue = byte;
        oss << std::setw(2) << byteValue;
    }
    std::string result = oss.str();
    return result;
}

size_t hexCharToInt(char input) {
    if (input >= '0' && input <= '9') {
        return input - '0';
    } else if (input >= 'a' && input <= 'f') {
        return input - 'a' + 10;
    }
    throw std::runtime_error("Invalid character");
}

std::vector<uint8_t> hexStrToByteVec(std::string& str) {
    std::vector<uint8_t> buffer(str.size() / 2);
    for (size_t i = 0; i < str.size(); i += 2) {
        buffer[i / 2] = (hexCharToInt(str[i + 1]) & 0x0F) + ((hexCharToInt(str[i]) << 4) & 0xF0);
    }
    return buffer;
}

static size_t curlWriteCallback(char* data, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(data, totalSize);
    return totalSize;
}

std::string curlRequest(const std::string& fqdn, const std::string& filename, const std::vector<std::string>& headers) {

    CURL *curl;
    std::string serverResponse;
    curl = curl_easy_init();
    if(curl) {
        
        // set standard options
        curl_easy_setopt(curl, CURLOPT_URL, fqdn.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, curlConnectionTimeout);
        
        // define where response is saved
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &serverResponse);

        // set the multipart/form-data request
        curl_mime* mime = curl_mime_init(curl);
        curl_mimepart* part = curl_mime_addpart(mime);
        if (!filename.empty()) {
            curl_mime_name(part, "file");
            curl_mime_filedata(part, filename.c_str());
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        }
        
        // set headers
        struct curl_slist *list = NULL;
        if (headers.size() > 0) {
            for (auto& header : headers) {
                list = curl_slist_append(list, header.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        }

        // execute
        CURLcode res = curl_easy_perform(curl);

        // process response
        if (res == CURLE_OK) {
            long http_response_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_response_code);
            if (http_response_code == 200) {
                std::cout << serverResponse << std::endl;
            } else if (http_response_code != 200) {
                throw std::runtime_error("Unexpected HTTP response:\n" + serverResponse);
            }
        } else {
            throw std::runtime_error(std::string(curl_easy_strerror(res)));
        }
     
        // cleanup
        curl_easy_cleanup(curl);
        if (mime != NULL) {
            curl_mime_free(mime);
        }
        if (list != NULL) {
            curl_slist_free_all(list);
        }
    }
    else {
        throw std::runtime_error("Failed to initialize libcurl");
    }

    return serverResponse;
}

void uploadFileToCodespace(const std::string& filename, const std::string& codespaceName) {
    
    // fqdn
    std::string url = "https://" + codespaceName + "-" + FLASK_PORT +  ".app.github.dev/upload";

    // no headers
    std::vector<std::string> empty;

    // send request
    curlRequest(url, filename.c_str(), empty);
}
