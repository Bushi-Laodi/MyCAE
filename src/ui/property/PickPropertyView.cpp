#include "ui/property/PickPropertyView.h"

#include <QFormLayout>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}
}

void PickPropertyView::populate(
    QWidget *parent,
    PickMode mode,
    const QString &geometryName,
    const std::vector<int> &faceIndices
)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;

    QStringList faceIndexTexts;
    for (const int faceIndex : faceIndices) {
        faceIndexTexts.append(QString::number(faceIndex));
    }

    form->addRow(zh(u8"拾取模式:"), new QLabel(pickModeName(mode), parent));
    form->addRow(zh(u8"几何:"), new QLabel(geometryName, parent));
    form->addRow(zh(u8"已选面:"), new QLabel(QString::number(faceIndices.size()), parent));
    form->addRow(zh(u8"面索引:"), new QLabel(faceIndexTexts.join(", "), parent));
    dynamicLayout->addLayout(form);
}
