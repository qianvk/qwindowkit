#include "mainwindow.h"

#include "demostyle.h"
#include "sidebartogglebutton.h"

#include <QAbstractItemView>
#include <QButtonGroup>
#include <QEasingCurve>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QSplitter>
#include <QSplitterHandle>
#include <QStackedWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVariantAnimation>
#include <QVBoxLayout>

#include <QWKWidgets/widgetwindowagent.h>

#include <algorithm>

namespace {

    constexpr int kTitleBarHeight = 52;
    constexpr int kToolbarButtonHeight = 30;

    constexpr int kDefaultFoldersWidth = 230;
    constexpr int kDefaultListWidth = 330;
    constexpr int kCollapsedFoldersWidth = 0;
    constexpr int kMinimumRememberedFoldersWidth = 170;

    QPushButton *makeToolbarButton(const QString &text, QWidget *parent)
    {
        auto *button = new QPushButton(text, parent);
        button->setObjectName(QStringLiteral("toolbarButton"));
        button->setCursor(Qt::ArrowCursor);
        button->setFocusPolicy(Qt::NoFocus);
        button->setFixedHeight(kToolbarButtonHeight);
        return button;
    }

    QPushButton *makeChromeButton(const QString &text, const QString &objectName, QWidget *parent)
    {
        auto *button = new QPushButton(text, parent);
        button->setObjectName(objectName);
        button->setCursor(Qt::ArrowCursor);
        button->setFocusPolicy(Qt::NoFocus);
        button->setFlat(true);
        button->setFixedSize(kToolbarButtonHeight, kToolbarButtonHeight);
        return button;
    }

    QLabel *makeLabel(const QString &text, const QString &objectName, QWidget *parent)
    {
        auto *label = new QLabel(text, parent);
        label->setObjectName(objectName);
        return label;
    }

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    installWindowAgent();

    m_root = new QWidget(this);
    m_root->setObjectName(QStringLiteral("rootSurface"));

    auto *rootLayout = new QHBoxLayout(m_root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_splitter = new QSplitter(Qt::Horizontal, m_root);
    m_splitter->setObjectName(QStringLiteral("notesSplitter"));
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setOpaqueResize(true);
    m_splitter->setHandleWidth(1);

    m_foldersPane = createFoldersPane();
    m_notesListPane = createNotesListPane();
    m_editorPane = createEditorPane();

    m_splitter->addWidget(m_foldersPane);
    m_splitter->addWidget(m_notesListPane);
    m_splitter->addWidget(m_editorPane);
    m_splitter->setCollapsible(0, true);
    m_splitter->setCollapsible(1, false);
    m_splitter->setCollapsible(2, false);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 0);
    m_splitter->setStretchFactor(2, 1);

    rootLayout->addWidget(m_splitter);
    setCentralWidget(m_root);

    // The toggle is an overlay so it remains available after the folder pane reaches width zero.
    m_sidebarToggleButton = new SidebarToggleButton(this);
    m_sidebarToggleButton->setToolTip(QStringLiteral("Toggle folders"));
    m_sidebarToggleButton->show();

    m_foldersAnimation = new QVariantAnimation(this);
    m_foldersAnimation->setDuration(220);
    m_foldersAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    connect(m_foldersAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) {
                setFoldersPaneWidth(value.toInt());
            });
    connect(m_sidebarToggleButton, &QPushButton::clicked,
            this, &MainWindow::toggleFoldersPane);
    connect(m_splitter, &QSplitter::splitterMoved, this, [this](int, int) {
        const int width = foldersPaneWidth();
        if (width >= kMinimumRememberedFoldersWidth) {
            m_lastExpandedFoldersWidth = width;
        }
        layoutOverlayChrome();
    });
    connect(m_foldersList, &QListWidget::currentRowChanged,
            this, &MainWindow::updateSelectedFolder);

    connect(m_newFolderButton, &QPushButton::clicked, this, [this]() {
        auto *item = new QListWidgetItem(QStringLiteral("New Folder"), m_foldersList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        m_foldersList->setCurrentItem(item);
        m_foldersList->editItem(item);
    });

    populateNotes();
    registerTitleBarsAndHitTestWidgets();
    applyInlineStyleSheet();

    setWindowTitle(QStringLiteral("Notes"));
    resize(1280, 820);
    m_splitter->setSizes({
        kDefaultFoldersWidth,
        kDefaultListWidth,
        std::max(600, width() - kDefaultFoldersWidth - kDefaultListWidth)
    });

    updateSelectedFolder();
    selectNote(m_buttonLabIndex);
    layoutOverlayChrome();

    // Child layouts settle after the top-level window is shown.
    QTimer::singleShot(0, this, &MainWindow::layoutOverlayChrome);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    layoutOverlayChrome();
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        layoutOverlayChrome();
    }
}

