#ifdef _WIN32
#include "MidiIn_WinMM.h"

#include <QDebug>

MidiIn_WinMM::MidiIn_WinMM() {}
MidiIn_WinMM::~MidiIn_WinMM() { close(); }

//------------------------------------------
QStringList MidiIn_WinMM::listInputs() const
//------------------------------------------
{
  QStringList list;
  UINT n = midiInGetNumDevs();
  for (UINT i = 0; i < n; ++i)
  {
    MIDIINCAPS caps{};
    if (midiInGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
    {
#ifdef UNICODE
      list << QString::fromWCharArray(caps.szPname);
#else
      list << QString::fromLocal8Bit(caps.szPname);
#endif
    }
  }
  return list;
}

//--------------------------------
bool MidiIn_WinMM::open(int index)
//--------------------------------
{
  if (index < 0) return false;
  close();

  MMRESULT r = midiInOpen(&handle_,
                         static_cast<UINT>(index),
                         reinterpret_cast<DWORD_PTR>(&MidiIn_WinMM::midiInProc),
                         reinterpret_cast<DWORD_PTR>(this),
                         CALLBACK_FUNCTION);
  if (r != MMSYSERR_NOERROR)
  {
    handle_ = nullptr;
    return false;
  }

  currentIndex_ = index;

  r = midiInStart(handle_);
  if (r != MMSYSERR_NOERROR)
  {
    close();
    return false;
  }
  return true;
}

void MidiIn_WinMM::close()
{
  if (!handle_) return;

  midiInStop(handle_);
  midiInReset(handle_);
  midiInClose(handle_);

  handle_ = nullptr;
  currentIndex_ = -1;
}

//-----------------------------------------
void MidiIn_WinMM::setCallback(Callback cb)
//-----------------------------------------
{
  std::lock_guard<std::mutex> lock(cbMutex_);
  callback_ = std::move(cb);
}

//----------------------------------------------------------------------------------------------------------------------
void CALLBACK MidiIn_WinMM::midiInProc(HMIDIIN, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
//----------------------------------------------------------------------------------------------------------------------
{
  auto* self = reinterpret_cast<MidiIn_WinMM*>(dwInstance);
  if (!self) return;

  switch (wMsg)
  {
    case MIM_DATA:
      // dwParam1: packed short message, dwParam2: timestamp in ms
      self->handleShortMessage(dwParam1, dwParam2);
      break;

    // SysEx (not implemented in this MVP):
    // case MIM_LONGDATA: ...
    default:
      break;
  }
}

//-----------------------------------------------------------------------
void MidiIn_WinMM::handleShortMessage(DWORD_PTR packed, DWORD_PTR timeMs)
//-----------------------------------------------------------------------
{
  MidiInEvent ev;
  ev.type = MidiInEvent::Type::ShortMessage;
  ev.status = static_cast<uint8_t>(packed & 0xFF);
  ev.data1  = static_cast<uint8_t>((packed >> 8) & 0xFF);
  ev.data2  = static_cast<uint8_t>((packed >> 16) & 0xFF);
  ev.timeMs = static_cast<uint32_t>(timeMs);

  Callback cb;
  {
    std::lock_guard<std::mutex> lock(cbMutex_);
    cb = callback_;
  }
  if (cb)
    cb(ev);
}
#endif
