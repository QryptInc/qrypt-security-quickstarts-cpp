#ifndef KEYGEN_H
#define KEYGEN_H

#include "QryptSecurity/qryptsecurity.h"
#include "QryptSecurity/qryptsecurity_exceptions.h"
#include "QryptSecurity/qryptsecurity_logging.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

class KeyGen {
public:
    KeyGen(std::string token, std::string key_type = "otp", size_t key_len = 32, uint32_t key_ttl = 3600, std::string key_format = "hexstr",
           ::QryptSecurity::LogLevel log_level = ::QryptSecurity::LogLevel::QRYPTSECURITY_LOG_LEVEL_DISABLE,
           std::string cacert_path = "");

    void generate(std::ostream& key_out, std::ostream& metadata_out);
    void replicate(std::ostream& key_out, std::istream& metadata_in);
private:
    std::unique_ptr<QryptSecurity::IKeyGenDistributedClient> sdk_client;
    std::string key_type;
    size_t key_len;
    uint32_t key_ttl;
    std::string key_format;
};

#endif /* KEYGEN_H */