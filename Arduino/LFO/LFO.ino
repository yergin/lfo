/**
 * @file LFO.ino
 * @author Gino Bollaert
 * @brief Main Arduino code file
 * @details
 * @date 2023-05-11
 * @copyright Gino Bollaert. All rights reserved.
 */

#include "Globals.h"

const char* kNotes[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

int channel = 0;
int pot1Control = 81;
int pot2Control = 82;

volatile int counter = 0;
//volatile float freq = 1;

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
  //setMidiStatus(kMidiReceiving);
#if USB_SERIAL_LOGGING
  String s = String() + "Received Program Change " + String(program) + " [Ch:" + String(channel + 1) + "]\n";
  CompositeSerial.write(s.c_str());
#endif
}

void handleSysExData(unsigned char data)
{
  //setMidiStatus(kMidiReceiving);
#if USB_SERIAL_LOGGING
  String s = String() + "Received SysEx byte: 0x" + String(data, 16) + "\n";
  CompositeSerial.write(s.c_str());
#endif
}

void handleSysExEnd()
{
  //setMidiStatus(kMidiReceiving);
#if USB_SERIAL_LOGGING
  String s = String() + "End of SysEx\n";
  CompositeSerial.write(s.c_str());
#endif
}

void setupUsb()
{
  USBComposite.clear();
  USBComposite.setManufacturerString("Tranzistor Bandicoot");
  USBComposite.setProductString("LFO");
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

void setup()
{
  pinMode(PinStatusLed, OUTPUT);
  digitalWrite(PinStatusLed, HIGH);

  Serial3.begin(31250);

  lfo.setFrequency(1);
  lfo.setPhaseOffset(1431655765, 1);
  lfo.setPhaseOffset(2863311531, 2);
  lfo.rampFrequency(0.1, 10000);

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
  if (counter % DownSample == 0)
  {
    lfo.advance();
    int n = 0;
    for (; n <= (int)PwmOut::V3; n++)
    {
      timer_set_compare(Pwms[n].timer, Pwms[n].channel, lfo.sampleIP(n) >> (16 - PwmBits));
    }
    n = (int)PwmOut::Clean;
    timer_set_compare(Pwms[n].timer, Pwms[n].channel, PwmMax);
  }
  counter++;
}

String noteName(int pitch)
{
  return String() + kNotes[pitch % 12] + String(pitch / 12 - 1);
}

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
  //sendControlChange(channel, pot1Control, TremoloPot.value());
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

void receiveSerial()
{
  setMidiStatus(MidiStatus::Receiving);
#if OLED_DISPLAY
  String oledStr;
  display.clear();
  display.drawString(0, 0, "Received MIDI");
#endif
  while (Serial3.available())
  {
    setMidiStatus(MidiStatus::Receiving);
    auto byte = Serial3.read();

#if OLED_DISPLAY
    oledStr += String(byte, 16) + " ";
#endif
#if USB_SERIAL_LOGGING
    {
      String s = String() + "Received byte: 0x" + String(byte, 16) + "\n";
      CompositeSerial.write(s.c_str());
    }
#endif
    delay(1);
  }
#if OLED_DISPLAY
  display.drawString(0, 16, oledStr.c_str());
  display.show();
#endif
}

void loop()
{
  if (Serial3.available())
  {
    receiveSerial();
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
