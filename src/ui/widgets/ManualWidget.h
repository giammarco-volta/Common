#pragma once

#include <QWidget>
#include <QString>

class QPdfDocument;
class QPdfView;

class UserManualWidget : public QWidget
{
  Q_OBJECT

public:
  explicit UserManualWidget(const QString& manualResourcePath, QWidget* parent = nullptr);

private:
  QPdfDocument* document_ = nullptr;
  QPdfView* view_ = nullptr;
};