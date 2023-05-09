#include "Globals.h"

const char* kNotes[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

int channel = 0;
int pot1Control = 81;
int pot2Control = 82;

volatile int counter = 0;
volatile float freq = 1;

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
    timer_set_reload(Pwms[i].timer, Resolution);
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

  setupPwms();
  
  delay(200);
  setupUsb();

  Display.init();
  Display.clear();
  Display.drawString(0, 0, "MIDI Controller");
  Display.show();

  Lfo.setFrequency(freq);
}

void TimerInterrupt()
{
  if (counter % DownSample == 0)
  {
    int fixed = static_cast<int>((process() / 2 + 0.5) * (Resolution - 1));
    int clamped = fixed < 0 ? 0 : (fixed >= Resolution ? Resolution - 1 : fixed);
    for (int i = 0; i < PwmOutCount; i++)
    {
      timer_set_compare(Pwms[i].timer, Pwms[i].channel, static_cast<uint16>(clamped));
    }
  }
  counter++;
}

float process()
{
  Lfo.advance();
  return Lfo.sampleIP();
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
  String s;
  s += "ch:" + String(chan + 1);
  s += " note:" + noteName(pitch);
  s += " vel:" + String(vel);
  Display.clear();
  Display.drawString(0, 0, "Note On");
  Display.drawString(0, 16, s.c_str());
  Display.show();
}

void sendControlChange(int chan, int ctl, int val)
{
#if not USB_SERIAL_LOGGING
  midi.sendControlChange(chan, ctl, val);
#endif
  String s;
  s += "ch:" + String(chan + 1);
  s += " ctl:" + String(ctl);
  s += " val:" + String(val);
  Display.clear();
  Display.drawString(0, 0, "Control Change");
  Display.drawString(0, 16, s.c_str());
  Display.show();
}

void sendPot1Value()
{
  sendControlChange(channel, pot1Control, TremoloPot.value());
}

void sendPot2Value()
{
  sendControlChange(channel, pot2Control, VibratoPot.value());
  float v = static_cast<float>(VibratoPot.value()) / 127;
  v = pow(2, v);
  Lfo.rampFrequency(v * 880, 1000);
}

void loop()
{
  /*
  if (TremoloPot.update()) {
    sendPot1Value();
  }  
  if (VibratoPot.update()) {
    sendPot2Value();
  }
  */
}
