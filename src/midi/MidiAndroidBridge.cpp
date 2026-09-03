#include "MidiAndroidBridge.h"

#include <QDebug>

#ifdef Q_OS_ANDROID

#include <QJniObject>
#include <QJniEnvironment>

static QStringList javaStringArrayToQStringList(const QJniObject& array)
{
  QStringList result;

  QJniEnvironment env;
  const jsize count = env->GetArrayLength(array.object<jobjectArray>());

  for (jsize i = 0; i < count; ++i)
  {
    jobject item = env->GetObjectArrayElement(array.object<jobjectArray>(), i);
    QJniObject str(item);
    result << str.toString();
    env->DeleteLocalRef(item);
  }

  return result;
}

QStringList MidiAndroidBridge::listInputs()
{
  qDebug() << "MORPHMASTER MIDI: calling listInputs";

  QJniObject array = QJniObject::callStaticObjectMethod(
    "org/qtproject/qt/android/MidiAndroidBridge",
    "listInputs",
    "()[Ljava/lang/String;");

  qDebug() << "MORPHMASTER MIDI: Java call returned";

  if (!array.isValid())
  {
    qDebug() << "MORPHMASTER MIDI: Java array invalid";
    QJniEnvironment env;
    if (env->ExceptionCheck())
    {
      env->ExceptionDescribe();
      env->ExceptionClear();
    }
    return {};
  }

  return javaStringArrayToQStringList(array);
}

QStringList MidiAndroidBridge::listOutputs()
{
  qDebug() << "MORPHMASTER MIDI: calling listOutputs";

  QJniObject array = QJniObject::callStaticObjectMethod(
    "org/qtproject/qt/android/MidiAndroidBridge",
    "listOutputs",
    "()[Ljava/lang/String;");

  qDebug() << "MORPHMASTER MIDI: Java call returned";

  if (!array.isValid())
  {
    qDebug() << "MORPHMASTER MIDI: Java array invalid";
    QJniEnvironment env;
    if (env->ExceptionCheck())
    {
      env->ExceptionDescribe();
      env->ExceptionClear();
    }
    return {};
  }

  return javaStringArrayToQStringList(array);
}

#else

QStringList MidiAndroidBridge::listInputs()
{
  return {};
}

QStringList MidiAndroidBridge::listOutputs()
{
  return {};
}

#endif