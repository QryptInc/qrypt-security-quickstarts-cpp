#include "common.h"
#include "eaas.h"
#include <curl/curl.h>

#include <iostream>

const uint32_t MIN_REQUEST = 1;
const uint32_t MAX_REQUEST = 512;
const std::string EAAS_FQDN = "https://api-eus.qrypt.com/api/v1/quantum-entropy";


// Request
//template <typename OptValueType>
std::string EaaS::requestEntropy(uint32_t size) {

    // check size
    if (size < MIN_REQUEST || size > MAX_REQUEST) {
        throw std::runtime_error("Entropy request size is not within the acceptable range");
    }

    //fqdn
    std::string url = EAAS_FQDN + std::string("?size=") + std::to_string(size);

    //headers
    std::vector<std::string> headers;
    headers.push_back("accept: application/json");
    std::string authBearer = "Authorization: Bearer " + _token;
    headers.push_back(authBearer);

    // file to send
    std::string empty("");

    // send request
    std::string response = curlRequest(url, empty, headers);

    return response;
}