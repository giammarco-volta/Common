#pragma once

#ifdef _WIN32
#include "IMidiIn.h"
#include <windows.h>
#include <mmsystem.h>
#include <mutex>

class MidiIn_WinMM : public IMidiIn
{
public:
  MidiIn_WinMM();
  ~MidiIn_WinMM() override;

  QStringList listInputs() const override;
  bool open(int index) override;
  void close() override;

  void setCallback(Callback cb) override;

private:
  static void CALLBACK midiInProc(HMIDIIN hMidiIn,
                                 UINT wMsg,
                                 DWORD_PTR dwInstance,
                                 DWORD_PTR dwParam1,
                                 DWORD_PTR dwParam2);

  void handleShortMessage(DWORD_PTR packed, DWORD_PTR timeMs);

private:
  HMIDIIN handle_{nullptr};
  int currentIndex_{-1};

  std::mutex cbMutex_;
  Callback callback_;
};
#endif
