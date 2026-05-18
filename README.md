# MyCAE

MyCAE is a lightweight CAE integration practice project built with Qt 6.

## Stage 1 Goal

The first stage only builds a runnable desktop shell:

- main window
- menu bar and tool bar
- project/model tree
- central placeholder render area
- property panel
- log output panel

Open CASCADE, VTK, Gmsh, and CalculiX are intentionally left for later stages.

## Build

```powershell
cmake -S . -B build-qt6 -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH=D:/dev/Qt/6.6.3/msvc2019_64
cmake --build build-qt6 --config Debug
```

The installed Qt package is `msvc2019_64`, so use a 64-bit MSVC compiler from Visual Studio 2019 16.7 or newer. Visual Studio 2017 is too old for Qt 6.6.3.

## Current Development Order

1. Qt 6 application skeleton
2. project data structure and project creation
3. minimal geometry model
4. render view integration
5. mesh and solver adapters
6. post-processing view
