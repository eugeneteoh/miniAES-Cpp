#include "MiniAES.h"
#include <iostream>

int main() {
    std::vector< uint16_t > pt = {0b1001110001100011};
    uint16_t key = 0b1100001111110000; // secret key

    MiniAES test(key);
    auto test_encrypt = test.encrypt(pt);
    auto test_decrypt = test.decrypt(test_encrypt);
    std::cout << "pt" << pt[0] << std::endl;
    std::cout << "key" << key << std::endl;
    std::cout << "test_encrypt: " << test_encrypt[0] << std::endl;
    std::cout << "test_decrypt: " << test_decrypt[0] << std::endl;
}

