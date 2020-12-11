#include "Crypto.h"

Crypto::Crypto()
{
    key.clear();
}
Crypto::~Crypto() {}

void Crypto::Initialize(string key)
{
    this->key = key;
}

#if IS_CRYPTO
string Crypto::EncDecryptStr(string str)
{
    char map[256] = "acvkjasewr23478cxvlkjdflk2349ds8fxcv,masfkjer234";
    int size = key.size() - 1;
    for (int i = 0; i < 256; ++i)
    {
        map[i] = key[i % size] & map[i];
    }

    for (int i = 0; i < str.length(); i++)
    {
        str[i] ^= map[i];
    }

    return str;
}
#else
string Crypto::EncDecryptStr(string str)
{
    return str;
}
#endif