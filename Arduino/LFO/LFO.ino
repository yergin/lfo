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

void handleControlChange(unsigned int channel, unsigned int controller, unsigned int value)
{
  if (channel != 0)
  {
    return;
  }
  setMidiStatus(MidiStatus::Receiving);
  handleControlValue((MidiCC)controller, value);
}

void handleProgramChange(unsigned int channel, unsigned int program)
{
  setMidiStatus(MidiStatus::Receiving);
#if USB_SERIAL_LOGGING
  String s = String() + "Received Program Change " + String(program) + " [Ch:" + String(channel + 1) + "]\n";
  CompositeSerial.write(s.c_str());
#endif
}

void handleSysExData(unsigned char data)
{
  setMidiStatus(MidiStatus::Receiving);
#if USB_SERIAL_LOGGING
  String s = String() + "Received SysEx byte: 0x" + String(data, 16) + "\n";
  CompositeSerial.write(s.c_str());
#endif
}

void handleSysExEnd()
{
  setMidiStatus(MidiStatus::Receiving);
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
  midi.setControlChangeCallback(handleControlChange);
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

void updateBypass()
{
  digitalWrite(PinBypass, !state.bypass);
}

void updateVoiceMode()
{
  digitalWrite(PinVoice, state.voiceMode == VoiceMode::Chorus);
}

void updateVibratoDepth()
{
  for (int n = (int)PwmOut::V1; n < (int)PwmOut::V3; n++)
  {
    state.oscMul[n] = state.vibratoDepth;
    state.oscOffset[n] = 0xffff - state.oscMul[n];
  }
}

void updateLevelsAndTremoloDepth()
{
  uint32_t v = (state.volume * state.expression) >> 16;
  for (int n = (int)PwmOut::L1; n < (int)PwmOut::R3; n++)
  {
    state.oscMul[n] = (state.tremoloDepth * v) >> 16;
    state.oscOffset[n] = (v - state.oscMul[n]) >> 1;
  }
}

void updateDryLevel()
{
  uint32_t v = (state.volume * state.expression) >> 16;
  state.dryMul = (v * state.dryLevel) >> 16;
}

void updateLfoRate()
{
  lfo.rampFrequency(state.rate, state.rampTimeMs);
}

void updateLfoPhases()
{
  lfo.rampPhaseOffset(state.syncDelta - state.stereoDelta, state.rampTimeMs, (int)PwmOut::L1);
  lfo.rampPhaseOffset(state.syncDelta + state.stereoDelta, state.rampTimeMs, (int)PwmOut::R1);
  lfo.rampPhaseOffset(PhaseOffset2 + state.syncDelta - state.stereoDelta, state.rampTimeMs, (int)PwmOut::L2);
  lfo.rampPhaseOffset(PhaseOffset2 + state.syncDelta + state.stereoDelta, state.rampTimeMs, (int)PwmOut::R2);
  lfo.rampPhaseOffset(PhaseOffset3 + state.syncDelta - state.stereoDelta, state.rampTimeMs, (int)PwmOut::L3);
  lfo.rampPhaseOffset(PhaseOffset3 + state.syncDelta + state.stereoDelta, state.rampTimeMs, (int)PwmOut::R3);
}

void updateRampTime()
{
  updateLfoRate();
  updateLfoPhases();
}

void initState()
{
  updateBypass();
  handleControlValue(MidiCC::Rate, 24);
  handleControlValue(MidiCC::RampTime, 75);
  handleControlValue(MidiCC::Volume, 100);
  handleControlValue(MidiCC::Expression, 100);
  handleControlValue(MidiCC::VoiceMode, 0);
  handleControlValue(MidiCC::AutopanWidth, 32);
  handleControlValue(MidiCC::Tremolo, 127);
  handleControlValue(MidiCC::Vibrato, 127);
  handleControlValue(MidiCC::RotaryPhase, 0);
  handleControlValue(MidiCC::Phaser, 0);
  lfo.setPhaseOffset(0, (int)PwmOut::V1);
  lfo.setPhaseOffset(PhaseOffset2, (int)PwmOut::V2);
  lfo.setPhaseOffset(PhaseOffset3, (int)PwmOut::V3);
  lfo.setFrequency(lfo.frequency());
  for (int n = 0; n < OscCount; n++)
  {
    lfo.setPhaseOffset(lfo.phaseOffset(n), n);
  }
}

void setup()
{
  pinMode(PinStatusLed, OUTPUT);
  pinMode(PinVoice, OUTPUT);
  pinMode(PinBypass, OUTPUT);

  Serial3.begin(31250);

  initState();
  setupPwms();

  delay(200);
  setupUsb();

#if OLED_DISPLAY
  display.init();
  display.clear();
  display.drawString(0, 0, "MIDI Controller");
  display.show();
  displayRealtimeChanges = true;
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

/*
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

void sendPot2Value()
{
  sendControlChange(channel, pot2Control, VibratoPot.value());
  float v = static_cast<float>(VibratoPot.value()) / 127;
  v = pow(2, v);
  Lfo.rampFrequency(v * 880, 1000);
}
*/

void setRate(int val)
{
  float rate = val * 3.f / 127;
  state.rate = rate + rate * rate * rate;
  updateLfoRate();
#if OLED_DISPLAY
  if (!displayRealtimeChanges)
  {
    return;
  }
  String oledStr;
  oledStr += String((int)(state.rate * 60)) + " rpm";
  display.clear();
  display.drawString(0, 0, "Speed:");
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

void setRampTime(int val)
{
  state.rampTimeMs = ((1001 * val) >> 7) + ((4071 * val * val) >> 14);
  updateRampTime();
#if OLED_DISPLAY
  if (!displayRealtimeChanges)
  {
    return;
  }
  String oledStr;
  oledStr += String(state.rampTimeMs) + " ms";
  display.clear();
  display.drawString(0, 0, "Ramp Time:");
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

uint16_t volume(int val)
{
  return ((val * val) << 1) + val * 262;
}

void setVolume(int val)
{
  state.volume = volume(val);
  updateLevelsAndTremoloDepth();
  updateDryLevel();
#if OLED_DISPLAY
  if (!displayRealtimeChanges)
  {
    return;
  }
  String oledStr;
  oledStr += String(val);
  display.clear();
  display.drawString(0, 0, "Volume:");
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

void setExpression(int val)
{
  state.expression = volume(val);
  updateLevelsAndTremoloDepth();
  updateDryLevel();
#if OLED_DISPLAY
  if (!displayRealtimeChanges)
  {
    return;
  }
  String oledStr;
  oledStr += String(val);
  display.clear();
  display.drawString(0, 0, "Expression:");
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

void setVoiceMode(int val)
{
  state.voiceMode = val == 0 ? VoiceMode::Vibrato : VoiceMode::Chorus;
  updateVoiceMode();
#if OLED_DISPLAY
  if (!displayRealtimeChanges)
  {
    return;
  }
  display.clear();
  display.drawString(0, 0, "Mode:");
  display.drawString(0, 16, state.voiceMode == VoiceMode::Vibrato ? "Vibrato" : "Chorus");
  display.show();
#endif
}

void setAutopanWidth(int val)
{
  state.stereoDelta = val * 0x8000 / 128;
  state.stereoDelta = state.stereoDelta << 16 + state.stereoDelta;
  updateLfoPhases();
#if OLED_DISPLAY
  if (!displayRealtimeChanges)
  {
    return;
  }
  String oledStr;
  int p = val * 200 / 128;
  oledStr += String(p > 100 ? p - 200 : p) + "%";
  display.clear();
  display.drawString(0, 0, "Auto-pan Width:");
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

void setRotaryPhase(int val)
{
  state.syncDelta = val * 0x10000 / 128;
  state.syncDelta = state.syncDelta << 16 + state.syncDelta;
  updateLfoPhases();
#if OLED_DISPLAY
  if (!displayRealtimeChanges)
  {
    return;
  }
  String oledStr;
  int p = val * 200 / 128;
  oledStr += String(p > 100 ? p - 200 : p) + "%";
  display.clear();
  display.drawString(0, 0, "Rotary Phase:");
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

void setTremolo(int val)
{
  state.tremoloDepth = (val << 9) + (val << 2);
  updateLevelsAndTremoloDepth();
#if OLED_DISPLAY
  if (!displayRealtimeChanges)
  {
    return;
  }
  String oledStr;
  oledStr += String(val * 100 / 127) + "%";
  display.clear();
  display.drawString(0, 0, "Tremolo/Auto-pan:");
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

void setVibrato(int val)
{
  state.vibratoDepth = (val << 9) + (val << 2);
  updateVibratoDepth();
#if OLED_DISPLAY
  if (!displayRealtimeChanges)
  {
    return;
  }
  String oledStr;
  oledStr += String(val * 100 / 127) + "%";
  display.clear();
  display.drawString(0, 0, "Vibrato/Chorus:");
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

void setPhaser(int val)
{
  state.dryLevel = (val << 9) + (val << 2);
  updateDryLevel();
#if OLED_DISPLAY
  if (!displayRealtimeChanges)
  {
    return;
  }
  String oledStr;
  oledStr += String(val * 100 / 127) + "%";
  display.clear();
  display.drawString(0, 0, "Phaser:");
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

void handleControlValue(MidiCC cc, int val)
{
  switch (cc)
  {
    case MidiCC::Rate: setRate(val); break;
    case MidiCC::RampTime: setRampTime(val); break;
    case MidiCC::Volume: setVolume(val); break;
    case MidiCC::Expression: setExpression(val); break;
    case MidiCC::VoiceMode: setVoiceMode(val); break;
    case MidiCC::AutopanWidth: setAutopanWidth(val); break;
    case MidiCC::Tremolo: setTremolo(val); break;
    case MidiCC::Vibrato: setVibrato(val); break;
    case MidiCC::RotaryPhase: setRotaryPhase(val); break;
    case MidiCC::Phaser: setPhaser(val); break;
  }
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
  midi.poll();
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
