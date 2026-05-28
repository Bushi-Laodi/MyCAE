#pragma once

#include "solver/BoundaryBindingInspector.h"

class QWidget;
struct FaceGroup;

class FaceGroupPropertyView
{
public:
    static void populate(QWidget *parent, const FaceGroup &faceGroup, const FaceGroupBindingSummary &bindingSummary);
};
