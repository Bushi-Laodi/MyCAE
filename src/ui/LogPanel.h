#pragma once

#include <QWidget>

class QPlainTextEdit;
class QString;

class LogPanel final : public QWidget
{
public:
    explicit LogPanel(QWidget *parent = nullptr);

    void appendMessage(const QString &message);

private:
    QPlainTextEdit *m_logView = nullptr;
};
