#pragma once

#if defined(Q_OS_ANDROID) || defined(ANDROID)

#include "IMidiOut.h"

class MidiOut_Android : public IMidiOut
{
public:
  MidiOut_Android();
  ~MidiOut_Android() override;

  QStringList listOutputs() const override;
  bool open(int index) override;
  void close() override;

  bool sendShort(uint8_t status, uint8_t data1, uint8_t data2) override;
  bool sendSysEx(const std::vector<uint8_t>& sysex) override;

private:
  int currentIndex_ = -1;
};

#endif