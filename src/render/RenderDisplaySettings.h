#pragma once

struct RenderDisplaySettings
{
    double primaryOpacity = 1.0;
    double resultOpacity = 0.96;
    double undeformedOverlayOpacity = 0.18;
    double highlightOpacity = 0.90;

    bool geometryEdgesVisible = false;
    bool meshEdgesVisible = true;
    bool orientationMarkerVisible = true;
};
