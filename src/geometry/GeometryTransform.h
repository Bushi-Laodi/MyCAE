#pragma once

struct GeometryTransformParameters
{
    bool setAbsoluteCenter = false;
    double targetCenterX = 0.0;
    double targetCenterY = 0.0;
    double targetCenterZ = 0.0;
    double translateX = 0.0;
    double translateY = 0.0;
    double translateZ = 0.0;
    double rotateXDegrees = 0.0;
    double rotateYDegrees = 0.0;
    double rotateZDegrees = 0.0;
    double uniformScale = 1.0;
};

struct GeometryCenter
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};
