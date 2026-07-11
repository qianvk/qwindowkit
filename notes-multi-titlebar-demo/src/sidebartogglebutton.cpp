#include "sidebartogglebutton.h"

#include <QEnterEvent>
#include <QEasingCurve>
#include <QPainter>
#include <QPaintEvent>
#include <QPropertyAnimation>

#include <algorithm>

SidebarToggleButton::SidebarToggleButton(QWidget *parent)
    : QPushButton(parent)
{
    setObjectName(QStringLiteral("sidebarToggleButton"));
    setFocusPolicy(Qt::NoFocus);
    setFlat(true);
    setCursor(Qt::ArrowCursor);
    setFixedSize(30, 30);
    setAttribute(Qt::WA_Hover, true);

    m_hoverAnimation = new QPropertyAnimation(this, "hoverProgress", this);
    m_hoverAnimation->setDuration(120);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void SidebarToggleButton::setHoverProgress(qreal value)
{
    const qreal clamped = std::clamp(value, 0.0, 1.0);
    if (qFuzzyCompare(m_hoverProgress, clamped)) {
        return;
    }
    m_hoverProgress = clamped;
    update();
}

void SidebarToggleButton::enterEvent(QEnterEvent *event)
{
    animateHover(1.0);
    QPushButton::enterEvent(event);
}

void SidebarToggleButton::leaveEvent(QEvent *event)
{
    animateHover(0.0);
    QPushButton::leaveEvent(event);
}

void SidebarToggleButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, static_cast<int>(28 * m_hoverProgress)));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 7, 7);

    painter.setPen(QPen(QColor(49, 50, 54), 1.5, Qt::SolidLine,
                        Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    const QRectF glyph = QRectF(rect()).adjusted(7.5, 8.5, -7.5, -8.5);
    painter.drawRoundedRect(glyph, 2.0, 2.0);
    painter.drawLine(QPointF(glyph.left() + 4.5, glyph.top()),
                     QPointF(glyph.left() + 4.5, glyph.bottom()));
}

void SidebarToggleButton::animateHover(qreal endValue)
{
    m_hoverAnimation->stop();
    m_hoverAnimation->setStartValue(m_hoverProgress);
    m_hoverAnimation->setEndValue(endValue);
    m_hoverAnimation->start();
}
