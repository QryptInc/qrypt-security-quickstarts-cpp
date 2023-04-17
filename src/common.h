#ifndef COMMON_H
#define COMMON_H

#include <stdexcept>
#include <map>
#include <vector>
#include <string>
#include <tuple>

const char* demo_token = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6IjExOThmMDEyZTA2ZjRmZjFhYWE4NzM2MzFkNTkxNmU1In0.e"
    "yJleHAiOjE3MTMyODI2NDYsIm5iZiI6MTY4MTc0NjY0NiwiaXNzIjoiQVVUSCIsImlhdCI6MTY4MTc0NjY0NiwiZ3JwcyI6WyJQVUIiXSwiYXVkIjp"
    "bIlFERUEiLCJSUFMiXSwicmxzIjpbIlFERVVTUiIsIlJORFVTUiJdLCJjaWQiOiJmYUtwQVhVUVI3NUJURzdIVjFJMksiLCJkdmMiOiI3YzA1MDc2Z"
    "DE0ODU0YTRkYjdkZTJhYjVhN2U4YTVkYSIsImp0aSI6IjBjZDY4YjY4ODNmNzRmN2FiYjgwMGYyN2ZiYTYyMDNiIiwidHlwIjozfQ.cHBCxYkQQgZt"
    "4Z0Y6ZGYeZKzBOIT5e4oZ962T153svZ7QXO2s3Ed6VLGk75CZdFRb2rFwFdZnnH2xK5tQdhcqg";
std::string test_token = demo_token;

const uint32_t IVLengthInBytes = 12;
const uint32_t AESKeyLengthInBytes = 32;
const uint32_t AESKeyWithIVLengthInBytes = AESKeyLengthInBytes + IVLengthInBytes;
const uint32_t AETagSizeInBytes = 2;
constexpr int OPENSSL_SUCCESS = 1;
constexpr int BMP_HEADER_SIZE = 54;

std::vector<uint8_t> xorVectors(const std::vector<uint8_t> otp, const std::vector<uint8_t> &data);

std::string byteVecToHexStr(std::vector<uint8_t>& bytes);

size_t hexCharToInt(char input);

std::vector<uint8_t> hexStrToByteVec(std::string& str);

#endif /* COMMON_H */