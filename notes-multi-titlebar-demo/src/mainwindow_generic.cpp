#include "mainwindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace {

    constexpr int kToolbarButtonHeight = 30;
    constexpr int kOverlayMargin = 12;

}

void MainWindow::addPlatformButtonLabControls(QVBoxLayout *layout, QWidget *page)
{
    layout->addSpacing(18);

    auto *sectionLabel = new QLabel(QStringLiteral("PLATFORM WINDOW CONTROLS"), page);
    sectionLabel->setObjectName(QStringLiteral("labSectionLabel"));
    layout->addWidget(sectionLabel);

    auto *description = new QLabel(
        QStringLiteral("This platform uses application-provided window controls."), page);
    description->setObjectName(QStringLiteral("visibilityStatus"));
    layout->addWidget(description);
}

QMargins MainWindow::platformFoldersTitleMargins() const
{
    return {kToolbarButtonHeight + 20, 11, 12, 11};
}

QMargins MainWindow::platformEditorTitleMargins() const
{
    return {14, 11, 14, 11};
}

int MainWindow::platformSidebarToggleX(int splitterX, int folderWidth) const
{
    Q_UNUSED(folderWidth)
    return splitterX + kOverlayMargin;
}

int MainWindow::platformMinimumFolderChromeWidth() const
{
    return (kToolbarButtonHeight * 2) + 42;
}

void MainWindow::updatePlatformChrome()
{
}
