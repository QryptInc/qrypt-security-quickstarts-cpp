#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <string>

const uint32_t IVLengthInBytes = 12;
const uint32_t AESKeyLengthInBytes = 32;
const uint32_t AESKeyWithIVLengthInBytes = AESKeyLengthInBytes + IVLengthInBytes;
const uint32_t AETagSizeInBytes = 2;
constexpr int OPENSSL_SUCCESS = 1;
constexpr int BMP_HEADER_SIZE = 54;

struct BitmapData {
    std::vector<uint8_t> header;
    std::vector<uint8_t> body;
};

std::vector<uint8_t> readFromFile(const std::string filename);

void writeToFile(const std::string filename, std::vector<uint8_t>& buffer);

BitmapData readBitmap(const std::string filename);

void writeBitmap(const std::string filename, const BitmapData& bitmapData);

std::vector<uint8_t> xorVectors(const std::vector<uint8_t> otp, const std::vector<uint8_t> &data);

std::string convertByteVecToHexStr(std::vector<uint8_t> bytes);

#endif