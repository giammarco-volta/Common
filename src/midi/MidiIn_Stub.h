#pragma once
#include "IMidiIn.h"

#ifdef Q_OS_ANDROID
#include "MidiAndroidBridge.h"
#endif

class MidiIn_Stub : public IMidiIn
{
public:
  QStringList listInputs() const override
  {
#ifdef Q_OS_ANDROID
    return MidiAndroidBridge::listInputs();
#else
    return { "<MIDI non implementato su questa piattaforma>" };
#endif
  }

  bool open(int) override { return false; }
  void close() override {}

  void setCallback(Callback) override {}
};
