#pragma once
#include <string>

class AES {
private:
    unsigned char key[32];
    unsigned char iv[16];

public:
    AES(const std::string& keyStr);
    std::string Encrypt(const std::string& plaintext);
    std::string Decrypt(const std::string& ciphertext);
    std::string ToBitString(const std::string& data);
    std::string FromBitString(const std::string& bitString);
};