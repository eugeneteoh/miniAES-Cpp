#include "MiniAES.h"
#include <iostream>

int main() {
    std::vector< uint16_t > pt = {0b1001110001100011, 0b1111, 0xffff};
    uint16_t key = 0b1100001111110000; // secret key

    MiniAES test(key);
    auto test_encrypt = test.encrypt(pt);
    auto test_decrypt = test.decrypt(test_encrypt);

    for (auto i = 0; i < test_encrypt.size(); i++) {
        std::cout << "pt" << i << ": " << pt[i] << std::endl;
    }
    std::cout << "key: " << key << std::endl;
    for (auto i = 0; i < test_encrypt.size(); i++) {
        std::cout << "test_encrypt" << i << ": " << test_encrypt[i] << std::endl;
        std::cout << "test_decrypt" << i << ": " << test_decrypt[i] << std::endl;
    }

}