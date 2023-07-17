#ifndef EAAS_H
#define EAAS_H

#include <string>

class EaaS {
public:
    EaaS(const std::string& token): _token{token} {};
    std::string requestEntropy(uint32_t size = 1);

private:
    std::string _token;
};

#endif /* EAAS_H */