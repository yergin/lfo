#pragma once

#include <cinttypes>

class WaveTable {
public:
  WaveTable(uint32_t sampleRate, float frequency)
  {
    _sampleRate = sampleRate;
    setFrequency(frequency);
  }

  void setFrequency(float freq)
  {
    _frequency = freq;
    _phaseDelta = phaseDelta();
    _frequencyRamp = 0;
    _rampSamples = 0;
  }

  void rampFrequency(float freq, uint32_t samples)
  {
    if (samples == 0)
    {
      setFrequency(freq);
      return;
    }
    _frequency = freq;
    _frequencyRamp = (phaseDelta() - _phaseDelta) / samples;
    _rampSamples = samples;
  }

  float frequency() const { return _frequency; }

  float sample() const
  {
    return static_cast<float>(SineTable[(int)_phase]) * Scaler - 1;
  }

  float sampleIP() const
  {
    int index1 = (int)_phase;
    int index2 = (index1 + 1) % TableSize;
    float alpha = _phase - index1;
    return (static_cast<float>(SineTable[index1]) * (1 - alpha) + static_cast<float>(SineTable[index2]) * alpha) * Scaler - 1;
  }

  void advance()
  {
    _phase += _phaseDelta;
    if (_rampSamples > 0)
    {
      _phaseDelta += _frequencyRamp;
      _rampSamples--;
    }
    while (_phase >= TableSize)
    {
      _phase -= TableSize;
    }
  }
  
private:
  inline float phaseDelta() const { return _frequency * TableSize / _sampleRate; }

  static constexpr int BitDepth = 12;
  static constexpr float Scaler = 2.f / (1 << BitDepth);
  static constexpr int TableSize = 120;
  inline static const uint16_t SineTable[TableSize] = {
    0x0, 0x2, 0xb, 0x19, 0x2c, 0x45, 0x64, 0x87, 0xb1, 0xdf,
    0x112, 0x14a, 0x187, 0x1c8, 0x20d, 0x257, 0x2a5, 0x2f6, 0x34c, 0x3a4,
    0x3ff, 0x45d, 0x4be, 0x521, 0x586, 0x5ed, 0x655, 0x6bf, 0x729, 0x794,
    0x7ff, 0x86a, 0x8d5, 0x93f, 0x9a9, 0xa11, 0xa78, 0xadd, 0xb40, 0xba1,
    0xbff, 0xc5a, 0xcb2, 0xd08, 0xd59, 0xda7, 0xdf1, 0xe36, 0xe77, 0xeb4,
    0xeec, 0xf1f, 0xf4d, 0xf77, 0xf9a, 0xfb9, 0xfd2, 0xfe5, 0xff3, 0xffc,
    0xfff, 0xffc, 0xff3, 0xfe5, 0xfd2, 0xfb9, 0xf9a, 0xf77, 0xf4d, 0xf1f,
    0xeec, 0xeb4, 0xe77, 0xe36, 0xdf1, 0xda7, 0xd59, 0xd08, 0xcb2, 0xc5a,
    0xbff, 0xba1, 0xb40, 0xadd, 0xa78, 0xa11, 0x9a9, 0x93f, 0x8d5, 0x86a,
    0x7ff, 0x794, 0x729, 0x6bf, 0x655, 0x5ed, 0x586, 0x521, 0x4be, 0x45d,
    0x3ff, 0x3a4, 0x34c, 0x2f6, 0x2a5, 0x257, 0x20d, 0x1c8, 0x187, 0x14a,
    0x112, 0xdf, 0xb1, 0x87, 0x64, 0x45, 0x2c, 0x19, 0xb, 0x2,
  };

  uint32_t _sampleRate;
  float _frequency = 440.0f;
  uint32_t _rampSamples = 0;
  float _phase = 0.0f;
  float _phaseDelta;
  float _frequencyRamp = 0;
};