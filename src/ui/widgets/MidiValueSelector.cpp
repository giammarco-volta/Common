#include "MidiValueSelector.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QScreen>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>

//---------------------------------------------------------------------
MidiValueSelector::MidiValueSelector(QWidget* parent) : QWidget(parent)
//---------------------------------------------------------------------
{
  button_ = new QToolButton(this);
  button_->setText(QString::number(value_));
  button_->setToolButtonStyle(Qt::ToolButtonTextOnly);
  button_->setMinimumHeight(36);
  button_->setMinimumWidth(56);
  button_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(button_);

  connect(button_, &QToolButton::clicked, this, &MidiValueSelector::showPopup);
}

//---------------------------------------
QSize MidiValueSelector::sizeHint() const
//---------------------------------------
{
  return QSize(64, 36);
}

//----------------------------------------------
QSize MidiValueSelector::minimumSizeHint() const
//----------------------------------------------
{
  return QSize(56, 36);
}

//--------------------------------------------------------
void MidiValueSelector::setRange(int minimum, int maximum)
//--------------------------------------------------------
{
  if (minimum > maximum)
    std::swap(minimum, maximum);

  minimum_ = minimum;
  maximum_ = maximum;

  if (slider_)
    slider_->setRange(minimum_, maximum_);

  setValue(value_);
}

//-----------------------------------------
void MidiValueSelector::setValue(int value)
//-----------------------------------------
{
  const int clamped = std::max(minimum_, std::min(maximum_, value));

  if (value_ == clamped)
  {
    updateButtonText();
    updatePopupValueLabel();
    updateStepButtonsEnabled();
    return;
  }

  value_ = clamped;

  if (slider_ && slider_->value() != value_)
    slider_->setValue(value_);

  updateButtonText();
  updatePopupValueLabel();
  updateStepButtonsEnabled();

  emit valueChanged(value_);
}

//----------------------------------------
void MidiValueSelector::updateButtonText()
//----------------------------------------
{
  if (button_)
    button_->setText(QString::number(value_));
}

//---------------------------------------------
void MidiValueSelector::updatePopupValueLabel()
//---------------------------------------------
{
  if (valueLabel_)
    valueLabel_->setText(QString::number(value_));
}

//------------------------------------------------
void MidiValueSelector::updateStepButtonsEnabled()
//------------------------------------------------
{
  if (minusButton_)
    minusButton_->setEnabled(value_ > minimum_);

  if (plusButton_)
    plusButton_->setEnabled(value_ < maximum_);
}

//--------------------------------
void MidiValueSelector::stepDown()
//--------------------------------
{
  setValue(value_ - 1);
}

//------------------------------
void MidiValueSelector::stepUp()
//------------------------------
{
  setValue(value_ + 1);
}

//----------------------------------
void MidiValueSelector::buildPopup()
//----------------------------------
{
  if (popup_)
    return;

#ifdef Q_OS_ANDROID
  popup_ = new QFrame(window(), Qt::FramelessWindowHint);
#else
  popup_ = new QFrame(nullptr, Qt::Popup);
#endif
  popup_->setFrameShape(QFrame::StyledPanel);
  popup_->setAttribute(Qt::WA_DeleteOnClose, false);
  popup_->setAttribute(Qt::WA_StyledBackground, true);
  popup_->setAutoFillBackground(true);
  popup_->setObjectName(QStringLiteral("MidiValueSelectorPopup"));

  auto* root = new QVBoxLayout(popup_);
  root->setContentsMargins(12, 12, 12, 12);
  root->setSpacing(8);

  auto* headerRow = new QHBoxLayout();

  valueLabel_ = new QLabel(QString::number(value_), popup_);
  valueLabel_->setAlignment(Qt::AlignCenter);
  valueLabel_->setMinimumHeight(32);

  headerRow->addWidget(valueLabel_, 1);

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
  closeButton_ = new QToolButton(popup_);
  closeButton_->setText(QStringLiteral("×"));
  closeButton_->setMinimumSize(48, 44);
  headerRow->addWidget(closeButton_);
  connect(closeButton_, &QToolButton::clicked, popup_, &QFrame::hide);
#endif

  slider_ = new QSlider(Qt::Horizontal, popup_);
  slider_->setRange(minimum_, maximum_);
  slider_->setValue(value_);
  slider_->setMinimumWidth(360);
  slider_->setMinimumHeight(44);
  slider_->setSingleStep(1);
  slider_->setPageStep(8);

  auto* minMaxRow = new QHBoxLayout();
  auto* minLabel = new QLabel(QString::number(minimum_), popup_);
  auto* maxLabel = new QLabel(QString::number(maximum_), popup_);
  minMaxRow->addWidget(minLabel);
  minMaxRow->addStretch();
  minMaxRow->addWidget(maxLabel);

  auto* stepRow = new QHBoxLayout();
  stepRow->setSpacing(12);

  minusButton_ = new QToolButton(popup_);
  minusButton_->setText(QStringLiteral("−"));
  minusButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
  minusButton_->setMinimumSize(56, 44);
  minusButton_->setAutoRepeat(true);
  minusButton_->setAutoRepeatDelay(350);
  minusButton_->setAutoRepeatInterval(80);

  plusButton_ = new QToolButton(popup_);
  plusButton_->setText(QStringLiteral("+"));
  plusButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
  plusButton_->setMinimumSize(56, 44);
  plusButton_->setAutoRepeat(true);
  plusButton_->setAutoRepeatDelay(350);
  plusButton_->setAutoRepeatInterval(80);

  //auto* stepValueLabel = new QLabel(tr("Step by 1"), popup_);
  //stepValueLabel->setAlignment(Qt::AlignCenter);

  stepRow->addWidget(minusButton_);
  //stepRow->addWidget(stepValueLabel, 1);
  stepRow->addWidget(plusButton_);

  root->addLayout(headerRow);
  root->addWidget(slider_);
  root->addLayout(minMaxRow);
  root->addLayout(stepRow);

  connect(slider_, &QSlider::valueChanged, this, &MidiValueSelector::setValue);
  connect(minusButton_, &QToolButton::clicked, this, &MidiValueSelector::stepDown);
  connect(plusButton_, &QToolButton::clicked, this, &MidiValueSelector::stepUp);

  updateStepButtonsEnabled();
}

//-------------------------------
void MidiValueSelector::showPopup()
//-------------------------------
{
  buildPopup();

  if (!popup_)
    return;

  slider_->setRange(minimum_, maximum_);
  slider_->setValue(value_);
  updatePopupValueLabel();
  updateStepButtonsEnabled();

  popup_->adjustSize();

  QPoint pos = mapToGlobal(QPoint(0, height() + 2));

  if (const QScreen* screen = QApplication::screenAt(pos))
  {
    const QRect available = screen->availableGeometry();
    const QSize popupSize = popup_->sizeHint();

    if (pos.x() + popupSize.width() > available.right())
      pos.setX(std::max(available.left(), available.right() - popupSize.width()));

    if (pos.y() + popupSize.height() > available.bottom())
      pos.setY(mapToGlobal(QPoint(0, -popupSize.height() - 2)).y());
  }

  popup_->move(pos);
  popup_->show();
#ifdef Q_OS_ANDROID
  popup_->raise();
#endif
  slider_->setFocus(Qt::PopupFocusReason);
}
