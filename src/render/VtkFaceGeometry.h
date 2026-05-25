#pragma once

#include "picking/PickSelection.h"

class vtkCell;

namespace VtkFaceGeometry
{
void fillPickSelectionFromCell(vtkCell *cell, PickSelection &selection);
}
