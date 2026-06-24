#include "MidiChannelSelector.h"

#include <QPushButton>
#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QEvent>
#include <QPointer>
#include <QApplication>
#include <QScreen>
#include <algorithm>

//-------------------------------------------------------------------------
MidiChannelSelector::MidiChannelSelector(QWidget* parent) : QWidget(parent)
//-------------------------------------------------------------------------
{
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  button_ = new QPushButton(this);
  button_->setMinimumHeight(36);
  button_->setMinimumWidth(64);
  button_->setText(QString::number(value_) + QStringLiteral(" ▼"));
  button_->setFocusPolicy(Qt::StrongFocus);

  layout->addWidget(button_);

  connect(button_, &QPushButton::clicked, this, &MidiChannelSelector::showPopup);
}

//-----------------------------------------------
void MidiChannelSelector::setValue(uint8_t value)
//-----------------------------------------------
{
  value = std::clamp<uint8_t>(value, 1, 16);

  if (value_ == value)
    return;

  value_ = value;
  updateButtonText();

  emit valueChanged(value_);
}

//------------------------------------------
void MidiChannelSelector::updateButtonText()
//------------------------------------------
{
  if (button_)
    button_->setText(QString::number(value_) + QStringLiteral(" ▼"));
}

//-----------------------------------
void MidiChannelSelector::showPopup()
//-----------------------------------
{
  if (popup_)
  {
    hidePopup();
    return;
  }

#ifdef Q_OS_ANDROID
  popup_ = new QFrame(window(), Qt::FramelessWindowHint);
#else
  popup_ = new QFrame(nullptr, Qt::Popup);
#endif
  popup_->setObjectName(QStringLiteral("MidiChannelSelectorPopup"));
  popup_->setFrameShape(QFrame::StyledPanel);
  popup_->setAttribute(Qt::WA_DeleteOnClose);
  popup_->installEventFilter(this);

  auto* grid = new QGridLayout(popup_);
#ifdef Q_OS_ANDROID
  grid->setContentsMargins(16, 16, 16, 16);
  grid->setSpacing(12);
#else
  grid->setContentsMargins(8, 8, 8, 8);
  grid->setSpacing(6);
#endif
  for (int ch = 1; ch <= 16; ++ch)
  {
    auto* b = new QPushButton(QString::number(ch), popup_);
#ifdef Q_OS_ANDROID
    b->setMinimumSize(96, 80);
#else
    b->setMinimumSize(48, 44);
#endif
    b->setCheckable(true);
    b->setChecked(ch == value_);
    b->setFocusPolicy(Qt::NoFocus);

    const int row = (ch - 1) / 4;
    const int col = (ch - 1) % 4;
    grid->addWidget(b, row, col);

    connect(b, &QPushButton::clicked, this,
      [this, ch]()
      {
        setValue(static_cast<uint8_t>(ch));
        hidePopup();
      });
  }

  popup_->adjustSize();

  QPoint pos = button_->mapToGlobal(QPoint(0, button_->height() + 2));

  if (const QScreen* screen = QApplication::screenAt(pos))
  {
    const QRect available = screen->availableGeometry();
    const QSize popupSize = popup_->sizeHint();

    if (pos.x() + popupSize.width() > available.right())
      pos.setX(std::max(available.left(), available.right() - popupSize.width()));

    if (pos.y() + popupSize.height() > available.bottom())
      pos.setY(button_->mapToGlobal(QPoint(0, -popupSize.height() - 2)).y());
  }

  popup_->move(pos);
  popup_->show(); 
#ifdef Q_OS_ANDROID
  popup_->raise();
#endif
}

//-----------------------------------
void MidiChannelSelector::hidePopup()
//-----------------------------------
{
  if (!popup_)
    return;

  QFrame* popup = popup_;
  popup_ = nullptr;
  popup->close();
}

//---------------------------------------------------------------
bool MidiChannelSelector::eventFilter(QObject* watched, QEvent* event)
//---------------------------------------------------------------
{
  if (watched == popup_ && event->type() == QEvent::Close)
    popup_ = nullptr;

  return QWidget::eventFilter(watched, event);
}
