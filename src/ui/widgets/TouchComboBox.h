#pragma once

#include <QComboBox>

class QFrame;
class QListWidget;

class TouchComboBox : public QComboBox
{
  Q_OBJECT

public:
  explicit TouchComboBox(QWidget* parent = nullptr);

protected:
  void showPopup() override;

private:
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
  void showTouchPopup();

  QFrame* popup_ = nullptr;
  QListWidget* list_ = nullptr;
#endif
};