#include "PropertyPanel.h"

#include "geometry/BoxGeometry.h"
#include "mesh/MeshObject.h"

#include <QFormLayout>
#include <QLabel>
#include <QString>

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *form = new QFormLayout(this);
    m_selectionValue = new QLabel("无", this);
    m_typeValue = new QLabel("-", this);
    m_nameValue = new QLabel("-", this);
    m_lengthValue = new QLabel("-", this);
    m_widthValue = new QLabel("-", this);
    m_heightValue = new QLabel("-", this);
    m_sourceGeometryValue = new QLabel("-", this);
    m_meshFileValue = new QLabel("-", this);
    m_nodeCountValue = new QLabel("-", this);
    m_tetraCountValue = new QLabel("-", this);
    m_createdAtValue = new QLabel("-", this);

    form->addRow("当前选择", m_selectionValue);
    form->addRow("类型", m_typeValue);
    form->addRow("名称", m_nameValue);
    form->addRow("长度", m_lengthValue);
    form->addRow("宽度", m_widthValue);
    form->addRow("高度", m_heightValue);
    form->addRow("来源几何", m_sourceGeometryValue);
    form->addRow("网格文件", m_meshFileValue);
    form->addRow("节点数量", m_nodeCountValue);
    form->addRow("四面体数量", m_tetraCountValue);
    form->addRow("创建时间", m_createdAtValue);
}

void PropertyPanel::showEmptySelection()
{
    m_selectionValue->setText("无");
    m_typeValue->setText("-");
    m_nameValue->setText("-");
    m_lengthValue->setText("-");
    m_widthValue->setText("-");
    m_heightValue->setText("-");
    m_sourceGeometryValue->setText("-");
    m_meshFileValue->setText("-");
    m_nodeCountValue->setText("-");
    m_tetraCountValue->setText("-");
    m_createdAtValue->setText("-");
}

void PropertyPanel::showBoxGeometry(const BoxGeometry &box)
{
    const QString suffix = " " + box.unit;
    m_selectionValue->setText(box.name);
    m_typeValue->setText("长方体");
    m_nameValue->setText(box.name);
    m_lengthValue->setText(QString::number(box.length) + suffix);
    m_widthValue->setText(QString::number(box.width) + suffix);
    m_heightValue->setText(QString::number(box.height) + suffix);
    m_sourceGeometryValue->setText("-");
    m_meshFileValue->setText("-");
    m_nodeCountValue->setText("-");
    m_tetraCountValue->setText("-");
    m_createdAtValue->setText("-");
}

void PropertyPanel::showMeshObject(const MeshObject &meshObject)
{
    m_selectionValue->setText(meshObject.name);
    m_typeValue->setText(meshObject.type);
    m_nameValue->setText(meshObject.name);
    m_lengthValue->setText("-");
    m_widthValue->setText("-");
    m_heightValue->setText("-");
    m_sourceGeometryValue->setText(meshObject.sourceGeometryName);
    m_meshFileValue->setText(meshObject.mshFile);
    m_nodeCountValue->setText(QString::number(meshObject.nodeCount));
    m_tetraCountValue->setText(QString::number(meshObject.tetraCount));
    m_createdAtValue->setText(meshObject.createdAt);
}
