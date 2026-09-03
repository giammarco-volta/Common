#include "TouchComboBox.h"

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
#include <QApplication>
#include <QFrame>
#include <QListWidget>
#include <QVBoxLayout>
#include <QScreen>
#endif

TouchComboBox::TouchComboBox(QWidget* parent)
  : QComboBox(parent)
{
}

void TouchComboBox::showPopup()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
  showTouchPopup();
#else
  QComboBox::showPopup();
#endif
}

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)

void TouchComboBox::showTouchPopup()
{
  if (!popup_)
  {
    popup_ = new QFrame(nullptr, Qt::Tool | Qt::FramelessWindowHint);
    popup_->setObjectName(QStringLiteral("TouchComboBoxPopup"));
    popup_->setAttribute(Qt::WA_StyledBackground, true);
    popup_->setAutoFillBackground(true);

    auto* layout = new QVBoxLayout(popup_);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(0);

    list_ = new QListWidget(popup_);
    list_->setObjectName(QStringLiteral("TouchComboBoxList"));
    list_->setSelectionMode(QAbstractItemView::SingleSelection);

    layout->addWidget(list_);

    connect(list_, &QListWidget::itemClicked, this,
      [this](QListWidgetItem* item)
      {
        const int row = list_->row(item);
        if (row >= 0 && row < count())
          setCurrentIndex(row);

        popup_->hide();
      });
  }

  list_->clear();

  for (int i = 0; i < count(); ++i)
  {
    auto* item = new QListWidgetItem(itemText(i), list_);
    item->setSizeHint(QSize(0, 52));

    if (i == currentIndex())
      item->setSelected(true);
  }

  const int visibleRows = qMin(count(), 8);
  const int rowHeight = 56;
  const int popupWidth = qMax(width(), 260);
  const int popupHeight = visibleRows * rowHeight + 16;

  popup_->resize(popupWidth, popupHeight);

  QPoint pos = mapToGlobal(QPoint(0, height() + 2));

  if (const QScreen* screen = QApplication::screenAt(pos))
  {
    const QRect available = screen->availableGeometry();

    if (pos.x() + popup_->width() > available.right())
      pos.setX(qMax(available.left(), available.right() - popup_->width()));

    if (pos.y() + popup_->height() > available.bottom())
      pos.setY(mapToGlobal(QPoint(0, -popup_->height() - 2)).y());

    if (pos.y() < available.top())
      pos.setY(available.top());
  }

  popup_->move(pos);
  popup_->show();
  popup_->raise();
  list_->setFocus();
}

#endif