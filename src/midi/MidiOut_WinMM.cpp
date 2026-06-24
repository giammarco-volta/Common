#ifdef _WIN32
#include "MidiOut_WinMM.h"
#include <QByteArray>

MidiOut_WinMM::MidiOut_WinMM() {}
MidiOut_WinMM::~MidiOut_WinMM() { close(); }

//--------------------------------------------
QStringList MidiOut_WinMM::listOutputs() const
//--------------------------------------------
{
  QStringList list;
  UINT n = midiOutGetNumDevs();
  for (UINT i = 0; i < n; ++i)
  {
    MIDIOUTCAPS caps{};
    if (midiOutGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
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

//---------------------------------
bool MidiOut_WinMM::open(int index)
//---------------------------------
{
  if (index < 0) return false;
  if (handle_ && currentIndex_ == index) return true;

  close();
  MMRESULT r = midiOutOpen(&handle_, (UINT)index, 0, 0, CALLBACK_NULL);
  if (r == MMSYSERR_NOERROR)
  {
    currentIndex_ = index;
    return true;
  }
  handle_ = nullptr;
  currentIndex_ = -1;
  return false;
}

//-------------------------
void MidiOut_WinMM::close()
//-------------------------
{
  if (handle_)
  {
    midiOutReset(handle_);
    midiOutClose(handle_);
    handle_ = nullptr;
    currentIndex_ = -1;
  }
}

//-------------------------------------------------------------------------
bool MidiOut_WinMM::sendShort(uint8_t status, uint8_t data1, uint8_t data2)
//-------------------------------------------------------------------------
{
  if (!handle_) return false;
  DWORD msg = 0;
  msg |= (DWORD)status;
  msg |= ((DWORD)data1) << 8;
  msg |= ((DWORD)data2) << 16;
  return midiOutShortMsg(handle_, msg) == MMSYSERR_NOERROR;
}

//--------------------------------------------------------------
bool MidiOut_WinMM::sendSysEx(const std::vector<uint8_t>& sysex)
//--------------------------------------------------------------
{
  if (!handle_) return false;
  if (sysex.empty()) return false;
  if (sysex.front() != 0xF0 || sysex.back() != 0xF7) return false;

  MIDIHDR hdr{};
  hdr.lpData = (LPSTR)sysex.data();
  hdr.dwBufferLength = (DWORD)sysex.size();

  if (midiOutPrepareHeader(handle_, &hdr, sizeof(hdr)) != MMSYSERR_NOERROR)
    return false;

  MMRESULT r = midiOutLongMsg(handle_, &hdr, sizeof(hdr));
  if (r != MMSYSERR_NOERROR)
  {
    midiOutUnprepareHeader(handle_, &hdr, sizeof(hdr));
    return false;
  }

  // attesa sincrona semplice (MVP). In versione “seria” si fa async/callback.
  while ((hdr.dwFlags & MHDR_DONE) == 0) { Sleep(1); }

  midiOutUnprepareHeader(handle_, &hdr, sizeof(hdr));
  return true;
}
#endif
