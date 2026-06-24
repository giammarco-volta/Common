#pragma once
#include <QStringList>
#include <cstdint>
#include <functional>

enum class EventType : uint8_t { noteOn, noteOff, expression, volume, program, CC0, CC32, pan, CC71, CC74, pitch, pressure, channelMsg, unknown };

/**
 * Minimal MIDI input interface (MVP).
 *
 * Threading note (Windows/WinMM):
 * - The callback can be invoked from a non-Qt system thread.
 * - Keep the callback light; if you need to touch Qt UI, forward using
 *   QMetaObject::invokeMethod(..., Qt::QueuedConnection) or queue events.
 */
struct MidiInEvent
{
  enum class Type : uint8_t
  {
    ShortMessage,   // generic 3-byte message
    SysEx           // not implemented in WinMM MVP
  };

  Type type{Type::ShortMessage};

  // For ShortMessage:
  uint8_t status{0};
  uint8_t data1{0};
  uint8_t data2{0};

  // Convenience:
  uint8_t channel() const { return static_cast<uint8_t>(status & 0x0F); }
  uint8_t message() const { return static_cast<uint8_t>(status & 0xF0); }

  // Timestamp from backend if available (milliseconds)
  uint32_t timeMs{0};
};

class IMidiIn
{
public:
  using Callback = std::function<void(const MidiInEvent&)>;

  virtual ~IMidiIn() = default;

  virtual QStringList listInputs() const = 0;
  virtual bool open(int index) = 0;
  virtual void close() = 0;

  virtual void setCallback(Callback cb) = 0;
};
