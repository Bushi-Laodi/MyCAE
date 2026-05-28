#pragma once

#include "solver/calculix/CalculiXInputDeck.h"

#include <QString>

#include <vector>

QString calculixSafeName(QString value, const QString &fallback);
QString calculixNumber(double value);
void appendCalculiXIdList(CalculiXInputDeck &deck, const std::vector<int> &ids);
