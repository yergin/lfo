#pragma once

#include "PotController.h"
#include "OledDisplay.h"
#include "MidiController.h"
#include "WaveTable.h"

#define USB_SERIAL_LOGGING 1

enum class PwmOut
{
  L1 = 0,
  R1,
  L2,
  R2,
  L3,
  R3,
  V1,
  V2,
  V3,
  Clean,
  Count
};

inline constexpr int PwmOutCount = (int)PwmOut::Count;

struct Pwm
{
  uint8_t pin;
  uint8_t channel;
  timer_dev* timer;
};

inline Pwm Pwms[PwmOutCount] =
{
  {PA0, 0, nullptr},
  {PA1, 0, nullptr},
  {PA2, 0, nullptr},
  {PA3, 0, nullptr},
  {PA6, 0, nullptr},
  {PA7, 0, nullptr},
  {PA8, 0, nullptr},
  {PA9, 0, nullptr},
  {PA10, 0, nullptr},
  {PB9, 0, nullptr},
};

constexpr int PinEnvelope = PB0;
constexpr int PinRate = PB1;
constexpr int PinTremolo = PA4;
constexpr int PinVibrato = PA5;

constexpr int PinDisplayScl = PB6;
constexpr int PinDisplaySda = PB7;

constexpr int PinStatusLed = PC13;

constexpr uint16 Resolution = 4096;
constexpr int DownSample = 35;
constexpr float SampleRate = static_cast<float>(F_CPU) / Resolution / DownSample; // ~70kHz for 10-bit resolution without division

#if USB_SERIAL_LOGGING
inline USBCompositeSerial CompositeSerial;
#endif

inline PotController RatePit(PinRate);
inline PotController TremoloPot(PinTremolo);
inline PotController VibratoPot(PinVibrato);
inline OledDisplay Display(PinDisplayScl, PinDisplaySda);
inline MidiController midi;
inline WaveTable<9> Lfo(SampleRate, 1);
