#include "solver/calculix/CalculiXInputDeck.h"

#include <QFile>
#include <QIODevice>
#include <QTextStream>

void CalculiXInputDeck::appendLine(const QString &line)
{
    m_lines.append(line);
}

void CalculiXInputDeck::appendComment(const QString &comment)
{
    m_lines.append("** " + comment);
}

QString CalculiXInputDeck::toText() const
{
    return m_lines.join('\n') + '\n';
}

bool CalculiXInputDeck::writeToFile(const QString &filePath, QString *errorMessage) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write CalculiX input file: " + file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream << toText();
    return true;
}
