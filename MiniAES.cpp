#include "MiniAES.h"
#include <iostream>
#include <string>
#include <array>
#include <unordered_map>
#include <vector>
#include <bitset>

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

MiniAES::MiniAES(uint16_t k) {
    setKey(k);
}

void MiniAES::setKey(uint16_t k) {
    key = k;

    // round key 0
    rkey[0][0][0] = key >> 12;
    rkey[0][1][0] = (key >> 8) & 15;
    rkey[0][0][1] = (key >> 4) & 15;
    rkey[0][1][1] = key & 15;

    // round key 1 and 2
    uint8_t rcon[2] = {0b0001, 0b0010};
    for (auto round = 1; round < 3; round++) {
        rkey[round][0][0] = gadd( gadd(rkey[round-1][0][0], s_box[rkey[round-1][1][1]]), rcon[round-1] );
        rkey[round][1][0] = gadd( rkey[round-1][1][0], rkey[round][0][0] );
        rkey[round][0][1] = gadd( rkey[round-1][0][1], rkey[round][1][0] );
        rkey[round][1][1] = gadd( rkey[round-1][1][1], rkey[round][0][1] );
    }
}

matrix MiniAES::strToMatrix(std::string s) {
    /* 
    converts 16 bit string to 2x2 nibble array
    below is the index mapping the one on the paper
    [0][0]: 0, [1][0]: 1, [0][1]: 2, [1][1]: 3
    uses masks to extract high/low nibble (bit shift, &)
    smallest data type in C++ is 1 byte (8 bit) so uint8_t gives the most efficient output
    */
    matrix vals;
    for (auto i = 0; i < 2; i++) {
        vals[0][i] = s[i] >> 4; // high nibble
        vals[1][i] = s[i] & 15; // low nibble
    }
    return vals;
}

std::string MiniAES::matrixToStr(matrix block) {
    std::string substr;
    for (auto i = 0; i < 2; i++) {
        substr += (block[0][i] << 4) | block[1][i];
    }
    return substr;
}

matrix MiniAES::bitsetToMatrix(std::bitset<16> block) {
    matrix res;
    res[0][0] = block.to_ulong() >> 12;
    res[1][0] = (block.to_ulong() >> 8) & 15;
    res[0][1] = (block.to_ulong() >> 4) & 15;
    res[1][1] = block.to_ulong() & 15;

    return res;
}

std::bitset<16> MiniAES::matrixToBitset(matrix block) {
    std::bitset<16> bitset;
    for (auto j = 0; j < 2; j++) {
        for (auto i = 0; i < 2; i++) {
            bitset <<= 4;
            bitset |= block[i][j];
        }
    }
    return bitset;
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

matrix MiniAES::keyAddition(matrix block, matrix curr_rkey) {
    for (auto i = 0; i < 2; i++)
        for (auto j = 0; j < 2; j++)
            block[i][j] = gadd(block[i][j], curr_rkey[i][j]);

    return block;
}

std::string MiniAES::encrypt (std::string pt) { 
    // encrypt from string to string
    std::string ct;
    // block cipher, each block has 16 bits / 2 bytes
    for (auto i = 0; i < pt.length(); i += 2) {
        auto block = strToMatrix(pt.substr(i, 2));

        // round 0
        block = keyAddition(block, rkey[0]);
        // round 1
        block = nibbleSub(block);
        block = shiftRow(block);
        block = mixColumn(block);
        block = keyAddition(block, rkey[1]);
        // round 2
        block = nibbleSub(block);
        block = shiftRow(block);
        block = keyAddition(block, rkey[2]);

        // add to ct
        ct += matrixToStr(block);
    }

    return ct;
}

std::vector< std::bitset<16> > MiniAES::encrypt(std::vector< std::bitset<16> > pt) {
    // encrypt from bitset vector to bitset vector
    std::vector < std::bitset<16> > ct;

    for (auto i = 0; i < pt.size(); i++) {
        auto block = bitsetToMatrix(pt[i]);

        // round 0
        block = keyAddition(block, rkey[0]);
        // round 1
        block = nibbleSub(block);
        block = shiftRow(block);
        block = mixColumn(block);
        block = keyAddition(block, rkey[1]);
        // round 2
        block = nibbleSub(block);
        block = shiftRow(block);
        block = keyAddition(block, rkey[2]);

        // add to ct
        ct.push_back(matrixToBitset(block));
    }

    return ct;
}

 
int main() {
    std::vector< std::bitset<16> > pt = {0b1111, 0b1001110001100011};
    uint16_t key = 0b1100001111110000; // secret key

    // for (unsigned i = 0; i < pt.length(); i += 2) {
    //     std::cout << pt.substr(i, 2) << std::endl;
    // }
    MiniAES test(key);
    auto test_encrypt = test.encrypt(pt);
    std::vector < std::string > test_str;
    for (auto i = 0; i < test_encrypt.size(); i++)
        test_str.push_back(test_encrypt[i].to_string());
}

