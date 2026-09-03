#pragma once

#if defined(Q_OS_ANDROID) || defined(ANDROID)

#include "IMidiIn.h"
#include <cstdint>
#include <mutex>

class MidiIn_Android : public IMidiIn
{
public:
  MidiIn_Android();
  ~MidiIn_Android() override;

  QStringList listInputs() const override;
  bool open(int index) override;
  void close() override;

  void setCallback(Callback cb) override;

  void handleBytes(const uint8_t* data, int count, uint32_t timeMs);

private:
  std::mutex cbMutex_;
  Callback callback_;
  int currentIndex_ = -1;
};

#endif