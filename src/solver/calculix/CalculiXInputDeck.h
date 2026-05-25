#pragma once

#include <QString>
#include <QStringList>

class CalculiXInputDeck
{
public:
    void appendLine(const QString &line = {});
    void appendComment(const QString &comment);
    QString toText() const;
    bool writeToFile(const QString &filePath, QString *errorMessage = nullptr) const;

private:
    QStringList m_lines;
};
