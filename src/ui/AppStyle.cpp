#include "ui/AppStyle.h"

#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

namespace
{
QPalette whitePalette()
{
    QPalette palette;
    palette.setColor(QPalette::Window, "#f7f8fa");
    palette.setColor(QPalette::WindowText, "#111827");
    palette.setColor(QPalette::Base, "#ffffff");
    palette.setColor(QPalette::AlternateBase, "#f8fafc");
    palette.setColor(QPalette::ToolTipBase, "#ffffff");
    palette.setColor(QPalette::ToolTipText, "#111827");
    palette.setColor(QPalette::Text, "#111827");
    palette.setColor(QPalette::Button, "#ffffff");
    palette.setColor(QPalette::ButtonText, "#111827");
    palette.setColor(QPalette::BrightText, "#ffffff");
    palette.setColor(QPalette::Highlight, "#dbeafe");
    palette.setColor(QPalette::HighlightedText, "#111827");
    palette.setColor(QPalette::PlaceholderText, "#9ca3af");
    return palette;
}

QString whiteStyleSheet()
{
    return QStringLiteral(R"(
QMainWindow,
QDialog {
    background: #f7f8fa;
    color: #111827;
    font-size: 13px;
}

QMenuBar {
    background: #ffffff;
    border-bottom: 1px solid #d8dde6;
    padding: 0 4px;
}

QMenuBar::item {
    background: transparent;
    padding: 5px 10px;
}

QMenuBar::item:selected {
    background: #edf3ff;
    color: #0f172a;
}

QMenu {
    background: #ffffff;
    border: 1px solid #cfd6e3;
    padding: 5px;
}

QMenu::item {
    border-radius: 4px;
    padding: 6px 28px 6px 24px;
}

QMenu::item:selected {
    background: #eaf2ff;
    color: #0f172a;
}

QMenu::separator {
    background: #e5e7eb;
    height: 1px;
    margin: 5px 8px;
}

QToolBar {
    background: #f6f7f9;
    border: 0;
    border-bottom: 1px solid #d8dde6;
    padding: 3px 6px;
    spacing: 4px;
}

QToolButton {
    border: 1px solid transparent;
    border-radius: 4px;
    color: #111827;
    padding: 4px;
}

QToolButton:hover {
    background: #e8eef7;
    border-color: #c8d5e8;
}

QToolButton:checked {
    background: #dbeafe;
    border-color: #93c5fd;
}

QToolButton:disabled {
    background: transparent;
    color: #9ca3af;
}

QStatusBar {
    background: #ffffff;
    border-top: 1px solid #d8dde6;
    color: #4b5563;
}

QDockWidget {
    background: #ffffff;
    border: 1px solid #d8dde6;
}

QDockWidget::title {
    background: #f3f5f8;
    border-bottom: 1px solid #d8dde6;
    color: #111827;
    font-weight: 600;
    padding: 5px 8px;
}

QTabWidget::pane {
    border: 1px solid #d8dde6;
    background: #ffffff;
}

QTabBar::tab {
    background: #f3f5f8;
    border: 1px solid #d8dde6;
    color: #374151;
    padding: 6px 10px;
}

QTabBar::tab:selected {
    background: #ffffff;
    color: #111827;
}

QTableWidget,
QTableView {
    alternate-background-color: #f8fafc;
    background: #ffffff;
    border: 1px solid #d8dde6;
    gridline-color: #e5e7eb;
    selection-background-color: #dbeafe;
    selection-color: #111827;
}

QHeaderView::section {
    background: #f3f5f8;
    border: 0;
    border-right: 1px solid #d8dde6;
    border-bottom: 1px solid #d8dde6;
    color: #374151;
    font-weight: 600;
    padding: 5px 8px;
}

QLineEdit,
QComboBox,
QSpinBox,
QDoubleSpinBox,
QPlainTextEdit,
QTextEdit {
    background: #ffffff;
    border: 1px solid #cfd6e3;
    border-radius: 4px;
    color: #111827;
    padding: 4px 6px;
    selection-background-color: #bfdbfe;
}

QLineEdit:focus,
QComboBox:focus,
QSpinBox:focus,
QDoubleSpinBox:focus,
QPlainTextEdit:focus,
QTextEdit:focus {
    border-color: #60a5fa;
}

QComboBox::drop-down {
    border-left: 1px solid #d8dde6;
    width: 22px;
}

QPushButton {
    background: #ffffff;
    border: 1px solid #cfd6e3;
    border-radius: 4px;
    color: #111827;
    padding: 5px 10px;
}

QPushButton:hover {
    background: #f3f7ff;
    border-color: #93c5fd;
}

QPushButton:pressed {
    background: #dbeafe;
}

QPushButton:disabled,
QLineEdit:disabled,
QComboBox:disabled,
QSpinBox:disabled,
QDoubleSpinBox:disabled {
    background: #f3f4f6;
    border-color: #e5e7eb;
    color: #9ca3af;
}

QCheckBox {
    color: #111827;
    spacing: 6px;
}

QGroupBox {
    border: 1px solid #d8dde6;
    border-radius: 6px;
    margin-top: 12px;
    padding: 8px;
}

QGroupBox::title {
    color: #374151;
    left: 8px;
    padding: 0 4px;
    subcontrol-origin: margin;
}

QLabel {
    color: #111827;
}

QScrollBar:vertical {
    background: #f3f4f6;
    margin: 0;
    width: 10px;
}

QScrollBar::handle:vertical {
    background: #cbd5e1;
    border-radius: 5px;
    min-height: 24px;
}

QScrollBar::handle:vertical:hover {
    background: #94a3b8;
}

QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar:horizontal {
    background: #f3f4f6;
    height: 10px;
    margin: 0;
}

QScrollBar::handle:horizontal {
    background: #cbd5e1;
    border-radius: 5px;
    min-width: 24px;
}

QScrollBar::handle:horizontal:hover {
    background: #94a3b8;
}

QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal {
    width: 0;
}
)");
}
}

void AppStyle::apply(QApplication &app)
{
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setPalette(whitePalette());
    app.setStyleSheet(whiteStyleSheet());
}
