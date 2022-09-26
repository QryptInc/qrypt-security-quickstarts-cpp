#include "common.h"

#include <exception>
#include <fstream>
#include <iomanip>
#include <sstream>

std::vector<uint8_t> readFromFile(const std::string filename) {
    std::ifstream input(filename, std::ios::binary);
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
    input.close();
    return buffer;
}

void writeToFile(const std::string filename, std::vector<uint8_t> &buffer) {
    std::ofstream output(filename, std::ios::out | std::ios::binary);
    output.write((char *)&buffer[0], buffer.size());
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

std::string convertByteVecToHexStr(std::vector<uint8_t> bytes) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto byte : bytes) {
        int byteValue = byte;
        oss << std::setw(2) << byteValue;
    }
    std::string result = oss.str();
    return result;
}
