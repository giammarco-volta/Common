#include "ArcDial.h"
#include <QPainter>
#include <QtMath>
#include <algorithm>

//-------------------------------------------------------------------------------------
ArcDial::ArcDial(NeutralMode mode, QWidget* parent) : QDial(parent), neutralMode_(mode)
//-------------------------------------------------------------------------------------
{
    // Evita "extra" inutili
    setNotchesVisible(false);
    setWrapping(false);

    // Così il widget non riempie uno sfondo proprio
    setAttribute(Qt::WA_TranslucentBackground, true);
}

//-------------------------------
int ArcDial::neutralValue() const
//-------------------------------
{
  if (neutralMode_ == NeutralMode::Center)
    return (minimum() + maximum()) / 2;
  else
    return minimum();
}

//-------------------------------------
double ArcDial::normalizedValue() const
//-------------------------------------
{
  int v = value();
  int n = neutralValue();

  if (neutralMode_ == NeutralMode::Center)
  {
    if (v >= n)
      return double(v - n) / (maximum() - n);
    else
      return double(v - n) / (n - minimum());
  }
  else
  {
    return double(value() - n) / (maximum() - n);
  }
}

//---------------------------------------------
double ArcDial::norm(int v, int minv, int maxv)
//---------------------------------------------
{
    if (maxv == minv) return 0.0;
    return (double)(v - minv) / (double)(maxv - minv);
}

//------------------------------------------------
double ArcDial::lerp(double a, double b, double t)
//------------------------------------------------
{
    return a + (b - a) * t;
}

//------------------------------------
void ArcDial::paintEvent(QPaintEvent*)
//------------------------------------
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int minv = minimum();
    const int maxv = maximum();
    const int val  = value();

    int cval = m_centerValue;
    if (cval == std::numeric_limits<int>::min())
      cval = neutralValue();

    const double tMin = 0.0;
    const double tMax = 1.0;
    const double tVal = std::clamp(norm(val,  minv, maxv), tMin, tMax);
    const double tCen = std::clamp(norm(cval, minv, maxv), tMin, tMax);

    // Angoli “da dial normale”: gap in basso, metà corsa in alto.
    // 240° (bottom-left) -> -60° (bottom-right) in senso orario = -300°.
    const double startDeg = 240.0;
    const double spanDeg = -300.0;

    auto angleAt = [&](double t) {
      return startDeg + spanDeg * t;
      };

    const double a0 = angleAt(tMin);
    const double a1 = angleAt(tMax);

    const double aC = angleAt(tCen);
    const double aV = angleAt(tVal);

    // Rettangolo dell’arco
    const int s = std::min(width(), height());
    const int margin = m_padding + m_width;
    QRectF r((width()  - s) / 2 + margin,
             (height() - s) / 2 + margin,
             s - 2*margin,
             s - 2*margin);

    QPen pen;
    pen.setWidth(m_width);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);

    auto drawArcSegT = [&](double tFrom, double tTo, const QColor& col) {
      tFrom = std::clamp(tFrom, 0.0, 1.0);
      tTo = std::clamp(tTo, 0.0, 1.0);
      if (qFuzzyCompare(tFrom, tTo)) return;

      QPen pen;
      pen.setWidth(m_width);
      pen.setCapStyle(Qt::RoundCap);
      pen.setColor(col);
      p.setPen(pen);

      const double aStart = angleAt(tFrom);
      const double aSpan = spanDeg * (tTo - tFrom);   // mantiene il verso corretto

      p.drawArc(r, int(aStart * 16.0), int(aSpan * 16.0));
      };

    // 1) tutto “non selezionato”
    drawArcSegT(0.0, 1.0, m_unselected);

    // 2) tratto selezionato tra centro e valore (in entrambi i versi)
    drawArcSegT(tCen, tVal, m_selected);

    // Se vuoi anche un piccolo “puntino” sul valore (cursore invisibile ma leggibile):
    // (commenta se vuoi zero marker)

    const double aV2 = angleAt(tVal);
    const double rad = qDegreesToRadians(aV2);

    const QPointF center = r.center();
    const double rr = r.width() / 2.0;

    const QPointF pt(center.x() + rr * std::cos(rad),
                     center.y() - rr * std::sin(rad));

    p.setBrush(m_selected);
    p.setPen(Qt::NoPen);
    p.drawEllipse(pt, m_width * 0.75, m_width * 0.75);
}
