#include "ui/property/FaceGroupPropertyView.h"

#include "geometry/FaceGroup.h"

#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString enabledText(bool enabled)
{
    return enabled ? zh(u8"启用") : zh(u8"禁用");
}

QLabel *emptyFaceGroupHint(QWidget *parent)
{
    auto *label = new QLabel(zh(u8"该面组还没有包含任何面。请先进入拾取面模式，选择目标面后再创建或追加到面组。"), parent);
    label->setWordWrap(true);
    label->setStyleSheet(
        "QLabel {"
        "  color: #92400e;"
        "  background: #fffbeb;"
        "  border: 1px solid #fcd34d;"
        "  border-radius: 4px;"
        "  padding: 6px 8px;"
        "}"
    );
    return label;
}

QString listText(const QStringList &values)
{
    return values.isEmpty() ? "-" : values.join("\n");
}

QLabel *hintLabel(QWidget *parent, const QString &text, const QString &color, const QString &background, const QString &border)
{
    auto *label = new QLabel(text, parent);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setStyleSheet(
        QString(
            "QLabel {"
            "  color: %1;"
            "  background: %2;"
            "  border: 1px solid %3;"
            "  border-radius: 4px;"
            "  padding: 6px 8px;"
            "}"
        ).arg(color, background, border)
    );
    return label;
}
}

void FaceGroupPropertyView::populate(
    QWidget *parent,
    const FaceGroup &faceGroup,
    const FaceGroupBindingSummary &bindingSummary
)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(faceGroup.id, parent));
    form->addRow(zh(u8"名称:"), new QLabel(faceGroup.name, parent));
    form->addRow(zh(u8"几何:"), new QLabel(faceGroup.geometryName, parent));
    form->addRow(zh(u8"角色:"), new QLabel(faceGroup.role, parent));
    form->addRow(zh(u8"面数量:"), new QLabel(QString::number(faceGroup.faceIndices.size()), parent));
    form->addRow(zh(u8"物理组:"), new QLabel(enabledText(faceGroup.physicalGroupEnabled), parent));
    form->addRow(zh(u8"局部网格:"), new QLabel(enabledText(faceGroup.localMeshEnabled), parent));
    form->addRow(zh(u8"局部网格尺寸:"), new QLabel(QString::number(faceGroup.localMeshSize), parent));
    form->addRow(zh(u8"复查状态:"), new QLabel(faceGroup.needsReview ? zh(u8"需要复查") : zh(u8"正常"), parent));
    if (faceGroup.needsReview && !faceGroup.reviewReason.isEmpty()) {
        form->addRow(zh(u8"复查原因:"), new QLabel(faceGroup.reviewReason, parent));
    }

    QStringList faceIndexTexts;
    for (const int faceIndex : faceGroup.faceIndices) {
        faceIndexTexts.append(QString::number(faceIndex));
    }
    form->addRow(zh(u8"面索引:"), new QLabel(faceIndexTexts.isEmpty() ? "-" : faceIndexTexts.join(", "), parent));

    if (!faceGroup.faceReferences.empty()) {
        const FaceReference &firstReference = faceGroup.faceReferences.front();
        form->addRow(zh(u8"首个面面积:"), new QLabel(QString::number(firstReference.area), parent));
        form->addRow(
            zh(u8"首个面法向:"),
            new QLabel(
                QString("(%1, %2, %3)")
                    .arg(firstReference.normal.x)
                    .arg(firstReference.normal.y)
                    .arg(firstReference.normal.z),
                parent
            )
        );
    }
    if (faceGroup.faceIndices.empty() && faceGroup.faceReferences.empty()) {
        dynamicLayout->addWidget(emptyFaceGroupHint(parent));
    }
    dynamicLayout->addLayout(form);

    auto *bindingForm = new QFormLayout;
    bindingForm->addRow(zh(u8"绑定状态:"), new QLabel(bindingSummary.status, parent));
    bindingForm->addRow(zh(u8"边界条件:"), new QLabel(listText(bindingSummary.boundaryConditions), parent));
    bindingForm->addRow(zh(u8"载荷:"), new QLabel(listText(bindingSummary.loads), parent));
    dynamicLayout->addLayout(bindingForm);

    for (const QString &warning : bindingSummary.warnings) {
        dynamicLayout->addWidget(hintLabel(parent, warning, "#92400e", "#fffbeb", "#fcd34d"));
    }
}
