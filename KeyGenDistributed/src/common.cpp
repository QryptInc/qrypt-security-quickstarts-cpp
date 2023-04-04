#include "common.h"

#include <exception>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>

std::tuple<std::string, std::string> tokenizeArg(std::string arg) {
    std::string flag, value;
    if(!(arg.find("--") == 0)) {
        throw invalid_arg_exception("Invalid argument: " + arg);
    }
    size_t delim_pos = arg.find("=");
    flag = arg.substr(0, delim_pos);
    if (arg.length() > delim_pos) {
        value = arg.substr(delim_pos + 1);
    }
    return {flag, value};
}

std::vector<uint8_t> readFromFile(const std::string filename) {
    std::ifstream input(filename, std::ios::binary);
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
    input.close();
    std::string buffer_string(buffer.begin(), buffer.end());
    if (buffer_string.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos) {
        return hexStrToByteVec(buffer_string);
    }
    return buffer;
}

void writeToFile(const std::string filename, std::vector<uint8_t> &buffer) {
    std::ofstream output(filename, std::ios::out | std::ios::binary);
    output.write((char *)&buffer[0], buffer.size());
    output.close();
}

void writeToFile(const std::string filename, std::string &buffer) {
    std::ofstream output(filename, std::ios::out);
    output << buffer;
    output.close();
}

BitmapData readBitmap(const std::string filename) {
    std::vector<uint8_t> buffer = readFromFile(filename);
    BitmapData bitmapData = {};
    bitmapData.header = std::vector<uint8_t>(buffer.begin(), buffer.begin() + BMP_HEADER_SIZE);
    bitmapData.body = std::vector<uint8_t>(buffer.begin() + BMP_HEADER_SIZE, buffer.end());
    return bitmapData;
}

void writeBitmap(const std::string filename, const BitmapData& bitmapData) {
    std::vector<uint8_t> buffer(bitmapData.header.begin(), bitmapData.header.end());
    buffer.insert(buffer.end(), bitmapData.body.begin(), bitmapData.body.end());
    writeToFile(filename, buffer);
}

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

std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(),str.begin(), ::toupper);
    return str;
}