QWidget *MainWindow::createFoldersPane()
{
    auto *pane = new QWidget;
    pane->setObjectName(QStringLiteral("foldersPane"));

    auto *outer = new QVBoxLayout(pane);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    m_foldersTitleBar = new QWidget(pane);
    m_foldersTitleBar->setObjectName(QStringLiteral("foldersTitleBar"));
    m_foldersTitleBar->setFixedHeight(kTitleBarHeight);

    auto *titleLayout = new QHBoxLayout(m_foldersTitleBar);
    titleLayout->setContentsMargins(platformFoldersTitleMargins());
    titleLayout->setSpacing(6);

    m_newFolderButton = makeChromeButton(QStringLiteral("+"),
                                         QStringLiteral("newFolderButton"),
                                         m_foldersTitleBar);
    m_newFolderButton->setToolTip(QStringLiteral("New folder"));

    titleLayout->addStretch(1);
    titleLayout->addWidget(m_newFolderButton);
    outer->addWidget(m_foldersTitleBar);

    auto *sectionWrapper = new QWidget(pane);
    auto *sectionLayout = new QHBoxLayout(sectionWrapper);
    sectionLayout->setContentsMargins(16, 8, 16, 0);
    sectionLayout->addWidget(makeLabel(QStringLiteral("ICLOUD"),
                                       QStringLiteral("sectionLabel"), sectionWrapper));
    outer->addWidget(sectionWrapper);

    m_foldersList = new QListWidget(pane);
    m_foldersList->setObjectName(QStringLiteral("foldersList"));
    m_foldersList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_foldersList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_foldersList->setEditTriggers(QAbstractItemView::EditKeyPressed |
                                   QAbstractItemView::SelectedClicked);
    m_foldersList->addItems({
        QStringLiteral("All iCloud"),
        QStringLiteral("Notes"),
        QStringLiteral("Pinned"),
        QStringLiteral("Work"),
        QStringLiteral("Personal"),
        QStringLiteral("Recently Deleted")
    });
    m_foldersList->setCurrentRow(0);
    outer->addWidget(m_foldersList, 1);

    return pane;
}

QWidget *MainWindow::createNotesListPane()
{
    auto *pane = new QWidget;
    pane->setObjectName(QStringLiteral("notesListPane"));

    auto *outer = new QVBoxLayout(pane);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    m_listTitleBar = new QWidget(pane);
    m_listTitleBar->setObjectName(QStringLiteral("listTitleBar"));
    m_listTitleBar->setFixedHeight(kTitleBarHeight);

    m_listTitleLayout = new QHBoxLayout(m_listTitleBar);
    m_listTitleLayout->setContentsMargins(14, 11, 12, 11);
    m_listTitleLayout->setSpacing(8);

    m_currentFolderLabel = makeLabel(QStringLiteral("All iCloud"),
                                     QStringLiteral("paneTitle"), m_listTitleBar);
    m_listMoreButton = makeChromeButton(QStringLiteral("..."),
                                        QStringLiteral("listMoreButton"), m_listTitleBar);
    m_listMoreButton->setToolTip(QStringLiteral("Folder options"));

    auto *folderMenu = new QMenu(m_listMoreButton);
    folderMenu->addAction(QStringLiteral("New Note"));
    folderMenu->addAction(QStringLiteral("Sort by Date"));
    folderMenu->addSeparator();
    folderMenu->addAction(QStringLiteral("Folder Settings"));
    m_listMoreButton->setMenu(folderMenu);

    m_listTitleLayout->addWidget(m_currentFolderLabel);
    m_listTitleLayout->addStretch(1);
    m_listTitleLayout->addWidget(m_listMoreButton);
    outer->addWidget(m_listTitleBar);

    m_notesList = new QListWidget(pane);
    m_notesList->setObjectName(QStringLiteral("notesList"));
    m_notesList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_notesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_notesList, &QListWidget::currentRowChanged,
            this, &MainWindow::selectNote);
    outer->addWidget(m_notesList, 1);

    return pane;
}

