#pragma once

#include <Arduino.h>
#undef min
#undef max

class Apa102Port
{
public:
  Apa102Port(gpio_reg_map& regMap);
  void configureStrip(int strip, uint8_t dataPinOffset, uint8_t clockPinOffset);
  bool writeStrip(int strip, uint8_t* data, uint16_t len);
  bool isStripReady(int strip) const { return _dataLen[strip] == 0 && isStripConfigured(strip); }

  bool update();
  bool needsUpdating() const { return _clockMask != 0; }
  
private:
  void configurePin(int pin);
  bool isStripConfigured(int strip) const { return _dataPin[strip] != kStripNotConfigured; }
  bool setStripData(int strip, uint8_t* data, uint16_t len);
  void writeStrips4();
  void writeStrips8();

  static constexpr int kMaxStripsPerPort = 8;
  static constexpr int kStripNotConfigured = 32;

  gpio_reg_map& _regMap;
  uint8_t _dataPin[kMaxStripsPerPort] = {};
  uint8_t _clockPin[kMaxStripsPerPort] = {};
  uint32_t _dataMask[kMaxStripsPerPort] = {};
  uint8_t _dataRotate[kMaxStripsPerPort] = {};
  uint16_t _clockMask = 0;
  uint8_t* _data[kMaxStripsPerPort];
  uint16_t _dataLen[kMaxStripsPerPort] = {};
  bool _moreThan4Strips = false;
  static uint8_t _zeroData;
};
