#pragma once

#include <QMainWindow>
#include <QMargins>
#include <QStringList>

class QButtonGroup;
class QHBoxLayout;
class QLabel;
class QListWidget;
class QPushButton;
#ifdef Q_OS_MAC
class QSlider;
#endif
class QSplitter;
class QStackedWidget;
class QTextEdit;
class QVariantAnimation;
class QVBoxLayout;
class QWidget;

class SidebarToggleButton;

namespace QWK {
    class WidgetWindowAgent;
}

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    QWidget *createFoldersPane();
    QWidget *createNotesListPane();
    QWidget *createEditorPane();
    QWidget *createButtonLabPage();

    void installWindowAgent();
    void registerTitleBarsAndHitTestWidgets();

    void applyInlineStyleSheet();
    void layoutOverlayChrome();
    void updateListTitleBarInsets();

    void addPlatformButtonLabControls(QVBoxLayout *layout, QWidget *page);
    QMargins platformFoldersTitleMargins() const;
    QMargins platformEditorTitleMargins() const;
    int platformSidebarToggleX(int splitterX, int folderWidth) const;
    int platformMinimumFolderChromeWidth() const;
    void updatePlatformChrome();

    void populateNotes();
    void selectNote(int index);
    void updateSelectedFolder();
    void setSystemButtonVisibility(int visibility);

    void toggleFoldersPane();
    void animateFoldersPaneTo(int targetWidth);
    void setFoldersPaneWidth(int width);
    int foldersPaneWidth() const;
    int expandedFoldersPaneWidth() const;
    int availableSplitterWidth() const;

private:
    QWK::WidgetWindowAgent *m_windowAgent = nullptr;

    QWidget *m_root = nullptr;
    QSplitter *m_splitter = nullptr;

    QWidget *m_foldersPane = nullptr;
    QWidget *m_notesListPane = nullptr;
    QWidget *m_editorPane = nullptr;

    QWidget *m_foldersTitleBar = nullptr;
    QWidget *m_listTitleBar = nullptr;
    QWidget *m_editorTitleBar = nullptr;
    QHBoxLayout *m_listTitleLayout = nullptr;

    SidebarToggleButton *m_sidebarToggleButton = nullptr;

    QPushButton *m_newFolderButton = nullptr;
    QPushButton *m_listMoreButton = nullptr;
    QPushButton *m_galleryButton = nullptr;
    QPushButton *m_checklistButton = nullptr;
    QPushButton *m_tableButton = nullptr;
    QPushButton *m_shareButton = nullptr;
    QPushButton *m_moreButton = nullptr;

    QListWidget *m_foldersList = nullptr;
    QListWidget *m_notesList = nullptr;
    QLabel *m_currentFolderLabel = nullptr;

    QStackedWidget *m_editorStack = nullptr;
    QWidget *m_notePage = nullptr;
    QWidget *m_buttonLabPage = nullptr;
    QLabel *m_editorTitleLabel = nullptr;
    QLabel *m_editorMetaLabel = nullptr;
    QTextEdit *m_editor = nullptr;

    QButtonGroup *m_visibilityButtonGroup = nullptr;
    QLabel *m_visibilityStatusLabel = nullptr;
#ifdef Q_OS_MAC
    QSlider *m_trafficLightXSlider = nullptr;
    QSlider *m_trafficLightYSlider = nullptr;
#endif

    QVariantAnimation *m_foldersAnimation = nullptr;

    int m_lastExpandedFoldersWidth = 230;
    int m_buttonLabIndex = 0;
#ifdef Q_OS_MAC
    int m_trafficLightX = 2;
    int m_trafficLightY = 10;
#endif

    QStringList m_noteTitles;
    QStringList m_noteSubtitles;
    QStringList m_noteDates;
    QStringList m_noteBodies;
};
