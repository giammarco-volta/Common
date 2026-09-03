#include "MidiOut_Android.h"

#if defined(Q_OS_ANDROID) || defined(ANDROID)

#include "MidiAndroidBridge.h"

#include <QDebug>
#include <QJniObject>
#include <QJniEnvironment>

MidiOut_Android::MidiOut_Android()
{
}

MidiOut_Android::~MidiOut_Android()
{
  close();
}

QStringList MidiOut_Android::listOutputs() const
{
  return MidiAndroidBridge::listOutputs();
}

bool MidiOut_Android::open(int index)
{
  close();

  qDebug() << "MORPHMASTER MIDI: MidiOut_Android::open" << index;

  const bool ok = QJniObject::callStaticMethod<jboolean>(
    "org/qtproject/qt/android/MidiAndroidBridge",
    "openOutput",
    "(I)Z",
    index);

  if (!ok)
  {
    qDebug() << "MORPHMASTER MIDI: openOutput failed";
    return false;
  }

  currentIndex_ = index;
  qDebug() << "MORPHMASTER MIDI: openOutput requested";
  return true;
}

void MidiOut_Android::close()
{
  if (currentIndex_ < 0)
    return;

  qDebug() << "MORPHMASTER MIDI: MidiOut_Android::close";

  QJniObject::callStaticMethod<void>(
    "org/qtproject/qt/android/MidiAndroidBridge",
    "closeOutput",
    "()V");

  currentIndex_ = -1;
}

bool MidiOut_Android::sendShort(uint8_t status, uint8_t data1, uint8_t data2)
{
  if (currentIndex_ < 0)
    return false;

  const bool ok = QJniObject::callStaticMethod<jboolean>(
    "org/qtproject/qt/android/MidiAndroidBridge",
    "sendShort",
    "(III)Z",
    int(status),
    int(data1),
    int(data2));

  return ok;
}

bool MidiOut_Android::sendSysEx(const std::vector<uint8_t>& sysex)
{
  if (currentIndex_ < 0 || sysex.empty())
    return false;

  QJniEnvironment env;

  jbyteArray arr = env->NewByteArray(jsize(sysex.size()));
  if (!arr)
    return false;

  env->SetByteArrayRegion(
    arr,
    0,
    jsize(sysex.size()),
    reinterpret_cast<const jbyte*>(sysex.data()));

  const bool ok = QJniObject::callStaticMethod<jboolean>(
    "org/qtproject/qt/android/MidiAndroidBridge",
    "sendBytes",
    "([B)Z",
    arr);

  env->DeleteLocalRef(arr);

  return ok;
}

#endif