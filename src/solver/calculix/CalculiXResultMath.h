#pragma once

#include "solver/calculix/CalculiXDatResultReader.h"

#include <cmath>

namespace CalculiXResultMath
{
inline double displacementMagnitude(const CalculiXNodeDisplacement &value)
{
    return std::sqrt(value.ux * value.ux + value.uy * value.uy + value.uz * value.uz);
}

inline double vonMisesStress(const CalculiXElementStress &stress)
{
    const double normal =
        (stress.sxx - stress.syy) * (stress.sxx - stress.syy)
        + (stress.syy - stress.szz) * (stress.syy - stress.szz)
        + (stress.szz - stress.sxx) * (stress.szz - stress.sxx);
    const double shear = 6.0 * (
        stress.sxy * stress.sxy
        + stress.sxz * stress.sxz
        + stress.syz * stress.syz
    );
    return std::sqrt(0.5 * (normal + shear));
}
}
