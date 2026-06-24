#pragma once
#include <QDial>
#include <QColor>

class ArcDial : public QDial
{
    Q_OBJECT

public:
    enum class NeutralMode
    {
      Center,   // es: -64 … +63 → neutro = 0 al centro
      Minimum   // es: 0 … 127 → neutro = minimo (0)
    };

    explicit ArcDial(NeutralMode mode, QWidget* parent = nullptr);

    int neutralValue() const;
    double normalizedValue() const;

    void setCenterValue(int v) { m_centerValue = v; update(); }
    int  centerValue() const { return m_centerValue; }

    void setSelectedColor(const QColor& c) { m_selected = c; update(); }
    void setUnselectedColor(const QColor& c) { m_unselected = c; update(); }
    void setTrackWidth(int px) { m_width = px; update(); }
    void setPadding(int px) { m_padding = px; update(); }

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    int   m_centerValue = std::numeric_limits<int>::min(); // auto = mid(range)
    QColor m_selected   = QColor("#ffaa00"); // bello con qdarkstyle
    QColor m_unselected = QColor(90, 90, 90);
    int   m_width = 6;
    int   m_padding = 6;

    static double norm(int v, int minv, int maxv);
    static double lerp(double a, double b, double t);

    NeutralMode neutralMode_ = NeutralMode::Center;
};
