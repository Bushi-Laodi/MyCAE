#include "solver/calculix/CalculiXDeckFormatting.h"

#include <QStringList>

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
