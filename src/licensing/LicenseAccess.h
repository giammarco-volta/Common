#pragma once
#include <QObject>
#include <QVariantMap>
#include <atomic>

namespace NaadaLab {
class LicenseAccess final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap status READ status NOTIFY changed)
    Q_PROPERTY(bool allowed READ allowed NOTIFY changed)
    Q_PROPERTY(bool enabled READ enabled CONSTANT)
public:
    explicit LicenseAccess(QObject* parent = nullptr);
    QVariantMap status() const { return status_; }
    bool allowed() const { return midiAllowed(); }
    bool enabled() const;
    static bool midiAllowed();
    Q_INVOKABLE void activate();
    Q_INVOKABLE void restore();
    Q_INVOKABLE void buy();
    Q_INVOKABLE void unlockReview(const QString& code);
    Q_INVOKABLE bool copySupportReference();
signals:
    void changed();
    void accessGranted();
private:
    void poll();
    void call(const char* method);
    QVariantMap status_;
    static std::atomic_bool allowed_;
};
}
