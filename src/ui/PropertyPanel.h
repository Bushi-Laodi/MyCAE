#pragma once

#include <QWidget>

class QLabel;
struct BoxGeometry;
struct MeshObject;

class PropertyPanel final : public QWidget
{
public:
    explicit PropertyPanel(QWidget *parent = nullptr);

    void showEmptySelection();
    void showBoxGeometry(const BoxGeometry &box);
    void showMeshObject(const MeshObject &meshObject);

private:
    QLabel *m_selectionValue = nullptr;
    QLabel *m_typeValue = nullptr;
    QLabel *m_nameValue = nullptr;
    QLabel *m_lengthValue = nullptr;
    QLabel *m_widthValue = nullptr;
    QLabel *m_heightValue = nullptr;
    QLabel *m_sourceGeometryValue = nullptr;
    QLabel *m_meshFileValue = nullptr;
    QLabel *m_nodeCountValue = nullptr;
    QLabel *m_tetraCountValue = nullptr;
    QLabel *m_createdAtValue = nullptr;
};
