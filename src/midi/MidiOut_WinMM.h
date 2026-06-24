#pragma once

#ifdef _WIN32
#include "IMidiOut.h"
#include <windows.h>
#include <mmsystem.h>

class MidiOut_WinMM : public IMidiOut
{
public:
  MidiOut_WinMM();
  ~MidiOut_WinMM() override;

  QStringList listOutputs() const override;
  bool open(int index) override;
  void close() override;

  bool sendShort(uint8_t status, uint8_t data1, uint8_t data2) override;
  bool sendSysEx(const std::vector<uint8_t>& sysex) override;

private:
  HMIDIOUT handle_{nullptr};
  int currentIndex_{-1};
};
#endif
