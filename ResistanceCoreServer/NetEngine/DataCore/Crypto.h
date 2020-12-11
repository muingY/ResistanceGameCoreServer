#ifndef CRYPTO_H
#define CRYPTO_H

#include "../NetRootHeader.h"

class Crypto
{
public:
    Crypto();
    ~Crypto();

    void Initialize(string key);
    
    string EncDecryptStr(string str);

private:
    string key;
};

#endif