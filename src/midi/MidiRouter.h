#pragma once

class MidiRouter
{
public:

  void noteOn(int channel, int note, int velocity);
  void noteOff(int channel, int note);
};