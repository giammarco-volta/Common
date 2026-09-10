#include "ManualDocumentParser.h"

#include <QFile>
#include <QImageReader>
#include <QRegularExpression>
#include <QVector>
#include <QSizeF>
#include <QSvgRenderer>
#include <QVariantMap>

namespace
{

QString normalizedBaseUrl(QString baseUrl)
{
  baseUrl = baseUrl.trimmed();

  if (baseUrl.isEmpty())
    baseUrl = QStringLiteral("qrc:/manual/");

  if (!baseUrl.endsWith(QLatin1Char('/')))
    baseUrl.append(QLatin1Char('/'));

  return baseUrl;
}

QString manualImageSourceToUrl(
  QString source,
  const QString& imageBaseUrl)
{
  source = source.trimmed();

  if (source.startsWith(QStringLiteral("qrc:/"))
      || source.startsWith(QStringLiteral(":/"))
      || source.startsWith(QStringLiteral("http://"))
      || source.startsWith(QStringLiteral("https://")))
  {
    return source;
  }

  if (source.startsWith(QStringLiteral("./")))
    source.remove(0, 2);

  const QString baseUrl = normalizedBaseUrl(imageBaseUrl);

  if (source.startsWith(QStringLiteral("images/")))
    return baseUrl + source;

  return baseUrl + QStringLiteral("images/") + source;
}

bool isInlineManualImage(const QString& imageTag)
{
  static const QRegularExpression inlineImageRe(
    QStringLiteral(
      "\\bclass\\s*=\\s*[\"'][^\"']*"
      "\\bmanual-inline-image\\b[^\"']*[\"']"),
    QRegularExpression::CaseInsensitiveOption);

  return inlineImageRe.match(imageTag).hasMatch();
}

QString decorateManualTextFragment(QString html)
{
  html.remove(QRegularExpression(
    QStringLiteral("<!DOCTYPE[^>]*>"),
    QRegularExpression::CaseInsensitiveOption));

  html.remove(QRegularExpression(
    QStringLiteral("<head[\\s\\S]*?</head>"),
    QRegularExpression::CaseInsensitiveOption));

  html.remove(QRegularExpression(
    QStringLiteral("</?html[^>]*>"),
    QRegularExpression::CaseInsensitiveOption));

  html.replace(QRegularExpression(
    QStringLiteral("<body[^>]*>"),
    QRegularExpression::CaseInsensitiveOption),
    QString());

  html.replace(QRegularExpression(
    QStringLiteral("</body>"),
    QRegularExpression::CaseInsensitiveOption),
    QString());

  return QStringLiteral(R"(
<style>
  body, p, div, span, li, td {
    color: #E8E8E8;
    background-color: transparent;
  }

  h1, h2, h3 {
    color: #D8B85A;
  }

  b, strong {
    color: #FFFFFF;
  }

  i {
    color: #B8B8B8;
  }

  a {
    color: #D8B85A;
    text-decoration: none;
    font-weight: bold;
  }
</style>
<div style="color:#E8E8E8;">
)")
    + html
    + QStringLiteral("</div>");
}

void appendManualTextBlocks(
  QVariantList& blocks,
  const QString& html)
{
  static const QRegularExpression anchorTagRe(
    QStringLiteral(
      R"(<[A-Za-z][A-Za-z0-9]*\b[^>]*\bid\s*=\s*["']([^"']+)["'][^>]*>)"),
    QRegularExpression::CaseInsensitiveOption);

  QVector<QRegularExpressionMatch> anchorMatches;
  auto matches = anchorTagRe.globalMatch(html);

  while (matches.hasNext())
    anchorMatches.append(matches.next());

  auto appendBlock = [&blocks](
                       const QString& fragment,
                       const QString& anchor = QString())
  {
    const QString trimmedFragment = fragment.trimmed();

    if (trimmedFragment.isEmpty())
      return;

    QVariantMap textBlock;
    textBlock["type"] = QStringLiteral("text");
    textBlock["html"] =
      decorateManualTextFragment(trimmedFragment);

    if (!anchor.isEmpty())
      textBlock["anchor"] = anchor;

    blocks.append(textBlock);
  };

  if (anchorMatches.isEmpty())
  {
    appendBlock(html);
    return;
  }

  appendBlock(html.left(anchorMatches.first().capturedStart()));

  for (qsizetype i = 0; i < anchorMatches.size(); ++i)
  {
    const qsizetype start = anchorMatches[i].capturedStart();
    const qsizetype end = i + 1 < anchorMatches.size()
      ? anchorMatches[i + 1].capturedStart()
      : html.size();

    appendBlock(
      html.mid(start, end - start),
      anchorMatches[i].captured(1).trimmed());
  }
}

