#pragma once

#include <QString>

#include <vector>

enum class MaterialDomain
{
    Fluid,
    Solid
};

enum class ViscosityModel
{
    Newtonian
};

struct MaterialProperty
{
    QString name;
    double value = 0.0;
    QString unit;
};

struct Material
{
    QString id;
    QString name;
    MaterialDomain domain = MaterialDomain::Fluid;
    ViscosityModel viscosityModel = ViscosityModel::Newtonian;

    bool hasDensity = false;
    double density = 0.0;
    QString densityUnit = "kg/m^3";

    bool hasDynamicViscosity = false;
    double dynamicViscosity = 0.0;
    QString dynamicViscosityUnit = "Pa*s";

    bool hasKinematicViscosity = false;
    double kinematicViscosity = 0.0;
    QString kinematicViscosityUnit = "m^2/s";

    std::vector<MaterialProperty> extraProperties;
};

inline QString toString(MaterialDomain domain)
{
    switch (domain) {
    case MaterialDomain::Fluid:
        return "fluid";
    case MaterialDomain::Solid:
        return "solid";
    }
    return "fluid";
}

inline QString toString(ViscosityModel model)
{
    switch (model) {
    case ViscosityModel::Newtonian:
        return "Newtonian";
    }
    return "Newtonian";
}
