/**
 * @file LFO.ino
 * @author Gino Bollaert
 * @brief Main Arduino code file
 * @details
 * @date 2023-05-11
 * @copyright Gino Bollaert. All rights reserved.
 */

#include "Globals.h"

const char* kNotes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

int channel = 0;
int pot1Control = 81;
int pot2Control = 82;

// volatile float freq = 1;

void setMidiStatus(MidiStatus status)
{
  midiIndicatorChanged = millis();
  midiIndicator = status;
}

void updateMidiStatus()
{
  constexpr uint32_t MidiIndicatorStrobe = 200;
  if (millis() >= midiIndicatorChanged + MidiIndicatorStrobe)
  {
    setMidiStatus(MidiStatus::Idle);
  }
  digitalWrite(PinStatusLed, midiIndicator == MidiStatus::Idle ? HIGH : LOW);
}

void handleProgramChange(unsigned int channel, unsigned int program)
{
  // setMidiStatus(kMidiReceiving);
#if USB_SERIAL_LOGGING
  String s = String() + "Received Program Change " + String(program) + " [Ch:" + String(channel + 1) + "]\n";
  CompositeSerial.write(s.c_str());
#endif
}

void handleSysExData(unsigned char data)
{
  // setMidiStatus(kMidiReceiving);
#if USB_SERIAL_LOGGING
  String s = String() + "Received SysEx byte: 0x" + String(data, 16) + "\n";
  CompositeSerial.write(s.c_str());
#endif
}

void handleSysExEnd()
{
  // setMidiStatus(kMidiReceiving);
#if USB_SERIAL_LOGGING
  String s = String() + "End of SysEx\n";
  CompositeSerial.write(s.c_str());
#endif
}

void setupUsb()
{
  USBComposite.clear();
  USBComposite.setManufacturerString("Tranzistor Bandicoot");
  USBComposite.setProductString("Chorus FX");
  USBComposite.setVendorId(0xc007);
  USBComposite.setProductId(0x1f0c);
#if USB_SERIAL_LOGGING
  CompositeSerial.registerComponent();
#else
  midi.registerComponent();
  midi.setProgramChangeCallback(handleProgramChange);
  midi.setSysExCallbacks(handleSysExData, handleSysExEnd);
#endif
  USBComposite.begin();
#if USB_SERIAL_LOGGING
  delay(2000);
  CompositeSerial.write("Chorus FX by Tranzistor Bandicoot\n");
#endif
}

void setupPwms()
{
  for (int i = 0; i < PwmOutCount; i++)
  {
    pinMode(Pwms[i].pin, PWM);
    Pwms[i].timer = PIN_MAP[Pwms[i].pin].timer_device;
    Pwms[i].channel = PIN_MAP[Pwms[i].pin].timer_channel;

    timer_pause(Pwms[i].timer);
    timer_set_prescaler(Pwms[i].timer, 0);
    timer_set_reload(Pwms[i].timer, PwmPrecision);
    timer_cc_enable(Pwms[i].timer, Pwms[i].channel);
    timer_generate_update(Pwms[i].timer);
    if (i == 0)
    {
      timer_attach_interrupt(Pwms[i].timer, TIMER_UPDATE_INTERRUPT, &TimerInterrupt);
    }
    timer_resume(Pwms[i].timer);
  }
}

void initState()
{
  for (int n = 0; n < OscCount; n++)
  {
    state.oscMul[n] = 0x7fff;
    state.oscOffset[n] = 0xffff - state.oscMul[n];
  }
}

void applyState()
{
  lfo.setFrequency(state.rate);
  lfo.setPhaseOffset(0, (int)PwmOut::V1);
  lfo.setPhaseOffset(PhaseOffset2, (int)PwmOut::V2);
  lfo.setPhaseOffset(PhaseOffset3, (int)PwmOut::V3);
  lfo.setPhaseOffset(state.syncDelta - state.stereoDelta, (int)PwmOut::L1);
  lfo.setPhaseOffset(state.syncDelta + state.stereoDelta, (int)PwmOut::R1);
  lfo.setPhaseOffset(PhaseOffset2 + state.syncDelta - state.stereoDelta, (int)PwmOut::L2);
  lfo.setPhaseOffset(PhaseOffset2 + state.syncDelta + state.stereoDelta, (int)PwmOut::R2);
  lfo.setPhaseOffset(PhaseOffset3 + state.syncDelta - state.stereoDelta, (int)PwmOut::L3);
  lfo.setPhaseOffset(PhaseOffset3 + state.syncDelta + state.stereoDelta, (int)PwmOut::R3);
}

