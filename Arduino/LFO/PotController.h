#pragma once

#include <Arduino.h>


class PotController {
public:
  PotController(int pin, int halfSchmitt = 32)
  : _pin(pin)
  , _halfSchmitt(halfSchmitt) {
    pinMode(pin, INPUT_ANALOG);
    _pinReading = analogRead(pin);
    _value = _pinReading / 32;
  }

  bool update() {
     return update(analogRead(_pin), _pinReading - 16 - _halfSchmitt - _direction * _halfSchmitt, _pinReading + 16 + _halfSchmitt - _direction * _halfSchmitt);
  }

  bool update(int reading, int min, int max) {
    if (reading < min) {
      _pinReading = reading;
    }
    else if (reading > max) {
      _pinReading = reading;
    }
    return setValue(_pinReading / 32);
  }

  bool setValue(int value) {
    if (_value == value) {
      return false;
    }
    _direction = value > _value ? 1 : -1;
    _value = value;
    return true;
  }

  int value() const { return _value; }

private:
  int _pin;
  int _value = 0;
  int _pinReading = 0;
  int _direction = 0;
  int _halfSchmitt;
};
