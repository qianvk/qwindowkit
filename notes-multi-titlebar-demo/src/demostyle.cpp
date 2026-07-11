#include "demostyle.h"

QString notesDemoStyleSheet()
{
    return QStringLiteral(R"(
QMainWindow,
QWidget#rootSurface {
    background: #ffffff;
}

QSplitter#notesSplitter,
QSplitter#notesSplitter::handle,
QSplitter#notesSplitter::handle:hover {
    background: transparent;
    border: none;
}

QSplitter#notesSplitter::handle {
    width: 1px;
}

QWidget#foldersPane,
QWidget#foldersTitleBar {
    background: #e9e9eb;
    border: none;
}

QWidget#notesListPane,
QWidget#listTitleBar {
    background: #f5f5f6;
    border: none;
}

QWidget#editorPane,
QWidget#editorTitleBar,
QWidget#notePage,
QWidget#buttonLabPage {
    background: #ffffff;
    border: none;
}

QLabel#paneTitle {
    color: #292a2e;
    font-size: 14px;
    font-weight: 600;
}

QLabel#sectionLabel,
QLabel#labSectionLabel {
    color: #74757a;
    font-size: 11px;
    font-weight: 600;
    letter-spacing: 0px;
}

QPushButton#toolbarButton,
QPushButton#newFolderButton,
QPushButton#listMoreButton {
    color: #34353a;
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 5px 8px;
    font-size: 12px;
    font-weight: 500;
}

QPushButton#newFolderButton {
    padding: 0px;
    font-size: 21px;
    font-weight: 300;
}

QPushButton#listMoreButton {
    padding: 0px 0px 5px 0px;
    font-size: 17px;
    font-weight: 600;
}

QPushButton#toolbarButton:hover,
QPushButton#newFolderButton:hover,
QPushButton#listMoreButton:hover,
SidebarToggleButton#sidebarToggleButton:hover {
    background: rgba(0, 0, 0, 0.055);
}

QPushButton#toolbarButton:pressed,
QPushButton#newFolderButton:pressed,
QPushButton#listMoreButton:pressed,
SidebarToggleButton#sidebarToggleButton:pressed {
    background: rgba(0, 0, 0, 0.10);
}

QPushButton#listMoreButton::menu-indicator {
    image: none;
    width: 0px;
}

QListWidget#foldersList,
QListWidget#notesList {
    background: transparent;
    border: none;
    outline: none;
    padding: 8px;
}

QListWidget#foldersList::item {
    color: #35363a;
    background: transparent;
    border: none;
    border-radius: 7px;
    padding: 8px 10px;
    margin: 1px 0px;
}

QListWidget#foldersList::item:selected {
    color: #1f2024;
    background: rgba(0, 0, 0, 0.075);
}

QListWidget#notesList::item {
    color: #3b3c41;
    background: transparent;
    border: none;
    border-radius: 8px;
    padding: 12px;
    margin: 2px 0px;
}

QListWidget#notesList::item:selected {
    color: #1d1e22;
    background: #ffe18a;
}

QLabel#editorTitle,
QLabel#labTitle {
    color: #17181c;
    font-size: 29px;
    font-weight: 700;
}

QLabel#editorMeta,
QLabel#labMeta,
QLabel#visibilityStatus,
QLabel#sliderValue {
    color: #7a7b80;
    font-size: 12px;
}

QTextEdit#editor {
    color: #24252a;
    background: transparent;
    border: none;
    font-size: 15px;
    padding: 0px;
}

QWidget#visibilitySegment {
    background: #ededf0;
    border: 1px solid rgba(0, 0, 0, 0.07);
    border-radius: 7px;
}

QPushButton#visibilitySegmentButton {
    min-width: 78px;
    color: #4c4d52;
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 7px 12px;
    font-size: 12px;
    font-weight: 500;
}

QPushButton#visibilitySegmentButton:hover {
    background: rgba(255, 255, 255, 0.55);
}

QPushButton#visibilitySegmentButton:checked {
    color: #202126;
    background: #ffffff;
}
)");
}
