#include "mainwindow.h"

#include "sidebartogglebutton.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

#include <QWKWidgets/widgetwindowagent.h>

#include <algorithm>

namespace {

    constexpr int kTrafficLightAreaWidth = 72;
    constexpr int kTrafficLightAreaHeight = 32;
    constexpr int kToolbarButtonHeight = 30;
    constexpr int kOverlayMargin = 12;

}

void MainWindow::addPlatformButtonLabControls(QVBoxLayout *layout, QWidget *page)
{
    layout->addSpacing(18);

    auto *sectionLabel = new QLabel(QStringLiteral("TRAFFIC LIGHT POSITION"), page);
    sectionLabel->setObjectName(QStringLiteral("labSectionLabel"));
    layout->addWidget(sectionLabel);

    auto addSlider = [this, page, layout](const QString &labelText, int minimum, int maximum,
                                          int value, QSlider **target, auto onValueChanged) {
        auto *row = new QWidget(page);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(12);

        auto *label = new QLabel(labelText, row);
        label->setMinimumWidth(92);
        auto *slider = new QSlider(Qt::Horizontal, row);
        slider->setRange(minimum, maximum);
        slider->setValue(value);
        slider->setFixedWidth(220);
        auto *valueLabel = new QLabel(QString::number(value), row);
        valueLabel->setObjectName(QStringLiteral("sliderValue"));
        valueLabel->setMinimumWidth(24);

        rowLayout->addWidget(label);
        rowLayout->addWidget(slider);
        rowLayout->addWidget(valueLabel);
        rowLayout->addStretch(1);
        layout->addWidget(row);

        connect(slider, &QSlider::valueChanged, this,
                [valueLabel, onValueChanged](int newValue) {
                    valueLabel->setText(QString::number(newValue));
                    onValueChanged(newValue);
                });
        *target = slider;
    };

    addSlider(QStringLiteral("Horizontal"), 0, 28, m_trafficLightX,
              &m_trafficLightXSlider, [this](int value) {
                  m_trafficLightX = value;
                  layoutOverlayChrome();
              });
    addSlider(QStringLiteral("Vertical"), 0, 16, m_trafficLightY,
              &m_trafficLightYSlider, [this](int value) {
                  m_trafficLightY = value;
                  layoutOverlayChrome();
              });
}

QMargins MainWindow::platformFoldersTitleMargins() const
{
    return {kTrafficLightAreaWidth + 12, 11, kToolbarButtonHeight + 20, 11};
}

QMargins MainWindow::platformEditorTitleMargins() const
{
    return {14, 11, 14, 11};
}

int MainWindow::platformSidebarToggleX(int splitterX, int folderWidth) const
{
    const QRect systemButtonArea = m_windowAgent->systemButtonAreaGeometry();
    const int collapsedX = systemButtonArea.isValid()
                               ? systemButtonArea.right() + 8
                               : kTrafficLightAreaWidth + 8;
    const int expandedX = splitterX + folderWidth -
                          kOverlayMargin - m_sidebarToggleButton->width();
    return std::max(collapsedX, expandedX);
}

int MainWindow::platformMinimumFolderChromeWidth() const
{
    return kTrafficLightAreaWidth + (kToolbarButtonHeight * 2) + 42;
}

void MainWindow::updatePlatformChrome()
{
    m_windowAgent->setSystemButtonAreaGeometry(
        QRect(m_trafficLightX, m_trafficLightY,
              kTrafficLightAreaWidth, kTrafficLightAreaHeight));
}
