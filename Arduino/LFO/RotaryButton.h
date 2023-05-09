#pragma once

#include <Yabl.h>
#undef min
#undef max

class RotaryButton : public Button {
public:
  class Responder {
    public:
      virtual void onChange(long counter) {}
  };

  struct State {
    long _counter;
    long _counterMinimum;
    long _counterMaximum;
    bool _counterWrap;
  };

  RotaryButton(int pinButton, int pinA, int pinB, WiringPinMode mode = INPUT_PULLUP, bool useInterrupts = true);

  void setResponder(Responder* responder);
  bool update();
  bool wasTurned() const { return _wasTurned; }
  void resetCounter(long value = 0);
  void setCounterRange(long min, long max, bool wrap = false);
  long counter() const { return _state._counter; }
  long counterMinimum() const { return _state._counterMinimum; }
  long counterMaximum() const { return _state._counterMaximum; }
  bool counterWrap() const { return _state._counterWrap; }
  void saveState(State& state) const { state = _state; }
  void restoreState(State& state) { _state = state; }
  
private:
  static void pinAInterrupt();
  static void pinBInterrupt();
  static void pinInterrupt();

  void onTurn();
  void onPinChange(uint8_t newPinState);
  uint8_t readPinState();
  
  static RotaryButton* _rotary;
  inline static volatile uint8_t _globalInterruptBusy = 0;

  int _pinButton;
  int _pinA;
  int _pinB;
  Responder* _responder = nullptr;
  uint8_t _pinState;
  int8_t _delta;
  volatile uint8_t _interruptBusy = 0;
  State _state;
  bool _wasTurned = false;
};
