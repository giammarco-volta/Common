#pragma once
#include <QString>
#include <QStringList>
#include <vector>
#include <cstdint>

class IMidiOut
{
public:
  virtual ~IMidiOut() = default;

  virtual QStringList listOutputs() const = 0;
  virtual bool open(int index) = 0;
  virtual void close() = 0;

  virtual bool sendShort(uint8_t status, uint8_t data1, uint8_t data2) = 0;
  virtual bool sendSysEx(const std::vector<uint8_t>& sysex) = 0;
};
