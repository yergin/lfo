/**
 * @file gen_sine.cpp
 * @author Gino Bollaert
 * @brief Sine wave table generator
 * @details
 * @date 2023-05-11
 * @copyright Gino Bollaert. All rights reserved.
 */

#include <cmath>
#include <iostream>
#include <string>

int main()
{
    static constexpr int BitDepth = 16;
    static constexpr int TableOrder = 8;
    static constexpr int TableSize = 1 << TableOrder;
    static constexpr int MaxTableValue = (1 << BitDepth) - 1;
    std::cout << "Sine wave:\n";
    for (int i = 0; i < TableSize;)
    {
        std::string separator = "";
        for (int c = 0; c < 8; c++)
        {
            auto phase = static_cast<double>(i) / TableSize;
            auto val = (1 - cos(phase * 2 * M_PI)) / 2;
            val = std::min(std::max(0., val), 1.);
            auto code = static_cast<int32_t>(val * MaxTableValue + 0.5f);
            code = std::min(std::max(0, code), MaxTableValue);
            std::cout << separator << "0x" << std::hex << code;
            separator = ", ";
            i++;
        }
        std::cout << ",\n";
    }
    return 0;
}