void setup()
{
  pinMode(PinStatusLed, OUTPUT);
  digitalWrite(PinStatusLed, HIGH);

  Serial3.begin(31250);

  initState();
  applyState();
  setupPwms();

  delay(200);
  setupUsb();

#if OLED_DISPLAY
  display.init();
  display.clear();
  display.drawString(0, 0, "MIDI Controller");
  display.show();
#endif
}

void TimerInterrupt()
{
  static int counter = 0;
  if (counter % DownSample == 0)
  {
    lfo.advance();
    int n = 0;
    for (; n <= (int)PwmOut::V3; n++)
    {
      auto v = ((lfo.sampleIP(n) * state.oscMul[n]) >> 16) + state.oscOffset[n];
      timer_set_compare(Pwms[n].timer, Pwms[n].channel, v >> (16 - PwmBits));
    }
    n = (int)PwmOut::Dry;
    timer_set_compare(Pwms[n].timer, Pwms[n].channel, state.dryLevel >> (16 - PwmBits));
  }
  counter++;
}

String noteName(int pitch) { return String() + kNotes[pitch % 12] + String(pitch / 12 - 1); }

void sendNoteOn(int chan, int pitch, int vel)
{
#if not USB_SERIAL_LOGGING
  midi.sendNoteOn(chan, pitch, vel);
#endif
#if OLED_DISPLAY
  String s;
  s += "ch:" + String(chan + 1);
  s += " note:" + noteName(pitch);
  s += " vel:" + String(vel);
  display.clear();
  display.drawString(0, 0, "Note On");
  display.drawString(0, 16, s.c_str());
  display.show();
#endif
}

void sendControlChange(int chan, int ctl, int val)
{
#if not USB_SERIAL_LOGGING
  midi.sendControlChange(chan, ctl, val);
#endif
#if OLED_DISPLAY
  display.clear();
  String s;
  s += "ch:" + String(chan + 1);
  s += " ctl:" + String(ctl);
  s += " val:" + String(val);
  display.drawString(0, 0, "Control Change");
  display.drawString(0, 16, s.c_str());
  display.show();
#endif
}

void sendPot1Value()
{
  // sendControlChange(channel, pot1Control, TremoloPot.value());
}

/*
void sendPot2Value()
{
  sendControlChange(channel, pot2Control, VibratoPot.value());
  float v = static_cast<float>(VibratoPot.value()) / 127;
  v = pow(2, v);
  Lfo.rampFrequency(v * 880, 1000);
}
*/

void handleControlValue(MidiCC cc, int val)
{
#if OLED_DISPLAY
  String oledStr;
  oledStr += "CC:" + String((int)cc) + " Val:" + String(val);
  display.clear();
  display.drawString(0, 0, "Received MIDI:");
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

void receiveMidiByte(int byte)
{
  static bool expectCC = false;
  static bool expectValue = false;
  static MidiCC cc;

  if (byte & 0x80)
  {
    expectCC = ((byte & 0xf0) == 0xb0);
    expectValue = false;
    return;
  }

  if (expectCC)
  {
    setMidiStatus(MidiStatus::Receiving);
    cc = (MidiCC)byte;
    expectCC = false;
    expectValue = true;
    return;
  }

  if (expectValue)
  {
    handleControlValue(cc, byte);
    expectValue = false;
    expectCC = true;
  }
}

void loop()
{
  while (Serial3.available())
  {
    receiveMidiByte(Serial3.read());
  }
  updateMidiStatus();
  /*
  if (TremoloPot.update()) {
    sendPot1Value();
  }
  if (VibratoPot.update()) {
    sendPot2Value();
  }
  */
}
