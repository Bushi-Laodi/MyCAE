#pragma once

#include "solver/calculix/CalculiXInputDeck.h"

#include <QString>

#include <vector>

QString calculixSafeName(QString value, const QString &fallback);
void appendCalculiXIdList(CalculiXInputDeck &deck, const std::vector<int> &ids);
