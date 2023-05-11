/**
 * @file WaveTable.h
 * @author Gino Bollaert
 * @brief 16-bit multiphase wavetable class
 * @details
 * @date 2023-05-11
 * @copyright Gino Bollaert. All rights reserved.
 */

#pragma once

#include <cinttypes>

template <int N> class WaveTable
{
public:
  WaveTable(uint32_t sampleRate, float frequency)
  {
    _sampleRate = sampleRate;
    setFrequency(frequency);
    for (int n = 0; n < N; n++)
    {
      _targetOffset[n] = 0;
      _phaseOffset[n] = 0;
      _phasePlusOffset[n] = 0;
      _offsetRamp[n] = 0;
    }
  }

  void setFrequency(float freq)
  {
    _frequency = freq;
    _targetDelta = _phaseDelta = phaseDelta();
    _rampSamples = 0;
  }

  void rampFrequency(float freq, uint32_t ms)
  {
    _frequency = freq;
    _targetDelta = phaseDelta();
    ramp(ms);
  }

  void setPhaseOffset(uint32_t offset, int n = 0)
  {
    _targetOffset[n] = _phaseOffset[n] = offset;
    _rampSamples = 0;
  }

  void rampPhaseOffset(float offset, uint32_t ms, int n = 0)
  {
    _targetOffset[n] = offset;
    ramp(ms);
  }

  void resetPhase(uint32_t phase = 0) { _phase = phase; }
  float frequency() const { return _frequency; }
  uint32_t phaseOffset(int n = 0) const { return _targetOffset[n]; }
  uint16_t sample(int n = 0) const { return SineTable[_phasePlusOffset[n] >> FractionBits]; }

  inline uint16_t sampleIP(int n = 0) const
  {
    uint32_t index0 = _phasePlusOffset[n] >> FractionBits;
    uint32_t index1 = (index0 + 1) % TableSize;
    uint32_t mul1 = (_phasePlusOffset[n] >> InterpolateShift) & InterpolateMask;
    uint32_t mul0 = InterpolateSum - mul1;
    return static_cast<uint16_t>((SineTable[index0] * mul0 + SineTable[index1] * mul1) >> InterpolateBits);
  }

  void advance()
  {
    if (_rampSamples > 0)
    {
      _phaseDelta += _frequencyRamp;
      for (int n = 0; n < N; n++)
      {
        _phaseOffset[n] += _offsetRamp[n];
      }
      _rampSamples--;
    }
    else
    {
      _phaseDelta = _targetDelta;
      for (int n = 0; n < N; n++)
      {
        _phaseOffset[n] = _targetOffset[n];
      }
    }
    _phase += _phaseDelta;
    for (int n = 0; n < N; n++)
    {
      _phasePlusOffset[n] = _phase + _phaseOffset[n];
    }
  }

private:
  static constexpr uint32_t IndexBits = 8;
  static constexpr uint32_t FractionBits = 32 - IndexBits;
  static constexpr uint32_t InterpolateBits = 16;
  static constexpr uint32_t InterpolateShift = FractionBits - InterpolateBits;
  static constexpr uint32_t InterpolateSum = (1 << InterpolateBits);
  static constexpr uint32_t InterpolateMask = InterpolateSum - 1;
  static constexpr uint32_t TableSize = 1 << IndexBits;
  /* clang-format off */
  inline static const uint16_t SineTable[TableSize] = {
    0x0, 0xa, 0x27, 0x59, 0x9e, 0xf6, 0x163, 0x1e2,
    0x276, 0x31c, 0x3d6, 0x4a3, 0x583, 0x676, 0x77b, 0x894,
    0x9be, 0xafb, 0xc4a, 0xdab, 0xf1d, 0x10a1, 0x1236, 0x13dc,
    0x1592, 0x1759, 0x1930, 0x1b17, 0x1d0e, 0x1f14, 0x2128, 0x234c,
    0x257d, 0x27bd, 0x2a0a, 0x2c65, 0x2ecc, 0x3140, 0x33c0, 0x364c,
    0x38e3, 0x3b85, 0x3e32, 0x40e8, 0x43a9, 0x4673, 0x4946, 0x4c21,
    0x4f04, 0x51ef, 0x54e0, 0x57d9, 0x5ad8, 0x5ddc, 0x60e6, 0x63f4,
    0x6707, 0x6a1e, 0x6d38, 0x7054, 0x7374, 0x7695, 0x79b8, 0x7cdb,
    0x7fff, 0x8324, 0x8647, 0x896a, 0x8c8b, 0x8fab, 0x92c7, 0x95e1,
    0x98f8, 0x9c0b, 0x9f19, 0xa223, 0xa527, 0xa826, 0xab1f, 0xae10,
    0xb0fb, 0xb3de, 0xb6b9, 0xb98c, 0xbc56, 0xbf17, 0xc1cd, 0xc47a,
    0xc71c, 0xc9b3, 0xcc3f, 0xcebf, 0xd133, 0xd39a, 0xd5f5, 0xd842,
    0xda82, 0xdcb3, 0xded7, 0xe0eb, 0xe2f1, 0xe4e8, 0xe6cf, 0xe8a6,
    0xea6d, 0xec23, 0xedc9, 0xef5e, 0xf0e2, 0xf254, 0xf3b5, 0xf504,
    0xf641, 0xf76b, 0xf884, 0xf989, 0xfa7c, 0xfb5c, 0xfc29, 0xfce3,
    0xfd89, 0xfe1d, 0xfe9c, 0xff09, 0xff61, 0xffa6, 0xffd8, 0xfff5,
    0xffff, 0xfff5, 0xffd8, 0xffa6, 0xff61, 0xff09, 0xfe9c, 0xfe1d,
    0xfd89, 0xfce3, 0xfc29, 0xfb5c, 0xfa7c, 0xf989, 0xf884, 0xf76b,
    0xf641, 0xf504, 0xf3b5, 0xf254, 0xf0e2, 0xef5e, 0xedc9, 0xec23,
    0xea6d, 0xe8a6, 0xe6cf, 0xe4e8, 0xe2f1, 0xe0eb, 0xded7, 0xdcb3,
    0xda82, 0xd842, 0xd5f5, 0xd39a, 0xd133, 0xcebf, 0xcc3f, 0xc9b3,
    0xc71c, 0xc47a, 0xc1cd, 0xbf17, 0xbc56, 0xb98c, 0xb6b9, 0xb3de,
    0xb0fb, 0xae10, 0xab1f, 0xa826, 0xa527, 0xa223, 0x9f19, 0x9c0b,
    0x98f8, 0x95e1, 0x92c7, 0x8fab, 0x8c8b, 0x896a, 0x8647, 0x8324,
    0x8000, 0x7cdb, 0x79b8, 0x7695, 0x7374, 0x7054, 0x6d38, 0x6a1e,
    0x6707, 0x63f4, 0x60e6, 0x5ddc, 0x5ad8, 0x57d9, 0x54e0, 0x51ef,
    0x4f04, 0x4c21, 0x4946, 0x4673, 0x43a9, 0x40e8, 0x3e32, 0x3b85,
    0x38e3, 0x364c, 0x33c0, 0x3140, 0x2ecc, 0x2c65, 0x2a0a, 0x27bd,
    0x257d, 0x234c, 0x2128, 0x1f14, 0x1d0e, 0x1b17, 0x1930, 0x1759,
    0x1592, 0x13dc, 0x1236, 0x10a1, 0xf1d, 0xdab, 0xc4a, 0xafb,
    0x9be, 0x894, 0x77b, 0x676, 0x583, 0x4a3, 0x3d6, 0x31c,
    0x276, 0x1e2, 0x163, 0xf6, 0x9e, 0x59, 0x27, 0xa,
  };
  /* clang-format on */

  inline uint32_t phaseDelta() const { return static_cast<uint32_t>((_frequency * 0x10000 / _sampleRate) * 0x10000); }
  inline uint32_t msToSamples(uint32_t ms) const { return ms * _sampleRate / 1000; }

  void ramp(uint32_t ms)
  {
    _rampSamples = static_cast<int32_t>(msToSamples(ms));
    if (_rampSamples == 0)
    {
      _phaseDelta = _targetDelta;
      for (int n = 0; n < N; n++)
      {
        _phaseOffset[n] = _targetOffset[n];
      }
      return;
    }
    if (_targetDelta >= _phaseDelta)
    {
      _frequencyRamp = static_cast<int32_t>(_targetDelta - _phaseDelta) / _rampSamples;
    }
    else
    {
      _frequencyRamp = -static_cast<int32_t>(_phaseDelta - _targetDelta) / _rampSamples;
    }
    for (int n = 0; n < N; n++)
    {
      if (_targetOffset[n] >= _phaseOffset[n])
      {
        _offsetRamp[n] = static_cast<int32_t>(_targetOffset[n] - _phaseOffset[n]) / _rampSamples;
      }
      else
      {
        _offsetRamp[n] = -static_cast<int32_t>(_phaseOffset[n] - _targetOffset[n]) / _rampSamples;
      }
    }
  }

  uint32_t _sampleRate = 0;
  float _frequency = 0;
  uint32_t _phase = 0;
  uint32_t _targetDelta = 0;
  uint32_t _phaseDelta = 0;
  uint32_t _targetOffset[N];
  uint32_t _phaseOffset[N];
  uint32_t _phasePlusOffset[N];
  int32_t _frequencyRamp = 0;
  int32_t _offsetRamp[N];
  int32_t _rampSamples = 0;
};
