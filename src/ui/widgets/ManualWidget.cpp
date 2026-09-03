#include "ManualWidget.h"

#include <QPdfDocument>
#include <QPdfView>

#include <QVBoxLayout>
#include <QLabel>

UserManualWidget::UserManualWidget(const QString& manualResourcePath, QWidget* parent)
  : QWidget(parent)
{
  auto* layout = new QVBoxLayout(this);

  document_ = new QPdfDocument(this);
  view_ = new QPdfView(this);

  view_->setDocument(document_);
  view_->setPageMode(QPdfView::PageMode::MultiPage);
  view_->setZoomMode(QPdfView::ZoomMode::FitToWidth);

  const auto status = document_->load(manualResourcePath);

  if (status != QPdfDocument::Error::None)
  {
    auto* label = new QLabel(tr("Unable to load the user manual."), this);
    layout->addWidget(label);
    view_->hide();
    return;
  }

  layout->addWidget(view_);
}