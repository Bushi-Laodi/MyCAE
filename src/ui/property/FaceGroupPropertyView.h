#pragma once

class QWidget;
struct FaceGroup;

class FaceGroupPropertyView
{
public:
    static void populate(QWidget *parent, const FaceGroup &faceGroup);
};
