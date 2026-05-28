#include "solver/calculix/CalculiXDeckFormatting.h"

#include <QStringList>

#include <cmath>

QString calculixSafeName(QString value, const QString &fallback)
{
    value = value.trimmed();
    bool hasUsefulCharacter = false;
    for (QChar &ch : value) {
        if (ch.isLetterOrNumber()) {
            hasUsefulCharacter = true;
        } else if (ch.isSpace() || ch == '_' || ch == '-') {
            ch = '_';
        } else {
            ch = '_';
        }
    }
    return hasUsefulCharacter ? value : fallback;
}

QString calculixNumber(double value)
{
    if (std::abs(value) < 1.0e-12) {
        value = 0.0;
    }
    return QString::number(value, 'g', 12);
}

void appendCalculiXIdList(CalculiXInputDeck &deck, const std::vector<int> &ids)
{
    QStringList currentLine;
    for (const int id : ids) {
        currentLine.append(QString::number(id));
        if (currentLine.size() == 16) {
            deck.appendLine(currentLine.join(", "));
            currentLine.clear();
        }
    }
    if (!currentLine.isEmpty()) {
        deck.appendLine(currentLine.join(", "));
    }
}
