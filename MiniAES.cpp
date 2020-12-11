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

std::unordered_map<uint8_t, uint8_t> MiniAES::inverse_s_box = inverse_map(s_box);

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

matrix MiniAES::uint16ToMatrix(uint16_t uint) {
    /*
    converts 16 bit uint16_t to 2x2 byte array using bit manipulation
    below is the index mapping the one on the paper
    [0][0]: 0, [1][0]: 1, [0][1]: 2, [1][1]: 3
    */
    matrix res;
    res[0][0] = uint >> 12;
    res[1][0] = (uint >> 8) & 15;
    res[0][1] = (uint >> 4) & 15;
    res[1][1] = uint & 15;

    return res;
}

uint16_t MiniAES::matrixToUInt16(matrix block) {
    uint16_t uint;
    for (auto j = 0; j < 2; j++) {
        for (auto i = 0; i < 2; i++) {
            uint <<= 4;
            uint |= block[i][j];
        }
    }
    return uint;
}


matrix MiniAES::nibbleSub (matrix block, bool inverse = false) {
    auto lookup = inverse ? inverse_s_box : s_box;
    for (auto i = 0; i < block.max_size(); i++) {
        for (auto j = 0; j < block[i].max_size(); j++)
            block[i][j] = lookup[block[i][j]];
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

std::vector< uint16_t > MiniAES::encrypt(std::vector< uint16_t > pt) {
    // encrypt from bitset vector to bitset vector
    std::vector < uint16_t > ct;

    for (auto i = 0; i < pt.size(); i++) {
        auto block = uint16ToMatrix(pt[i]);

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
        ct.push_back(matrixToUInt16(block));
    }

    return ct;
}

std::vector< uint16_t > MiniAES::decrypt(std::vector< uint16_t > ct) {
    // decrypt from bitset vector to bitset vector
    std::vector < uint16_t > pt;

    for (auto i = 0; i < ct.size(); i++) {
        auto block = uint16ToMatrix(ct[i]);

        block = keyAddition(block, rkey[2]);
        block = shiftRow(block);
        block = nibbleSub(block, true);

        block = keyAddition(block, rkey[1]);
        block = mixColumn(block);
        block = shiftRow(block);
        block = nibbleSub(block, true);

        block = keyAddition(block, rkey[0]);

        pt.push_back(matrixToUInt16(block));
    }

    return pt;
}
 
