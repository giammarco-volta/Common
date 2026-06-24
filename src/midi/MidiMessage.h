#pragma once
#include <cstdint>
#include <vector>

struct MidiMessage
{
  // Short messages: status + 1/2 data bytes
  static MidiMessage NoteOn(uint8_t ch, uint8_t note, uint8_t vel)
  {
    return { uint8_t(0x90 | (ch & 0x0F)), note, vel };
  }

  static MidiMessage NoteOff(uint8_t ch, uint8_t note, uint8_t vel=0)
  {
    return { uint8_t(0x80 | (ch & 0x0F)), note, vel };
  }

  static MidiMessage CC(uint8_t ch, uint8_t cc, uint8_t val)
  {
    return { uint8_t(0xB0 | (ch & 0x0F)), cc, val };
  }

  // SysEx as raw bytes including 0xF0...0xF7
  static std::vector<uint8_t> SysEx(const std::vector<uint8_t>& bytes)
  {
    return bytes;
  }

  uint8_t status{};
  uint8_t data1{};
  uint8_t data2{};
};
