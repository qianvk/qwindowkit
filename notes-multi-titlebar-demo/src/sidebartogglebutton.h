#pragma once

#include <QPushButton>

class QEnterEvent;
class QPropertyAnimation;

class SidebarToggleButton final : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)

public:
    explicit SidebarToggleButton(QWidget *parent = nullptr);

    qreal hoverProgress() const noexcept
    {
        return m_hoverProgress;
    }

    void setHoverProgress(qreal value);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void animateHover(qreal endValue);

private:
    qreal m_hoverProgress = 0.0;
    QPropertyAnimation *m_hoverAnimation = nullptr;
};
