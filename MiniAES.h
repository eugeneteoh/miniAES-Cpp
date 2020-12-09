#include <array>
#include <string>
#include <unordered_map>

// 2 x 2 uint8_t array
typedef std::array< std::array<uint8_t, 2>, 2 > matrix;

class MiniAES {
    private:
        static std::unordered_map<uint8_t, uint8_t> s_box;      // S-box implementation using hash map
        uint16_t key;
        std::array< matrix, 3 > rkey;
        matrix blockToArray(std::string);
        matrix nibbleSub(matrix);
        matrix shiftRow(matrix);
        matrix mixColumn(matrix);
        matrix keyAddition(matrix, matrix);
        matrix keySchedule(matrix, uint8_t);
    public:
        MiniAES() {};
        MiniAES(uint16_t k);
        void setKey (uint16_t k);
        std::string encrypt(std::string);
};

uint8_t gadd(uint8_t, uint8_t);
uint8_t gmul(uint8_t, uint8_t);