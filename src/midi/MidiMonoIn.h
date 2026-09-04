#pragma once
#include "IMidiIn.h"

#include <memory>
#include <mutex>
#include <cstdint>
#include <list>
#include <utility>
#include <bitset>

class MidiIn_MonoInterpreter final : public IMidiIn
{
public:
  enum class NoteOffStatePolicy
  {
    KeepLastNote,
    ClearCurrentNote
  };

  struct Configuration
  {
    std::bitset<128> ignoredControlChanges;
    bool ignoreProgramChanges = false;
    NoteOffStatePolicy noteOffStatePolicy = NoteOffStatePolicy::KeepLastNote;
  };

  MidiIn_MonoInterpreter(
    std::unique_ptr<IMidiIn> inner,
    Configuration configuration)
    : inner_(std::move(inner)),
      configuration_(std::move(configuration))
  {
    inner_->setCallback(
      [this](const MidiInEvent& ev) { onInnerEvent(ev); });
  }

  // IMidiIn passthrough
  QStringList listInputs() const override { return inner_->listInputs(); }
  bool open(int index) override { return inner_->open(index); }
  void close() override { inner_->close(); }

  void setCallback(Callback cb) override
  {
    std::lock_guard<std::mutex> lock(cbMutex_);
    userCb_ = std::move(cb);
  }

  // Canale sorgente (0..15). Tutto il resto viene ignorato.
  void setSourceChannel(uint8_t ch)
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    sourceChannel_ = static_cast<uint8_t>(ch & 0x0F);
    resetState_NoLock();
  }

  uint8_t sourceChannel() const
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return sourceChannel_;
  }

  // Stato “numerico” (interpretazione mono)
  struct State
  {
    EventType type = EventType::unknown;
    bool hasCurrentNote = false;
    uint8_t currentNote = 0;
    uint8_t currentVel = 0;
    uint32_t lastTimeMs = 0;

    bool sustainDown = false;

    // true se NOTE OFF della currentNote è arrivato mentre sustain era giù
    bool releasedWhileSustain = false;
	
	std::list<std::pair<uint8_t, uint8_t>> livingNotes;
  };

  State getState() const
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return st_;
  }

  // Utility comode (se ti servono in giro)
  bool hasCurrentNote() const { return getState().hasCurrentNote; }
  uint8_t currentNote() const { return getState().currentNote; }

private:
  void dispatchToUser(const MidiInEvent& ev)
  {
    Callback cb;
    {
      std::lock_guard<std::mutex> lock(cbMutex_);
      cb = userCb_;
    }
    if (cb) cb(ev);
  }

  static bool isShort(const MidiInEvent& ev)
  {
    return ev.type == MidiInEvent::Type::ShortMessage;
  }

  static bool isNoteOn(const MidiInEvent& ev)
  {
    // Note On con velocity 0 è Note Off
    return isShort(ev) && (ev.message() == 0x90) && (ev.data2 != 0);
  }

  static bool isNoteOff(const MidiInEvent& ev)
  {
    return isShort(ev) &&
      ((ev.message() == 0x80) || ((ev.message() == 0x90) && (ev.data2 == 0)));
  }

  static bool isProgramNumber(const MidiInEvent& ev)
  {
    return isShort(ev) && (ev.message() == 0xC0);
  }

  void resetState_NoLock()
  {
    st_ = State{};
  }

  void onInnerEvent(const MidiInEvent& ev);

private:
  Configuration configuration_;
  std::unique_ptr<IMidiIn> inner_;

  mutable std::mutex stateMutex_;
  uint8_t sourceChannel_ = 0;
  State st_;

  std::mutex cbMutex_;
  Callback userCb_;
};