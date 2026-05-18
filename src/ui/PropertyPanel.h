#pragma once

#include <QWidget>

class QLabel;
struct BoxGeometry;

class PropertyPanel final : public QWidget
{
public:
    explicit PropertyPanel(QWidget *parent = nullptr);

    void showEmptySelection();
    void showBoxGeometry(const BoxGeometry &box);

private:
    QLabel *m_selectionValue = nullptr;
    QLabel *m_typeValue = nullptr;
    QLabel *m_nameValue = nullptr;
    QLabel *m_lengthValue = nullptr;
    QLabel *m_widthValue = nullptr;
    QLabel *m_heightValue = nullptr;
};
