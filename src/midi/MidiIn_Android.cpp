#include "MidiIn_Android.h"

#if defined(Q_OS_ANDROID) || defined(ANDROID)

#include "MidiAndroidBridge.h"

#include <QDebug>
#include <QJniEnvironment>
#include <QJniObject>

static MidiIn_Android* g_midiInAndroid = nullptr;

MidiIn_Android::MidiIn_Android()
{
  g_midiInAndroid = this;
}

MidiIn_Android::~MidiIn_Android()
{
  close();

  if (g_midiInAndroid == this)
    g_midiInAndroid = nullptr;
}

QStringList MidiIn_Android::listInputs() const
{
  return MidiAndroidBridge::listInputs();
}

bool MidiIn_Android::open(int index)
{
  close();

  qDebug() << "MORPHMASTER MIDI: MidiIn_Android::open" << index;

  const bool ok = QJniObject::callStaticMethod<jboolean>(
    "org/qtproject/qt/android/MidiAndroidBridge",
    "openInput",
    "(I)Z",
    index);

  if (!ok)
  {
    qDebug() << "MORPHMASTER MIDI: openInput failed";
    return false;
  }

  currentIndex_ = index;
  qDebug() << "MORPHMASTER MIDI: openInput requested";
  return true;
}

void MidiIn_Android::close()
{
  if (currentIndex_ < 0)
    return;

  qDebug() << "MORPHMASTER MIDI: MidiIn_Android::close";

  QJniObject::callStaticMethod<void>(
    "org/qtproject/qt/android/MidiAndroidBridge",
    "closeInput",
    "()V");

  currentIndex_ = -1;
}

void MidiIn_Android::setCallback(Callback cb)
{
  std::lock_guard<std::mutex> lock(cbMutex_);
  callback_ = std::move(cb);
}

void MidiIn_Android::handleBytes(const uint8_t* data, int count, uint32_t timeMs)
{
  if (!data || count <= 0)
    return;

  int i = 0;

  while (i < count)
  {
    const uint8_t status = data[i];

    // Active Sensing
    if (status == 0xFE)
    {
      ++i;
      continue;
    }

    // Per ora gestiamo solo messaggi MIDI "channel voice".
    if (status < 0x80)
    {
      ++i;
      continue;
    }

    int msgLen = 0;

    switch (status & 0xF0)
    {
      case 0xC0: // Program Change
      case 0xD0: // Channel Pressure
        msgLen = 2;
        break;

      case 0x80: // Note Off
      case 0x90: // Note On
      case 0xA0: // Poly Aftertouch
      case 0xB0: // Control Change
      case 0xE0: // Pitch Bend
        msgLen = 3;
        break;

      default:
        ++i;
        continue;
    }

    if (i + msgLen > count)
      break;

    MidiInEvent ev;
    ev.type = MidiInEvent::Type::ShortMessage;
    ev.status = data[i];
    ev.data1  = (msgLen > 1) ? data[i + 1] : 0;
    ev.data2  = (msgLen > 2) ? data[i + 2] : 0;
    ev.timeMs = timeMs;

    //qDebug() << "MORPHMASTER MIDI:"
    //         << QString::number(ev.status, 16)
    //         << ev.data1
    //         << ev.data2;

    Callback cb;
    {
      std::lock_guard<std::mutex> lock(cbMutex_);
      cb = callback_;
    }

    if (cb)
      cb(ev);

    i += msgLen;
  }
}

extern "C"
JNIEXPORT void JNICALL
Java_org_qtproject_qt_android_MidiAndroidBridge_nativeOnMidiReceived(
  JNIEnv* env,
  jclass,
  jbyteArray data,
  jint count,
  jlong timestamp)
{
  if (!g_midiInAndroid || !data || count <= 0)
    return;

  jbyte* bytes = env->GetByteArrayElements(data, nullptr);
  if (!bytes)
    return;

  g_midiInAndroid->handleBytes(
    reinterpret_cast<const uint8_t*>(bytes),
    static_cast<int>(count),
    static_cast<uint32_t>(timestamp / 1000000)); // Android timestamp è in ns

  env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);
}

#endif