QWidget *MainWindow::createEditorPane()
{
    auto *pane = new QWidget;
    pane->setObjectName(QStringLiteral("editorPane"));

    auto *outer = new QVBoxLayout(pane);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    m_editorTitleBar = new QWidget(pane);
    m_editorTitleBar->setObjectName(QStringLiteral("editorTitleBar"));
    m_editorTitleBar->setFixedHeight(kTitleBarHeight);

    auto *titleLayout = new QHBoxLayout(m_editorTitleBar);
    titleLayout->setContentsMargins(platformEditorTitleMargins());
    titleLayout->setSpacing(6);

    auto *scopeLabel = makeLabel(QStringLiteral("All iCloud"),
                                 QStringLiteral("paneTitle"), m_editorTitleBar);
    m_galleryButton = makeToolbarButton(QStringLiteral("Gallery"), m_editorTitleBar);
    m_checklistButton = makeToolbarButton(QStringLiteral("Checklist"), m_editorTitleBar);
    m_tableButton = makeToolbarButton(QStringLiteral("Table"), m_editorTitleBar);
    m_shareButton = makeToolbarButton(QStringLiteral("Share"), m_editorTitleBar);
    m_moreButton = makeToolbarButton(QStringLiteral("More"), m_editorTitleBar);

    titleLayout->addWidget(scopeLabel);
    titleLayout->addStretch(1);
    titleLayout->addWidget(m_galleryButton);
    titleLayout->addWidget(m_checklistButton);
    titleLayout->addWidget(m_tableButton);
    titleLayout->addWidget(m_shareButton);
    titleLayout->addWidget(m_moreButton);
    outer->addWidget(m_editorTitleBar);

    m_editorStack = new QStackedWidget(pane);

    m_notePage = new QWidget(m_editorStack);
    m_notePage->setObjectName(QStringLiteral("notePage"));
    auto *noteLayout = new QVBoxLayout(m_notePage);
    noteLayout->setContentsMargins(34, 25, 38, 28);
    noteLayout->setSpacing(10);

    m_editorTitleLabel = makeLabel(QString(), QStringLiteral("editorTitle"), m_notePage);
    m_editorMetaLabel = makeLabel(QString(), QStringLiteral("editorMeta"), m_notePage);
    m_editor = new QTextEdit(m_notePage);
    m_editor->setObjectName(QStringLiteral("editor"));
    m_editor->setFrameStyle(QFrame::NoFrame);

    noteLayout->addWidget(m_editorTitleLabel);
    noteLayout->addWidget(m_editorMetaLabel);
    noteLayout->addSpacing(8);
    noteLayout->addWidget(m_editor, 1);

    m_buttonLabPage = createButtonLabPage();
    m_editorStack->addWidget(m_notePage);
    m_editorStack->addWidget(m_buttonLabPage);
    outer->addWidget(m_editorStack, 1);

    return pane;
}

