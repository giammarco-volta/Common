#pragma once

#include <QWidget>
#include <cstdint>

class QPushButton;
class QFrame;

class MidiChannelSelector : public QWidget
{
  Q_OBJECT

public:
  explicit MidiChannelSelector(QWidget* parent = nullptr);

  uint8_t value() const { return value_; }

public slots:
  void setValue(uint8_t value);

signals:
  void valueChanged(uint8_t value);

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  void updateButtonText();
  void showPopup();
  void hidePopup();

private:
  QPushButton* button_{};
  QFrame* popup_{};

  uint8_t value_ = 1;
};
