#ifndef COMMON_H
#define COMMON_H

#include <exception>
#include <map>
#include <vector>
#include <string>
#include <tuple>

using KeyValuePair = std::tuple<std::string, std::string>;

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

class invalid_arg_exception : public std::exception {
  private:
    std::string _ExceptionMsg;

  public:
    invalid_arg_exception(std::string message) {
        _ExceptionMsg = message;
    }
    ~invalid_arg_exception() = default;
    const char *what() const noexcept override { return _ExceptionMsg.c_str(); };
};

void generate(std::vector<KeyValuePair> args);

void encrypt(std::string operation, std::vector<KeyValuePair> args);

// void decrypt(std::vector<KeyValuePair> args);

KeyValuePair tokenizeArg(std::string arg);

std::vector<uint8_t> readFromFile(const std::string filename);

void writeToFile(const std::string filename, std::vector<uint8_t>& buffer);

void writeToFile(const std::string filename, std::string &buffer);

BitmapData readBitmap(const std::string filename);

void writeBitmap(const std::string filename, const BitmapData& bitmapData);

std::vector<uint8_t> xorVectors(const std::vector<uint8_t> otp, const std::vector<uint8_t> &data);

std::string byteVecToHexStr(std::vector<uint8_t>& bytes);

size_t hexCharToInt(char input);

std::vector<uint8_t> hexStrToByteVec(std::string& str);

std::string toUpper(std::string str);

#endif