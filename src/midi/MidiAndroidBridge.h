#pragma once

#include <QStringList>

class MidiAndroidBridge
{
public:
  static QStringList listInputs();
  static QStringList listOutputs();
};