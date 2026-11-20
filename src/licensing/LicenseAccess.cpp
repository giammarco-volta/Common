#include "LicenseAccess.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QJniEnvironment>
#include <QtCore/qnativeinterface.h>
#endif

namespace NaadaLab {
#ifdef Q_OS_ANDROID
std::atomic_bool LicenseAccess::allowed_{false};
#else
std::atomic_bool LicenseAccess::allowed_{true};
#endif
bool LicenseAccess::midiAllowed() { return allowed_.load(std::memory_order_relaxed); }
bool LicenseAccess::enabled() const {
#ifdef Q_OS_ANDROID
    return true;
#else
    return false;
#endif
}
LicenseAccess::LicenseAccess(QObject* parent) : QObject(parent) {
    status_ = {{"state", "checking"}, {"busy", enabled()}, {"allowed", allowed()},
               {"message", "Checking accessâ€¦"}};
#ifdef Q_OS_ANDROID
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([] {
        auto activity = QNativeInterface::QAndroidApplication::context();
        QJniObject::callStaticMethod<void>("com/naadalab/licensing/PlayLicenseBridge", "initialize",
                                         "(Landroid/app/Activity;)V", activity.object());
    });
    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LicenseAccess::poll);
    timer->start(500);
    connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState s) {
        if (s == Qt::ApplicationActive) call("resume");
    });
#endif
}
void LicenseAccess::call(const char* method) {
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>("com/naadalab/licensing/PlayLicenseBridge", method, "()V");
#else
    Q_UNUSED(method)
#endif
}
void LicenseAccess::activate() { call("activate"); }
void LicenseAccess::restore() { call("restore"); }
void LicenseAccess::buy() { call("buy"); }
void LicenseAccess::unlockReview(const QString& code) {
#ifdef Q_OS_ANDROID
    const auto value = QJniObject::fromString(code);
    QJniObject::callStaticMethod<void>("com/naadalab/licensing/PlayLicenseBridge", "review",
                                     "(Ljava/lang/String;)V", value.object<jstring>());
#else
    Q_UNUSED(code)
#endif
}
bool LicenseAccess::copySupportReference() {
    const QString reference = status_.value("supportReference").toString();
    if (reference.isEmpty() || !QGuiApplication::clipboard()) return false;
    QGuiApplication::clipboard()->setText(reference);
    return true;
}
void LicenseAccess::poll() {
#ifdef Q_OS_ANDROID
    const auto json = QJniObject::callStaticObjectMethod("com/naadalab/licensing/PlayLicenseBridge", "status", "()Ljava/lang/String;");
    QJniEnvironment env;
    if (env->ExceptionCheck()) { env->ExceptionClear(); return; }
    const auto doc = QJsonDocument::fromJson(json.toString().toUtf8());
    if (!doc.isObject()) return;
    const auto next = doc.object().toVariantMap();
    if (next == status_) return;
    const bool previous = allowed();
    allowed_.store(next.value("allowed", false).toBool(), std::memory_order_relaxed);
    status_ = next;
    emit changed();
    if (!previous && allowed()) emit accessGranted();
#endif
}
}
