#include <iostream>

#include "header/AES.h"
#include "header/Encoder.h"
#include "header/Decoder.h"

int main() {
    AES enc("mysecretkey123");

    //std::string original = "Hello World";
    //std::string encrypted = enc.Encrypt(original);
    //std::string bitString = enc.ToBitString(encrypted);
    //std::string decrypted = enc.Decrypt(enc.FromBitString(bitString));

    //std::cout << "Original: " << original << std::endl;
    //std::cout << "Encrypted (bits): " << bitString << std::endl;

    //std::cout << "Decrypted: " << decrypted << std::endl;

    // Embedding
    Encoder embedder("input.mp4", "101110001010011", 12345);
    embedder.embedData("output.mp4");

    // Decoding
    Decoder decoder("output.mp4", 12345);  // Same seed!
    std::string extractedBits = decoder.decodeData();
	std::cout << "Extracted Bits: " << extractedBits << std::endl;
    // extractedBits == "1101001010..."
	//std::string decrypted = enc.Decrypt(enc.FromBitString(extractedBits));
	//std::cout << "Decrypted: " << decrypted << std::endl;


    return 0;
}