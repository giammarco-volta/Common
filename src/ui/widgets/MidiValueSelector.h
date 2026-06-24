#pragma once

#include <QWidget>

class QLabel;
class QFrame;
class QSlider;
class QToolButton;

/*
  Compact touch-friendly numeric selector.

  Closed state: a button showing the current value.
  Open state: a popup with a long horizontal slider.

  Intended as a replacement for QSpinBox in tablet/mobile contexts,
  especially for MIDI values in the 0..127 range.
*/
class MidiValueSelector : public QWidget
{
  Q_OBJECT

public:
  explicit MidiValueSelector(QWidget* parent = nullptr);

  int value() const { return value_; }
  int minimum() const { return minimum_; }
  int maximum() const { return maximum_; }

  void setRange(int minimum, int maximum);
  void setValue(int value);

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

signals:
  void valueChanged(int value);

private:
  void buildPopup();
  void showPopup();
  void updateButtonText();
  void updatePopupValueLabel();
  void updateStepButtonsEnabled();

  void stepDown();
  void stepUp();

private:
  int minimum_ = 0;
  int maximum_ = 127;
  int value_ = 0;

  QToolButton* button_ = nullptr;

  QFrame* popup_ = nullptr;
  QSlider* slider_ = nullptr;
  QLabel* valueLabel_ = nullptr;
  QToolButton* minusButton_ = nullptr;
  QToolButton* plusButton_ = nullptr;
  QToolButton* closeButton_ = nullptr;
};
