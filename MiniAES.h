#ifndef MINIAES_H
#define MINIAES_H

#include <array>
#include <string>
#include <unordered_map>
#include <bitset>
#include <vector>

// 2 x 2 uint8_t array
typedef std::array< std::array<uint8_t, 2>, 2 > matrix;

class MiniAES {
    private:
        static std::unordered_map<uint8_t, uint8_t> s_box; // S-box implementation using hash map
        static std::unordered_map<uint8_t, uint8_t> inverse_s_box; // inverse S-box
        uint16_t key;
        std::array< matrix, 3 > rkey;
        matrix strToMatrix(std::string);
        std::string matrixToStr(matrix);
        matrix bitsetToMatrix(std::bitset<16>);
        std::bitset<16> matrixToBitset(matrix);
        matrix nibbleSub(matrix, bool);
        matrix shiftRow(matrix);
        matrix mixColumn(matrix);
        matrix keyAddition(matrix, matrix);
    public:
        MiniAES() {};
        MiniAES(uint16_t k);
        void setKey (uint16_t k);
        std::string encrypt(std::string);
        std::vector< std::bitset<16> > encrypt(std::vector< std::bitset<16> >);
        std::vector< std::bitset<16> > decrypt(std::vector< std::bitset<16> >);
};

uint8_t gadd(uint8_t, uint8_t);
uint8_t gmul(uint8_t, uint8_t);

template<typename K, typename V>
std::unordered_map<V,K> inverse_map(std::unordered_map<K,V> &map)
{
	std::unordered_map<V,K> inverse;
	for (const auto &p: map) {
		inverse.insert(std::make_pair(p.second, p.first));
	}
	return inverse;
}

#endif