QWidget *MainWindow::createButtonLabPage()
{
    auto *page = new QWidget;
    page->setObjectName(QStringLiteral("buttonLabPage"));

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 25, 38, 28);
    layout->setSpacing(12);

    layout->addWidget(makeLabel(QStringLiteral("Window Button Lab"),
                                QStringLiteral("labTitle"), page));
    layout->addWidget(makeLabel(QStringLiteral("Today  -  Local window chrome"),
                                QStringLiteral("labMeta"), page));
    layout->addSpacing(12);
    layout->addWidget(makeLabel(QStringLiteral("VISIBILITY"),
                                QStringLiteral("labSectionLabel"), page));

    auto *segment = new QWidget(page);
    segment->setObjectName(QStringLiteral("visibilitySegment"));
    segment->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    auto *segmentLayout = new QHBoxLayout(segment);
    segmentLayout->setContentsMargins(2, 2, 2, 2);
    segmentLayout->setSpacing(2);

    m_visibilityButtonGroup = new QButtonGroup(page);
    m_visibilityButtonGroup->setExclusive(true);

    const struct {
        const char *label;
        QWK::WindowAgentBase::SystemButtonVisibility visibility;
    } modes[] = {
        {"Always", QWK::WindowAgentBase::AlwaysVisible},
        {"On Hover", QWK::WindowAgentBase::VisibleOnHover},
        {"Hidden", QWK::WindowAgentBase::AlwaysHidden},
    };

    for (const auto &mode : modes) {
        auto *button = new QPushButton(QString::fromLatin1(mode.label), segment);
        button->setObjectName(QStringLiteral("visibilitySegmentButton"));
        button->setCheckable(true);
        button->setFocusPolicy(Qt::NoFocus);
        m_visibilityButtonGroup->addButton(button, static_cast<int>(mode.visibility));
        segmentLayout->addWidget(button);
        if (mode.visibility == QWK::WindowAgentBase::AlwaysVisible) {
            button->setChecked(true);
        }
    }

    connect(m_visibilityButtonGroup, &QButtonGroup::idClicked,
            this, &MainWindow::setSystemButtonVisibility);

    layout->addWidget(segment, 0, Qt::AlignLeft);
    m_visibilityStatusLabel = makeLabel(QStringLiteral("Current: always visible"),
                                        QStringLiteral("visibilityStatus"), page);
    layout->addWidget(m_visibilityStatusLabel);

    addPlatformButtonLabControls(layout, page);

    layout->addStretch(1);
    return page;
}

void MainWindow::installWindowAgent()
{
    m_windowAgent = new QWK::WidgetWindowAgent(this);
    m_windowAgent->setup(this);
    m_windowAgent->installSystemButtons();
}

void MainWindow::registerTitleBarsAndHitTestWidgets()
{
    m_windowAgent->addTitleBar(m_foldersTitleBar);
    m_windowAgent->addTitleBar(m_listTitleBar);
    m_windowAgent->addTitleBar(m_editorTitleBar);

    m_windowAgent->setHitTestVisible(m_foldersTitleBar, m_newFolderButton, true);
    m_windowAgent->setHitTestVisible(m_foldersTitleBar, m_sidebarToggleButton, true);
    m_windowAgent->setHitTestVisible(m_listTitleBar, m_sidebarToggleButton, true);
    m_windowAgent->setHitTestVisible(m_listTitleBar, m_listMoreButton, true);

    m_windowAgent->setHitTestVisible(m_editorTitleBar, m_galleryButton, true);
    m_windowAgent->setHitTestVisible(m_editorTitleBar, m_checklistButton, true);
    m_windowAgent->setHitTestVisible(m_editorTitleBar, m_tableButton, true);
    m_windowAgent->setHitTestVisible(m_editorTitleBar, m_shareButton, true);
    m_windowAgent->setHitTestVisible(m_editorTitleBar, m_moreButton, true);

    // Splitter handles are siblings of the pane title bars. Registering them makes Qt's native
    // splitter drag win over the surrounding draggable title-bar regions.
    if (auto *firstHandle = m_splitter->handle(1)) {
        m_windowAgent->setHitTestVisible(m_foldersTitleBar, firstHandle, true);
        m_windowAgent->setHitTestVisible(m_listTitleBar, firstHandle, true);
    }
    if (auto *secondHandle = m_splitter->handle(2)) {
        m_windowAgent->setHitTestVisible(m_listTitleBar, secondHandle, true);
        m_windowAgent->setHitTestVisible(m_editorTitleBar, secondHandle, true);
    }

}

