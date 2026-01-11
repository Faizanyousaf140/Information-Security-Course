#include "AES.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <cstring>

AES::AES(const std::string& keyStr) {
    memset(key, 0, 32);
    memset(iv, 0, 16);
    memcpy(key, keyStr.c_str(), keyStr.length() < 32 ? keyStr.length() : 32);
}

std::string AES::Encrypt(const std::string& plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    int len;
    int ciphertext_len;
    unsigned char* ciphertext = new unsigned char[plaintext.length() + EVP_CIPHER_block_size(EVP_aes_256_cbc())];

    EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)plaintext.c_str(), plaintext.length());
    ciphertext_len = len;

    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    std::string result((char*)ciphertext, ciphertext_len);
    delete[] ciphertext;
    return result;
}

std::string AES::Decrypt(const std::string& ciphertext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    int len;
    int plaintext_len;
    unsigned char* plaintext = new unsigned char[ciphertext.length()];

    EVP_DecryptUpdate(ctx, plaintext, &len, (unsigned char*)ciphertext.c_str(), ciphertext.length());
    plaintext_len = len;

    EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    std::string result((char*)plaintext, plaintext_len);
    delete[] plaintext;
    return result;
}

std::string AES::ToBitString(const std::string& data) {
    std::string bitString;
    for (unsigned char c : data) {
        for (int i = 7; i >= 0; i--) {
            bitString += ((c >> i) & 1) ? '1' : '0';
        }
    }
    return bitString;
}

std::string AES::FromBitString(const std::string& bitString) {
    std::string result;
    for (size_t i = 0; i < bitString.length(); i += 8) {
        unsigned char byte = 0;
        for (int j = 0; j < 8 && i + j < bitString.length(); j++) {
            if (bitString[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        result += byte;
    }
    return result;
}