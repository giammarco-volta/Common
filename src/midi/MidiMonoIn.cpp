#include "MidiMonoIn.h"

void MidiIn_MonoInterpreter::onInnerEvent(const MidiInEvent& ev)
{
  if (!isShort(ev))
    return;

  // Filtra canale sorgente
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (ev.channel() != sourceChannel_)
      return;
  }

  // 1) Eventi che non vanno propagati
  const bool ignoredProgramChange =
    configuration_.ignoreProgramChanges
    && isProgramNumber(ev);

  const bool ignoredControlChange =
    ev.message() == 0xB0
    && configuration_.ignoredControlChanges.test(ev.data1);

  if (ignoredProgramChange || ignoredControlChange)
    return;

  // 2) Note On => diventa la nuova nota corrente (sostituisce sempre la precedente)
  if (isNoteOn(ev))
  {
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      st_.type = EventType::noteOn;
      st_.currentNote = ev.data1;
      st_.currentVel = ev.data2;
      st_.lastTimeMs = ev.timeMs;

      st_.livingNotes.push_back({ ev.data1 , ev.data2 });
    }

    dispatchToUser(ev);
    return;
  }

  // 3) Note Off => spegne la nota corrente corrispondente
  if (isNoteOff(ev))
  {
    {
      std::lock_guard<std::mutex> lock(stateMutex_);

      if (configuration_.noteOffStatePolicy
          == NoteOffStatePolicy::ClearCurrentNote)
      {
        st_.hasCurrentNote = false;
        st_.currentNote = 0;
        st_.currentVel = 0;
        st_.releasedWhileSustain = false;
        st_.type = EventType::noteOff;
        st_.lastTimeMs = ev.timeMs;
      }

      std::list<std::pair<uint8_t, uint8_t>>::iterator it = st_.livingNotes.begin();
      while (it != st_.livingNotes.end())
      {
        if (ev.data1 == it->first)
        {
          st_.type = EventType::noteOff;
          st_.lastTimeMs = ev.timeMs;
          it = st_.livingNotes.erase(it);
        }
        else
          ++it;
      }
    }

    dispatchToUser(ev);
    return;
  }

  // 4) Altri messaggi del canale selezionato: forward senza toccare lo stato
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    st_.type = EventType::channelMsg;
  }
  dispatchToUser(ev);
}
