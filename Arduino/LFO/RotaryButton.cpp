#include "RotaryButton.h"

#include <limits.h>

RotaryButton* RotaryButton::_rotary = nullptr;

void RotaryButton::pinAInterrupt() {
  _rotary->onPinChange((digitalRead(_rotary->_pinA) ? 1 : 0) + (_rotary->_pinState & 2));
}

void RotaryButton::pinBInterrupt() {
  _rotary->onPinChange((digitalRead(_rotary->_pinB) ? 2 : 0) + (_rotary->_pinState & 1));
}

void RotaryButton::pinInterrupt() {
  noInterrupts();
  while (_globalInterruptBusy);
  _globalInterruptBusy = 1;
  _rotary->onPinChange((digitalRead(_rotary->_pinA) ? 1 : 0) + (digitalRead(_rotary->_pinB) ? 2 : 0));
  _globalInterruptBusy = 0;
  interrupts();
}

RotaryButton::RotaryButton(int pinButton, int pinA, int pinB, WiringPinMode mode, bool useInterrupts)
: _pinButton(pinButton)
, _pinA(pinA)
, _pinB(pinB) {
  attach(pinButton, mode);
  pinMode(pinA, mode);
  pinMode(pinB, mode);

  _state._counterMinimum = INT_MIN;
  _state._counterMaximum = INT_MAX;
  _state._counterWrap = true;
  resetCounter();
  
  if (useInterrupts) {
    _rotary = this;
    attachInterrupt(digitalPinToInterrupt(pinA), RotaryButton::pinInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pinB), RotaryButton::pinInterrupt, CHANGE);
  }
}

uint8_t RotaryButton::readPinState() {
  return ((digitalRead(_pinA) ? 1 : 0)) + ((digitalRead(_pinB) ? 2 : 0));
}

void RotaryButton::resetCounter(long value) {
  if (_rotary == this) {
    noInterrupts();
    _delta = 0;
    interrupts();
  }
  else {
    _pinState = readPinState();
    _delta = 0;
  }
  _state._counter = value < _state._counterMinimum ? _state._counterMinimum :
                    (value > _state._counterMaximum ? _state._counterMaximum : value);
  _wasTurned = false;
}

void RotaryButton::setCounterRange(long min, long max, bool wrap) {
  _state._counter = _state._counter < min ? min : (_state._counter > max ? max : _state._counter);
  _state._counterMinimum = min;
  _state._counterMaximum = max >= min ? max : min;
  _state._counterWrap = wrap;
}

void RotaryButton::setResponder(Responder* responder) {
  _responder = responder;
}

bool RotaryButton::update() {
  int8_t delta = 0;
  if (this == _rotary) {
    noInterrupts();
    while (_interruptBusy);
    delta = _delta / 2;
    _delta = _delta - (delta * 2);
    interrupts();
  }
  else {
    onPinChange(readPinState());
    delta = _delta / 2;
    _delta = _delta - (delta * 2);
  }

  _wasTurned = delta != 0;
  
  while (delta > 0) {
    if (_state._counter < _state._counterMaximum) {
      _state._counter++;
    }
    else {
      _state._counter = _state._counterWrap ? _state._counterMinimum : _state._counterMaximum;
    }
    delta--;
  }
  
  while (delta < 0) {
    if (_state._counter > _state._counterMinimum) {
      _state._counter--;
    }
    else {
      _state._counter = _state._counterWrap ? _state._counterMaximum : _state._counterMinimum;
    }
    delta++;
  }

  if (_wasTurned && _responder) {
    _responder->onChange(_state._counter);
  }
  
  return Button::update() || _wasTurned;
}

void RotaryButton::onPinChange(uint8_t newPinState) {
  static const int8_t kTrasitionTable[4][4] = { { 0, -1, 1, 0 },
                                                { 1, 0, 0, -1 },
                                                { -1, 0, 0, 1 },
                                                { 0, 1, -1, 0 } };
  _delta += kTrasitionTable[_pinState][newPinState];
  _pinState = newPinState;
}
