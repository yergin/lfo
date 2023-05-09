#pragma once

#include "DisplayInterface.h"
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

class OledDisplay : public DisplayInterface {
public:
  OledDisplay(int pinClock, int pinData)
  : _display(U8G2_R0, U8X8_PIN_NONE, pinClock, pinData) {}

  void init() {
    _display.begin();
    _display.setFontMode(1);
    _display.setFont(u8g2_font_crox1h_tf);
    _display.clearBuffer();    
  }
  
  void clear() override { _display.clear(); }
  void show() override {
    _display.sendBuffer();
    _display.clearBuffer();
  }
  int width() override { return _display.getDisplayWidth(); }
  int height() override { return _display.getDisplayHeight(); }
  void requestFontHeight(int hgt) override { _display.setFont(u8g2_font_crox1h_tf); }
  int fontHeight() override { return _display.getMaxCharHeight(); }
  void drawString(int x, int y, String s, Align align = Align::kLeft) override {
    _display.drawStr(align == Align::kLeft ? x : x - _display.getStrWidth(s.c_str()), y + fontHeight(), s.c_str());
  }

private:
  U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C _display;
};
