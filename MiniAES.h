#include <array>
#include <string>
#include <unordered_map>

// 2 x 2 uint8_t array
typedef std::array< std::array<uint8_t, 2>, 2 > matrix;

class MiniAES {
    private:
        std::string key;
    public:
        static std::unordered_map<uint8_t, uint8_t> s_box;      // S-box implementation using hash map
        MiniAES() {};
        MiniAES(std::string k) : key(k) {};
        void setKey (std::string k) {
            key = k;
        };
        matrix blockToArray(std::string);
        matrix nibbleSub(matrix);
        matrix shiftRow(matrix);
        matrix mixColumn(matrix);
        matrix keyAddition(matrix, matrix);
        std::string encrypt(std::string);
};

uint8_t gadd(uint8_t, uint8_t);
uint8_t gmul(uint8_t, uint8_t);