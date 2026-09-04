#pragma once

#include <QString>
#include <QVariantList>

namespace NaadaLab
{

class ManualDocumentParser
{
public:
  static QVariantList loadFromResource(
    const QString& resourcePath,
    const QString& imageBaseUrl = QStringLiteral("qrc:/manual/"));
};

}