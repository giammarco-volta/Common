#include "CenterFillSlider.h"
#include <QStylePainter>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionSlider>

CenterFillSlider::CenterFillSlider(Qt::Orientation ori, QWidget* parent)
  : QSlider(ori, parent)
{
  // opzionale: tick di default OFF, perché la tacca centrale la disegniamo noi.
  setTickPosition(QSlider::NoTicks);
  setObjectName("centerFillSlider");
}

int CenterFillSlider::centerValue() const
{
  return 0; // neutro = 0 (equabile)
}

void CenterFillSlider::paintEvent(QPaintEvent* e)
{
  Q_UNUSED(e);

  // 1) Fai disegnare al QStyle (quindi anche QSS) groove + handle + tick, ecc.
  QStylePainter p(this);
  QStyleOptionSlider opt;
  initStyleOption(&opt);

  // Disegna lo slider "standard" (QSS incluso)
  p.drawComplexControl(QStyle::CC_Slider, opt);

  // IMPORTANTISSIMO: includi l'handle qui, altrimenti handleRect può risultare vuoto
  opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle | QStyle::SC_SliderTickmarks;

  // 2) Calcola geometrie di groove e handle
  const QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt,
    QStyle::SC_SliderGroove, this);
  const QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &opt,
    QStyle::SC_SliderHandle, this);

  if (grooveRect.isEmpty() || handleRect.isEmpty())
    return;

  // Centro logico (metà range). Se vuoi un centro diverso, sostituisci qui.
  const int centerValue = (minimum() + maximum()) / 2;

  // 3) Converti value -> pixel sul groove (centro del cursore)
  auto valueToX = [&](int v) -> int {
    // span è la corsa effettiva del cursore dentro il groove
    const int span = grooveRect.width() - handleRect.width();
    const int pos = QStyle::sliderPositionFromValue(minimum(), maximum(), v, span, opt.upsideDown);
    // pos è l'offset del "left" dell'handle dentro il groove.
    return grooveRect.x() + pos + handleRect.width() / 2;
    };

  const int xCenter = valueToX(centerValue);
  const int xHandle = handleRect.center().x();

  // 4) Rettangolo fill tra centro e handle
  const int x1 = qMin(xCenter, xHandle);
  const int x2 = qMax(xCenter, xHandle);

  // Usa l'altezza del groove, centrata verticalmente sul groove
  QRect fillRect(QPoint(x1, grooveRect.center().y() - grooveRect.height() / 2),
    QPoint(x2, grooveRect.center().y() + grooveRect.height() / 2));

  // Evita spessori strani
  fillRect = fillRect.normalized();
  if (fillRect.width() < 2)
    return;

  // 5) Colore fill: puoi scegliere tu (qui uso palette + stato enabled/disabled)
  QColor fillColor = palette().color(isEnabled() ? QPalette::Normal : QPalette::Disabled, QPalette::Highlight);
  // Se vuoi un colore “fisso”, metti direttamente QColor("#...")

  p.save();
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setPen(Qt::NoPen);
  p.setBrush(fillColor);
  p.drawRoundedRect(fillRect.adjusted(0, 0, -1, -1), grooveRect.height() / 2.0, grooveRect.height() / 2.0);
  p.restore();

  // 6) (Opzionale) Tacca centrale
  const QColor tickColor = palette().color(isEnabled() ? QPalette::Normal : QPalette::Disabled, QPalette::Text);

  p.save();
  p.setPen(QPen(tickColor, 1));
  const int y1 = grooveRect.top() - 6;
  const int y2 = grooveRect.bottom() + 6;
  p.drawLine(QPoint(xCenter, y1), QPoint(xCenter, y2));
  p.restore();
}
/*
void CenterFillSlider::paintEvent(QPaintEvent*)
{
  QStylePainter p(this);
  QStyleOptionSlider opt;
  initStyleOption(&opt);

  // 1) Disegna groove + tickmarks (ma NON handle)
  opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderTickmarks;
  p.drawComplexControl(QStyle::CC_Slider, opt);

  const QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt,
    QStyle::SC_SliderGroove, this);
  const QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &opt,
    QStyle::SC_SliderHandle, this);
  if (grooveRect.isEmpty() || handleRect.isEmpty())
    return;

  // 2) Overlay fill centro <-> cursore
  const int centerValue = (minimum() + maximum()) / 2;

  auto valueToX = [&](int v) -> int {
    const int span = grooveRect.width() - handleRect.width();
    const int pos = QStyle::sliderPositionFromValue(minimum(), maximum(), v, span, opt.upsideDown);
    return grooveRect.x() + pos + handleRect.width() / 2;
    };

  const int xCenter = valueToX(centerValue);
  const int xHandle = handleRect.center().x();
  const int x1 = qMin(xCenter, xHandle);
  const int x2 = qMax(xCenter, xHandle);

  QRect fillRect(QPoint(x1, grooveRect.top()), QPoint(x2, grooveRect.bottom()));
  fillRect = fillRect.normalized();

  if (fillRect.width() >= 2) {
    const QColor fillColor = palette().color(
      isEnabled() ? QPalette::Normal : QPalette::Disabled,
      QPalette::Highlight
    );

    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(fillColor);
    p.drawRoundedRect(fillRect.adjusted(0, 0, -1, -1),
      grooveRect.height() / 2.0, grooveRect.height() / 2.0);
    p.restore();
  }

  // Tacca centrale
  {
    const QColor tickColor = palette().color(
      isEnabled() ? QPalette::Normal : QPalette::Disabled,
      QPalette::Text
    );

    p.save();
    p.setPen(QPen(tickColor, 1));
    p.drawLine(QPoint(xCenter, grooveRect.top() - 6),
      QPoint(xCenter, grooveRect.bottom() + 6));
    p.restore();
  }

  // 3) Disegna SOLO l'handle sopra tutto
  opt.subControls = QStyle::SC_SliderHandle;
  p.drawComplexControl(QStyle::CC_Slider, opt);

#ifdef DISEGNA_TICKMARKS
  // --- Tickmarks manuali (sempre visibili) ---
  if (tickPosition() != QSlider::NoTicks) {
    const int interval = (tickInterval() > 0) ? tickInterval() : pageStep();
    if (interval > 0) {
      const QColor tickColor = palette().color(
        isEnabled() ? QPalette::Normal : QPalette::Disabled,
        QPalette::Text
      );

      p.save();
      p.setPen(QPen(tickColor, 1));

      const int tickLen = 4;
      const bool below = (tickPosition() == QSlider::TicksBelow || tickPosition() == QSlider::TicksBothSides);
      const bool above = (tickPosition() == QSlider::TicksAbove || tickPosition() == QSlider::TicksBothSides);

      const int yBelow1 = grooveRect.bottom() + 3;
      const int yBelow2 = yBelow1 + tickLen;

      const int yAbove2 = grooveRect.top() - 3;
      const int yAbove1 = yAbove2 - tickLen;

      // allinea ai "multipli" di interval
      int start = minimum();
      if (interval > 1) {
        const int rem = (start - minimum()) % interval;
        if (rem != 0) start += (interval - rem);
      }

      for (int v = minimum(); v <= maximum(); v += interval) {
        const int x = valueToX(v);

        if (below) p.drawLine(QPoint(x, yBelow1), QPoint(x, yBelow2));
        if (above) p.drawLine(QPoint(x, yAbove1), QPoint(x, yAbove2));
      }

      p.restore();
    }
  }
#endif
}
*/
/*
void CenterFillSlider::paintEvent(QPaintEvent*)
{
  QStyleOptionSlider opt;
  initStyleOption(&opt);

  // 1) Base (QSS): groove + (eventuali) cose standard, ma NON handle
  {
    QStylePainter sp(this);
    opt.subControls = QStyle::SC_SliderGroove;  // (tick standard spesso inutili, tu li disegni a mano)
    sp.drawComplexControl(QStyle::CC_Slider, opt);
  }

  // Geometrie (serve lo style)
  const QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt,
    QStyle::SC_SliderGroove, this);
  const QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &opt,
    QStyle::SC_SliderHandle, this);
  if (grooveRect.isEmpty() || handleRect.isEmpty())
    return;

  auto valueToX = [&](int v) -> int {
    const int span = grooveRect.width() - handleRect.width();
    const int pos = QStyle::sliderPositionFromValue(minimum(), maximum(), v, span, opt.upsideDown);
    return grooveRect.x() + pos + handleRect.width() / 2;
    };

  const int centerValue = (minimum() + maximum()) / 2;
  const int xCenter = valueToX(centerValue);
  const int xHandle = handleRect.center().x();

  // 2) Overlay (tuo): fill + tacca + tick manuali
  {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // >>> qui scegli un colore NON dipendente dalla palette (consigliato)
    const QColor fillColor = isEnabled() ? QColor("#3daee9") : QColor("#606060");

    const int x1 = qMin(xCenter, xHandle);
    const int x2 = qMax(xCenter, xHandle);

    QRect fillRect(QPoint(x1, grooveRect.top()), QPoint(x2, grooveRect.bottom()));
    fillRect = fillRect.normalized();

    if (fillRect.width() >= 2) {
      p.setPen(Qt::NoPen);
      p.setBrush(fillColor);
      p.drawRoundedRect(fillRect.adjusted(0, 0, -1, -1),
        grooveRect.height() / 2.0, grooveRect.height() / 2.0);
    }

    // tacca centrale
    const QColor tickColor = isEnabled() ? QColor("#e6e6e6") : QColor("#7a7a7a");
    p.setPen(QPen(tickColor, 1));
    p.drawLine(QPoint(xCenter, grooveRect.top() - 6),
      QPoint(xCenter, grooveRect.bottom() + 6));

#ifdef DISEGNA_TICKMARKS
    // tick manuali (se vuoi)
    if (tickPosition() != QSlider::NoTicks) {
      const int interval = (tickInterval() > 0) ? tickInterval() : pageStep();
      if (interval > 0) {
        const int tickLen = 4;
        const bool below = (tickPosition() == QSlider::TicksBelow || tickPosition() == QSlider::TicksBothSides);
        const bool above = (tickPosition() == QSlider::TicksAbove || tickPosition() == QSlider::TicksBothSides);

        const int yBelow1 = grooveRect.bottom() + 3;
        const int yBelow2 = yBelow1 + tickLen;
        const int yAbove2 = grooveRect.top() - 3;
        const int yAbove1 = yAbove2 - tickLen;

        for (int v = minimum(); v <= maximum(); v += interval) {
          const int x = valueToX(v);
          if (below) p.drawLine(QPoint(x, yBelow1), QPoint(x, yBelow2));
          if (above) p.drawLine(QPoint(x, yAbove1), QPoint(x, yAbove2));
        }
      }
    }
#endif
  }

  // 3) Handle sopra tutto (QSS)
  {
    QStylePainter sp(this);
    opt.subControls = QStyle::SC_SliderHandle;
    sp.drawComplexControl(QStyle::CC_Slider, opt);
  }
}*/
/*
void CenterFillSlider::paintEvent(QPaintEvent*)
{
  QStyleOptionSlider optAll;
  initStyleOption(&optAll);

  // IMPORTANTISSIMO: includi l'handle qui, altrimenti handleRect può risultare vuoto
  optAll.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle | QStyle::SC_SliderTickmarks;

  const QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &optAll,
    QStyle::SC_SliderGroove, this);
  const QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &optAll,
    QStyle::SC_SliderHandle, this);

  if (grooveRect.isEmpty() || handleRect.isEmpty())
    return;

  // 1) Disegna base (groove) con QSS, SENZA handle
  {
    QStylePainter sp(this);
    QStyleOptionSlider opt = optAll;
    opt.subControls = QStyle::SC_SliderGroove;            // qui puoi anche aggiungere tickmarks se ti servono
    sp.drawComplexControl(QStyle::CC_Slider, opt);
  }

  auto valueToX = [&](int v) -> int {
    const int span = grooveRect.width() - handleRect.width();
    const int pos = QStyle::sliderPositionFromValue(minimum(), maximum(), v, span, optAll.upsideDown);
    return grooveRect.x() + pos + handleRect.width() / 2;
    };

  const int centerValue = (minimum() + maximum()) / 2;
  const int xCenter = valueToX(centerValue);
  const int xHandle = handleRect.center().x();

  // 2) Overlay (fill + tacca centrale + tick manuali se vuoi)
  {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QColor fillColor = isEnabled() ? QColor("#3daee9") : QColor("#606060");
    const QColor tickColor = isEnabled() ? QColor("#e6e6e6") : QColor("#7a7a7a");

    const int x1 = qMin(xCenter, xHandle);
    const int x2 = qMax(xCenter, xHandle);

    QRect fillRect(QPoint(x1, grooveRect.top()), QPoint(x2, grooveRect.bottom()));
    fillRect = fillRect.normalized();

    if (fillRect.width() >= 2) {
      p.setPen(Qt::NoPen);
      p.setBrush(fillColor);
      p.drawRoundedRect(fillRect.adjusted(0, 0, -1, -1),
        grooveRect.height() / 2.0, grooveRect.height() / 2.0);
    }

    p.setPen(QPen(tickColor, 1));
    p.drawLine(QPoint(xCenter, grooveRect.top() - 6),
      QPoint(xCenter, grooveRect.bottom() + 6));

    // (Se già ti funzionano i tick manuali, rimettili qui)
  }

  // 3) Disegna handle sopra tutto (QSS)
  {
    QStylePainter sp(this);
    QStyleOptionSlider opt = optAll;
    opt.subControls = QStyle::SC_SliderHandle;
    sp.drawComplexControl(QStyle::CC_Slider, opt);
  }
}
*/
/*
void CenterFillSlider::paintEvent(QPaintEvent* e)
{
  Q_UNUSED(e);

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  QStyleOptionSlider optAll;
  initStyleOption(&optAll);
  optAll.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle | QStyle::SC_SliderTickmarks;

  const QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &optAll,
    QStyle::SC_SliderGroove, this);
  const QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &optAll,
    QStyle::SC_SliderHandle, this);

  // --- Disegno "base" senza handle (QSS)
  {
    QStyleOptionSlider opt = optAll;
    opt.subControls = QStyle::SC_SliderGroove; // + tickmarks standard se vuoi, ma li disegniamo a mano
    style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
  }

  // ===== DIAGNOSTICA: una riga rossa che DEVE vedersi sempre =====
  p.save();
  p.setPen(QPen(Qt::red, 1));
  p.drawLine(0, 0, width(), 0);
  p.restore();
  // ===============================================================

  if (grooveRect.isEmpty() || handleRect.isEmpty()) {
    // Se per qualche motivo qui è vuoto, almeno vedi la riga rossa sopra.
    // E disegniamo comunque l'handle standard:
    QStyleOptionSlider opt = optAll;
    opt.subControls = QStyle::SC_SliderHandle;
    style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
    return;
  }

  // value -> X (orizzontale)
  auto valueToX = [&](int v) -> int {
    const int span = grooveRect.width() - handleRect.width();
    const int pos = QStyle::sliderPositionFromValue(minimum(), maximum(), v, span, optAll.upsideDown);
    return grooveRect.x() + pos + handleRect.width() / 2;
    };

  const int centerValue = (minimum() + maximum()) / 2;
  const int xCenter = valueToX(centerValue);
  const int xHandle = handleRect.center().x();

  // --- Overlay fill tra centro e handle (colori volutamente forti)
  const int x1 = qMin(xCenter, xHandle);
  const int x2 = qMax(xCenter, xHandle);

  QRect fillRect(QPoint(x1, grooveRect.top()), QPoint(x2, grooveRect.bottom()));
  fillRect = fillRect.normalized();

  if (fillRect.width() >= 2) {
    const QColor fillColor = isEnabled() ? QColor("#00c0ff") : QColor("#808080"); // visibile sicuro
    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(fillColor);
    p.drawRoundedRect(fillRect.adjusted(0, 0, -1, -1),
      grooveRect.height() / 2.0, grooveRect.height() / 2.0);
    p.restore();
  }

  // --- Tacca centrale (gialla, super visibile)
  {
    p.save();
    p.setPen(QPen(Qt::yellow, 2));
    p.drawLine(QPoint(xCenter, grooveRect.top() - 8),
      QPoint(xCenter, grooveRect.bottom() + 8));
    p.restore();
  }

  // --- Tick manuali (se abilitati)
  if (tickPosition() != QSlider::NoTicks) {
    const int interval = (tickInterval() > 0) ? tickInterval() : pageStep();
    if (interval > 0) {
      p.save();
      p.setPen(QPen(isEnabled() ? QColor("#e6e6e6") : QColor("#7a7a7a"), 1));

      const int tickLen = 4;
      const bool below = (tickPosition() == QSlider::TicksBelow || tickPosition() == QSlider::TicksBothSides);
      const bool above = (tickPosition() == QSlider::TicksAbove || tickPosition() == QSlider::TicksBothSides);

      const int yBelow1 = grooveRect.bottom() + 3;
      const int yBelow2 = yBelow1 + tickLen;

      const int yAbove2 = grooveRect.top() - 3;
      const int yAbove1 = yAbove2 - tickLen;

      for (int v = minimum(); v <= maximum(); v += interval) {
        const int x = valueToX(v);
        if (below) p.drawLine(QPoint(x, yBelow1), QPoint(x, yBelow2));
        if (above) p.drawLine(QPoint(x, yAbove1), QPoint(x, yAbove2));
      }
      p.restore();
    }
  }

  // --- Disegno handle sopra tutto (QSS)
  {
    QStyleOptionSlider opt = optAll;
    opt.subControls = QStyle::SC_SliderHandle;
    style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
  }
}
*/