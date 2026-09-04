#include "MidiInFactory.h"

#if defined(_WIN32)
  #include "MidiIn_WinMM.h"
#elif defined(Q_OS_ANDROID)
  #include "MidiIn_Android.h"
#else
  #include "MidiIn_Stub.h"
#endif

std::unique_ptr<MidiIn_MonoInterpreter> createMidiIn(const MidiIn_MonoInterpreter::Configuration& configuration)
{
#if defined(_WIN32)
  auto in = std::make_unique<MidiIn_WinMM>();
#elif defined(Q_OS_ANDROID)
  auto in = std::make_unique<MidiIn_Android>();
#else
  auto in = std::make_unique<MidiIn_Stub>();
#endif
  return std::make_unique<MidiIn_MonoInterpreter>(
    std::move(in),
    configuration);
}