void MainWindow::applyInlineStyleSheet()
{
    setStyleSheet(notesDemoStyleSheet());
}

void MainWindow::layoutOverlayChrome()
{
    if (!m_sidebarToggleButton || !m_foldersTitleBar) {
        return;
    }

    updatePlatformChrome();

    const QPoint splitterOrigin = m_splitter->mapTo(this, QPoint());
    const int titleBarY = m_listTitleBar->mapTo(this, QPoint()).y();
    const int toggleY = titleBarY + (kTitleBarHeight - kToolbarButtonHeight) / 2;

    const int toggleX = platformSidebarToggleX(splitterOrigin.x(), foldersPaneWidth());

    m_sidebarToggleButton->move(toggleX, toggleY);
    m_sidebarToggleButton->raise();

    const int minimumFolderChromeWidth = platformMinimumFolderChromeWidth();
    m_newFolderButton->setVisible(foldersPaneWidth() >= minimumFolderChromeWidth);

    updateListTitleBarInsets();
}

void MainWindow::updateListTitleBarInsets()
{
    if (!m_listTitleLayout || !m_listTitleBar || !m_sidebarToggleButton) {
        return;
    }

    constexpr int defaultLeftMargin = 14;
    const int listLeft = m_listTitleBar->mapTo(this, QPoint()).x();
    const QRect toggleRect = m_sidebarToggleButton->geometry();

    int leftMargin = defaultLeftMargin;
    if (toggleRect.right() >= listLeft &&
        toggleRect.left() < listLeft + m_listTitleBar->width()) {
        leftMargin = std::max(defaultLeftMargin, toggleRect.right() - listLeft + 10);
    }
    m_listTitleLayout->setContentsMargins(leftMargin, 11, 12, 11);
}

void MainWindow::populateNotes()
{
    m_noteTitles = {
        QStringLiteral("Window Button Lab"),
        QStringLiteral("QWindowKit multi-titlebar"),
        QStringLiteral("Design review"),
        QStringLiteral("Weekend trip"),
        QStringLiteral("Groceries"),
        QStringLiteral("Reading queue"),
        QStringLiteral("UI notes")
    };

    m_noteSubtitles = {
        QStringLiteral("Traffic lights and caption visibility"),
        QStringLiteral("Three independent draggable regions"),
        QStringLiteral("Window chrome follow-up"),
        QStringLiteral("Packing list"),
        QStringLiteral("Kitchen staples"),
        QStringLiteral("Platform window APIs"),
        QStringLiteral("A quieter Notes-style surface")
    };

    m_noteDates = {
        QStringLiteral("Today"),
        QStringLiteral("Today"),
        QStringLiteral("Yesterday"),
        QStringLiteral("Monday"),
        QStringLiteral("Sunday"),
        QStringLiteral("Friday"),
        QStringLiteral("Thursday")
    };

    m_noteBodies = {
        QString(),
        QStringLiteral(
            "The window uses three title-bar widgets, one per pane.\n\n"
            "Interactive controls and both splitter handles are explicit hit-test exclusions. "
            "Window movement remains delegated to the platform through QWindow::startSystemMove()."),
        QStringLiteral(
            "Review native traffic-light alignment on macOS.\n\n"
            "Verify that Windows caption buttons preserve minimize, maximize, close, and snap behavior."),
        QStringLiteral(
            "Light jacket\nCamera\nNotebook\nPhone charger\nTrain tickets"),
        QStringLiteral(
            "Eggs\nMilk\nCoffee\nBananas\nGreek yogurt\nRice"),
        QStringLiteral(
            "Qt QWindow::startSystemMove\nQt QSplitterHandle\n"
            "AppKit NSWindow standardWindowButton\nMicrosoft DwmDefWindowProc"),
        QStringLiteral(
            "Use neutral pane colors instead of borders.\n\n"
            "Keep title-bar controls compact, quiet, and aligned on a single visual row.")
    };

    for (int i = 0; i < m_noteTitles.size(); ++i) {
        const QString text = QStringLiteral("%1\n%2\n%3")
                                 .arg(m_noteTitles.at(i),
                                      m_noteDates.at(i),
                                      m_noteSubtitles.at(i));
        auto *item = new QListWidgetItem(text);
        item->setSizeHint(QSize(260, 78));
        m_notesList->addItem(item);
    }

    m_notesList->setCurrentRow(m_buttonLabIndex);
}

