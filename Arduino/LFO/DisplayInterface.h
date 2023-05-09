#pragma once

#include <Arduino.h>

enum class Align {
  kLeft = 0,
  kRight,
};

class DisplayInterface {
public:
  virtual void clear() = 0;
  virtual void show() = 0;
  virtual int width() = 0;
  virtual int height() = 0;
  virtual void requestFontHeight(int hgt) = 0;
  virtual int fontHeight() = 0;
  virtual void drawString(int x, int y, String s, Align align = Align::kLeft) = 0;
};
