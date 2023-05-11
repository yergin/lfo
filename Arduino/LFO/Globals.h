/**
 * @file Globals.h
 * @author Gino Bollaert
 * @brief Global variables and constants
 * @details
 * @date 2023-05-11
 * @copyright Gino Bollaert. All rights reserved.
 */

#pragma once

#include "PotController.h"
#include "OledDisplay.h"
#include "MidiController.h"
#include "WaveTable.h"

#define USB_SERIAL_LOGGING 1
#define OLED_DISPLAY 1

enum class MidiStatus
{
  Idle,
  Receiving,
  Sending,
};

enum class MidiCC : uint8_t
{
  Rate = 1,
  RampTime = 5,
  Volume = 7,
  Expression = 11,
  VoiceMode = 70,
  AutopanDirection = 91,
  Tremolo = 92,
  Vibrato = 93,
  RotaryPhase = 94,
  DryLevel = 95,
};

enum class VoiceMode : uint8_t
{
  Vibrato = 0,
  Chorus = 1,
};

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
  OscCount = V3,
  Dry,
  Count
};

inline constexpr int OscCount = (int)PwmOut::OscCount;
inline constexpr int PwmOutCount = (int)PwmOut::Count;

struct State
{
  float rate = 0.5f;
  uint32_t rampTimeMs = 3000;
  uint32_t stereoDelta = 0x40000000;
  uint32_t syncDelta = 0x0;
  uint16_t tremoloDepth = 0x7fff;
  uint16_t vibratoDepth = 0x7fff;
  uint16_t volume = 0x7fff;
  uint16_t expression = 0xffff;
  uint16_t dryLevel = 0x0;
  uint16_t dryMul = 0x0;
  VoiceMode voiceMode = VoiceMode::Vibrato;
  bool bypass = false;
  uint32_t oscMul[OscCount];
  uint32_t oscOffset[OscCount];
};

uint32_t PhaseOffset2 = 1431655765;
uint32_t PhaseOffset3 = 2863311531;

struct Pwm
{
  uint8_t pin;
  uint8_t channel;
  timer_dev* timer;
};

inline Pwm Pwms[PwmOutCount] = {
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

constexpr int PinVoice = PC15;
constexpr int PinBypass = PC14;

constexpr int PinMidiIn = B11;
constexpr int PinMidiOut = B10;

constexpr int PinDisplayScl = PB6;
constexpr int PinDisplaySda = PB7;

constexpr int PinStatusLed = PC13;

constexpr uint16 PwmBits = 12;
constexpr uint16 PwmPrecision = 1 << PwmBits;
constexpr uint16 PwmMax = PwmPrecision - 1;
constexpr int DownSample = 35;
constexpr float SampleRate = static_cast<float>(F_CPU) / PwmPrecision / DownSample;

#if USB_SERIAL_LOGGING
inline USBCompositeSerial CompositeSerial;
#endif

// inline PotController RatePit(PinRate);
// inline PotController TremoloPot(PinTremolo);
// inline PotController VibratoPot(PinVibrato);
#if OLED_DISPLAY
inline OledDisplay display(PinDisplayScl, PinDisplaySda);
#endif
// inline MidiController midi;
inline WaveTable<9> lfo(SampleRate, 1);
inline MidiStatus midiIndicator = MidiStatus::Idle;
inline uint32_t midiIndicatorChanged = 0;
inline State state = {};
