#include "common.h"

#include <exception>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace {
const char* _demo_token = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6IjExOThmMDEyZTA2ZjRmZjFhYWE4NzM2MzFkNTkxNmU1In0.e"
    "yJleHAiOjE3MTMyODI2NDYsIm5iZiI6MTY4MTc0NjY0NiwiaXNzIjoiQVVUSCIsImlhdCI6MTY4MTc0NjY0NiwiZ3JwcyI6WyJQVUIiXSwiYXVkIjp"
    "bIlFERUEiLCJSUFMiXSwicmxzIjpbIlFERVVTUiIsIlJORFVTUiJdLCJjaWQiOiJmYUtwQVhVUVI3NUJURzdIVjFJMksiLCJkdmMiOiI3YzA1MDc2Z"
    "DE0ODU0YTRkYjdkZTJhYjVhN2U4YTVkYSIsImp0aSI6IjBjZDY4YjY4ODNmNzRmN2FiYjgwMGYyN2ZiYTYyMDNiIiwidHlwIjozfQ.cHBCxYkQQgZt"
    "4Z0Y6ZGYeZKzBOIT5e4oZ962T153svZ7QXO2s3Ed6VLGk75CZdFRb2rFwFdZnnH2xK5tQdhcqg";
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