void attachAnchorIndexes(QVariantList& blocks)
{
  QVariantMap anchorIndexes;

  for (qsizetype i = 0; i < blocks.size(); ++i)
  {
    const QVariantMap block = blocks.at(i).toMap();
    const QString anchor = block.value("anchor").toString();

    if (!anchor.isEmpty())
      anchorIndexes.insert(anchor, i);
  }

  if (anchorIndexes.isEmpty())
    return;

  for (qsizetype i = 0; i < blocks.size(); ++i)
  {
    QVariantMap block = blocks.at(i).toMap();
    block.insert("anchorIndexes", anchorIndexes);
    blocks[i] = block;
  }
}

QString extractFirstImageSource(const QString& html)
{
  static const QRegularExpression imageRe(
    QStringLiteral(
      R"(<img\b[^>]*\bsrc\s*=\s*["']([^"']+)["'][^>]*>)"),
    QRegularExpression::CaseInsensitiveOption);

  const QRegularExpressionMatch match = imageRe.match(html);

  if (!match.hasMatch())
    return {};

  return match.captured(1).trimmed();
}

QString extractFigcaptionHtml(const QString& html)
{
  static const QRegularExpression captionRe(
    QStringLiteral(
      R"(<figcaption\b[^>]*>([\s\S]*?)</figcaption>)"),
    QRegularExpression::CaseInsensitiveOption);

  const QRegularExpressionMatch match = captionRe.match(html);

  if (!match.hasMatch())
    return {};

  return match.captured(1).trimmed();
}

QString qrcUrlToResourcePath(QString source)
{
  source = source.trimmed();

  if (source.startsWith(QStringLiteral("qrc:/")))
    return QStringLiteral(":") + source.mid(4);

  return source;
}

QSizeF manualImageNaturalSize(const QString& source)
{
  const QString resourcePath = qrcUrlToResourcePath(source);

  if (resourcePath.endsWith(
        QStringLiteral(".svg"),
        Qt::CaseInsensitive))
  {
    QSvgRenderer renderer(resourcePath);

    if (renderer.isValid())
    {
      const QSize defaultSize = renderer.defaultSize();

      if (!defaultSize.isEmpty())
        return QSizeF(defaultSize);

      const QRectF viewBox = renderer.viewBoxF();

      if (!viewBox.isEmpty())
        return viewBox.size();
    }
  }

  QImageReader reader(resourcePath);
  const QSize size = reader.size();

  if (!size.isEmpty())
    return QSizeF(size);

  return QSizeF(600.0, 400.0);
}

}

namespace NaadaLab
{

QVariantList ManualDocumentParser::loadFromResource(
  const QString& resourcePath,
  const QString& imageBaseUrl)
{
  QVariantList blocks;

  QFile file(resourcePath);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QVariantMap block;
    block["type"] = QStringLiteral("text");
    block["html"] =
      QStringLiteral(
        "<h1>User Manual</h1>"
        "<p>Unable to load the user manual.</p>");

    blocks.append(block);
    return blocks;
  }

  const QString html = QString::fromUtf8(file.readAll());

  static const QRegularExpression blockRe(
    QStringLiteral(
      R"((<figure\b[^>]*>[\s\S]*?</figure>)|(<img\b[^>]*\bsrc\s*=\s*["']([^"']+)["'][^>]*>))"),
    QRegularExpression::CaseInsensitiveOption);

  qsizetype position = 0;
  auto matches = blockRe.globalMatch(html);

  while (matches.hasNext())
  {
    const QRegularExpressionMatch match = matches.next();

    // Small inline images remain inside the RichText fragment.
    if (isInlineManualImage(match.captured(0)))
      continue;

    const qsizetype start = match.capturedStart();
    const qsizetype end = match.capturedEnd();
    const QString textPart =
      html.mid(position, start - position).trimmed();

    if (!textPart.isEmpty())
      appendManualTextBlocks(blocks, textPart);

    const QString figureHtml = match.captured(1);

    QString imageSource;
    QString captionHtml;

    if (!figureHtml.isEmpty())
    {
      imageSource = extractFirstImageSource(figureHtml);
      captionHtml = extractFigcaptionHtml(figureHtml);
    }
    else
    {
      imageSource = match.captured(3).trimmed();
    }

    if (!imageSource.isEmpty())
    {
      const QString resolvedSource =
        manualImageSourceToUrl(imageSource, imageBaseUrl);
      const QSizeF naturalSize =
        manualImageNaturalSize(resolvedSource);

      QVariantMap imageBlock;
      imageBlock["type"] = QStringLiteral("image");
      imageBlock["source"] = resolvedSource;
      imageBlock["naturalWidth"] = naturalSize.width();
      imageBlock["naturalHeight"] = naturalSize.height();

      if (!captionHtml.isEmpty())
      {
        imageBlock["captionHtml"] =
          QStringLiteral(
            "<div style=\"color:#B8B8B8;"
            " font-style:italic; font-size:13px;\">")
          + captionHtml
          + QStringLiteral("</div>");
      }

      blocks.append(imageBlock);
    }

    position = end;
  }

  const QString finalText = html.mid(position).trimmed();

  if (!finalText.isEmpty())
    appendManualTextBlocks(blocks, finalText);

  attachAnchorIndexes(blocks);

  return blocks;
}

}
