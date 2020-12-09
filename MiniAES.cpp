#include "MiniAES.h"
#include <iostream>
#include <string>
#include <array>
#include <unordered_map>

uint8_t gadd(uint8_t a, uint8_t b) {
    // GF(2^4) addition
    return (a ^ b);
}

uint8_t gmul(uint8_t a, uint8_t b) {
    /*
    GF(2^4) multiplication
    using modified version of russian peasant algorithm
    it also uses branchless programming to prevent timing attacks

    reference: https://en.wikipedia.org/wiki/Finite_field_arithmetic
    */
    uint8_t p = 0;
    for (auto i = 0; i < 4; i++) {
        p ^= -(b & 1) & a;
        auto mask = -((a >> 3) & 1);
        // 0b10011 is x^4 + x + 1
        a = (a << 1) ^ (0b10011 & mask);
        b >>= 1;
    } 

    return p;
}

std::unordered_map<uint8_t, uint8_t> MiniAES::s_box = {
    {0b0000, 0b1110}, {0b1000, 0b0011},
    {0b0001, 0b0100}, {0b1001, 0b1010},
    {0b0010, 0b1101}, {0b1010, 0b0110},
    {0b0011, 0b0001}, {0b1011, 0b1100},
    {0b0100, 0b0010}, {0b1100, 0b0101},
    {0b0101, 0b1111}, {0b1101, 0b1001},
    {0b0110, 0b1011}, {0b1110, 0b0000},
    {0b0111, 0b1000}, {0b1111, 0b0111}
};

matrix MiniAES::blockToArray(std::string s) {
    // converts 16 bit string to 2x2 nibble array
    // uses masks to extract high/low nibble (bit shift, &)
    // smallest data type in C++ is 1 byte (8 bit) so uint8_t gives the most efficient output
    matrix vals;
    for (auto i = 0; i < 2; i++) {
        vals[0][i] = s[i] >> 4; // high nibble
        vals[1][i] = s[i] & 15; // low nibble
    }
    return vals;
}

matrix MiniAES::nibbleSub (matrix block) {
    for (auto i = 0; i < block.max_size(); i++) {
        for (auto j = 0; j < block[i].max_size(); j++)
            block[i][j] = MiniAES::s_box[block[i][j]];
    }

    return block;
}

matrix MiniAES::shiftRow(matrix block) {
    uint8_t temp = block[1][1];
    block[1][1] = block[1][0];
    block[1][0] = temp;
    return block;
}

matrix MiniAES::mixColumn(matrix block) {
    matrix res;
    res[0][0] = gadd(gmul(3, block[0][0]), gmul(2, block[1][0])); // d0
    res[1][0] = gadd(gmul(2, block[0][0]), gmul(3, block[1][0])); // d1
    res[0][1] = gadd(gmul(3, block[0][1]), gmul(2, block[1][1])); // d2
    res[1][1] = gadd(gmul(2, block[0][1]), gmul(3, block[1][1])); // d3

    return res;
}

matrix MiniAES::keyAddition(matrix block, matrix rkey) {
    for (auto i = 0; i < 2; i++)
        for (auto j = 0; j < 2; j++)
            block[i][j] = gadd(block[i][j], rkey[i][j]);

    return block;
}

std::string MiniAES::encrypt (std::string pt) { 
    // block cipher, each block has 16 bits / 2 bytes
    for (auto i = 0; i < pt.length(); i += 2) {
        auto str = pt.substr(i, 2);
        auto block = this->blockToArray(pt.substr(i, 2));
        auto sub = this->nibbleSub(block);
        auto shift = this->shiftRow(block);
        auto mix = this->mixColumn(block);
        auto add = this->keyAddition(block, sub);
    }

    return "hi";
}

 
int main() {
    std::string pt ("hi cfqwef"); // plain text
    std::string key (""); // secret key

    // for (unsigned i = 0; i < pt.length(); i += 2) {
    //     std::cout << pt.substr(i, 2) << std::endl;
    // }

    MiniAES test(key);
    std::cout << test.encrypt(pt);

}

