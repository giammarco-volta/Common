#pragma once
#include <QSlider>

class CenterFillSlider : public QSlider
{
  Q_OBJECT
public:
  explicit CenterFillSlider(Qt::Orientation ori, QWidget* parent = nullptr);

protected:
  void paintEvent(QPaintEvent* e) override;

private:
  int centerValue() const; // valore neutro (qui: 0)
};
