/**
 * @file curves.cpp
 * @author Gino Bollaert
 * @brief MIDI CC curve preview
 * @details
 * @date 2023-05-12
 * @copyright Gino Bollaert. All rights reserved.
 */

#include <iostream>
#include <cmath>

int main()
{
    std::cout << "Rate:\n";
    for (int val = 0; val < 128; val++)
    {
        float rate = val * 3.f / 127;
        rate = rate + rate * rate * rate;
        std::cout << "  " << val << ":\t" << rate << "Hz = " << int(rate * 60 + 0.5f) << "RPM\n";
    }
    std::cout << std::dec << '\n';

    std::cout << "Ramp Time:\n";
    for (int val = 0; val < 128; val++)
    {
        uint32_t ms = ((1001 * val) >> 7) + ((4071 * val * val) >> 14);
        std::cout << "  " << val << ":\t" << ms << " ms\n";
    }
    std::cout << std::dec << '\n';

    std::cout << "Volume / Expression:\n";
    for (int val = 0; val < 128; val++)
    {
        uint16_t v = ((val * val) << 1) + val * 262;
        float f = float(v) / 0xffff;
        float d = 20 * log10f(f);
        std::cout << "  " << std::dec << val << ":\t" << d << "dB = " << f << "\t(" << std::hex << v << ")\n";
    }
    std::cout << std::dec << '\n';

    std::cout << "Autopan Width:\n";
    for (int val = 0; val < 128; val++)
    {
        int p = val * 200 / 128;
        std::cout << "  " << val << ":\t" << (p > 100 ? p - 200 : p) << "%\n";
    }
    std::cout << std::dec << '\n';
}