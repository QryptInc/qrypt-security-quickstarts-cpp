#include "common.h"

#include <exception>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>


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