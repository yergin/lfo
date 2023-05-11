/**
 * @file WaveTableTest.cpp
 * @author Gino Bollaert
 * @brief WaveTable tests
 * @details
 * @date 2023-05-11
 * @copyright Gino Bollaert. All rights reserved.
 */

#include "WaveTable.h"
#include <gtest/gtest.h>

TEST(WaveTable, Ramp)
{
    constexpr float SampleRate = 500;
    WaveTable<1> lfo(SampleRate, 1);
    lfo.rampFrequency(0.1, 1000);
}