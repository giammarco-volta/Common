#include "MidiOutFactory.h"

#if defined(_WIN32)
  #include "MidiOut_WinMM.h"
#elif defined(Q_OS_ANDROID)
  #include "MidiOut_Android.h"
#else
  #include "MidiOut_Stub.h"
#endif

std::unique_ptr<IMidiOut> createMidiOut()
{
#if defined(_WIN32)
  return std::make_unique<MidiOut_WinMM>();
#elif defined(Q_OS_ANDROID)
  return std::make_unique<MidiOut_Android>();
#else
  return std::make_unique<MidiOut_Stub>();
#endif
}
