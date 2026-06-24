#pragma once
#include "IMidiOut.h"

#ifdef Q_OS_ANDROID
#include "MidiAndroidBridge.h"
#endif

class MidiOut_Stub : public IMidiOut
{
public:
  QStringList listOutputs() const override
  {
#ifdef Q_OS_ANDROID
    return MidiAndroidBridge::listOutputs();
#else
    return { "<MIDI non implementato su questa piattaforma>" };
#endif
  }
  bool open(int) override { return false; }
  void close() override {}

  bool sendShort(uint8_t, uint8_t, uint8_t) override { return false; }
  bool sendSysEx(const std::vector<uint8_t>&) override { return false; }
};