void MainWindow::selectNote(int index)
{
    if (index < 0 || index >= m_noteTitles.size()) {
        return;
    }

    if (index == m_buttonLabIndex) {
        m_editorStack->setCurrentWidget(m_buttonLabPage);
        return;
    }

    m_editorStack->setCurrentWidget(m_notePage);
    m_editorTitleLabel->setText(m_noteTitles.at(index));
    m_editorMetaLabel->setText(m_noteDates.at(index) + QStringLiteral("  -  iCloud"));
    m_editor->setPlainText(m_noteBodies.at(index));
}

void MainWindow::updateSelectedFolder()
{
    if (auto *item = m_foldersList->currentItem()) {
        m_currentFolderLabel->setText(item->text());
    }
}

void MainWindow::setSystemButtonVisibility(int visibility)
{
    const auto mode = static_cast<QWK::WindowAgentBase::SystemButtonVisibility>(visibility);
    m_windowAgent->setSystemButtonVisibility(mode);

    QString label;
    switch (mode) {
        case QWK::WindowAgentBase::AlwaysVisible:
            label = QStringLiteral("Current: always visible");
            break;
        case QWK::WindowAgentBase::VisibleOnHover:
            label = QStringLiteral("Current: visible on hover");
            break;
        case QWK::WindowAgentBase::AlwaysHidden:
            label = QStringLiteral("Current: hidden");
            break;
    }
    m_visibilityStatusLabel->setText(label);
}

void MainWindow::toggleFoldersPane()
{
    const bool collapsed = foldersPaneWidth() <= 4;
    animateFoldersPaneTo(collapsed ? expandedFoldersPaneWidth()
                                   : kCollapsedFoldersWidth);
}

void MainWindow::animateFoldersPaneTo(int targetWidth)
{
    const int start = foldersPaneWidth();
    const int maxLeft = std::max(0, availableSplitterWidth() - 520);
    const int end = std::clamp(targetWidth, 0, maxLeft);
    if (start == end) {
        return;
    }

    m_foldersAnimation->stop();
    m_foldersAnimation->setStartValue(start);
    m_foldersAnimation->setEndValue(end);
    m_foldersAnimation->start();
}

void MainWindow::setFoldersPaneWidth(int width)
{
    if (!m_splitter) {
        return;
    }

    const QList<int> oldSizes = m_splitter->sizes();
    int middle = oldSizes.size() > 1 ? oldSizes.at(1) : kDefaultListWidth;

    const int total = availableSplitterWidth();
    const int left = std::clamp(width, 0, std::max(0, total - 520));
    const int remaining = std::max(0, total - left);

    middle = std::max(250, middle);
    if (middle > remaining - 320) {
        middle = std::max(250, remaining / 3);
    }
    const int right = std::max(320, remaining - middle);

    m_splitter->setSizes({left, middle, right});
    if (left >= kMinimumRememberedFoldersWidth) {
        m_lastExpandedFoldersWidth = left;
    }
    layoutOverlayChrome();
}

int MainWindow::foldersPaneWidth() const
{
    if (!m_splitter) {
        return 0;
    }
    const QList<int> sizes = m_splitter->sizes();
    return sizes.isEmpty() ? 0 : sizes.first();
}

int MainWindow::expandedFoldersPaneWidth() const
{
    return std::max(kMinimumRememberedFoldersWidth, m_lastExpandedFoldersWidth);
}

int MainWindow::availableSplitterWidth() const
{
    return m_splitter ? std::max(0, m_splitter->width()) : 0;
}
