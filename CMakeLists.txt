cmake_minimum_required(VERSION 3.21)

project(MyCAE VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MYCAE_QT_ROOT "D:/dev/Qt/6.6.3/msvc2019_64" CACHE PATH "Root directory of the local Qt 6 installation")
if (EXISTS "${MYCAE_QT_ROOT}/lib/cmake/Qt6/Qt6Config.cmake")
    list(PREPEND CMAKE_PREFIX_PATH "${MYCAE_QT_ROOT}")
endif()

set(MYCAE_VTK_ROOT "D:/dev/VTK-9.4.2" CACHE PATH "Root directory of the local VTK installation")
set(MYCAE_VTK_RELEASE_DIR "${MYCAE_VTK_ROOT}/vtk-9.4.2-qt-release/lib/cmake/vtk-9.4")
set(MYCAE_VTK_DEBUG_DIR "${MYCAE_VTK_ROOT}/vtk-9.4.2-qt-debug/lib/cmake/vtk-9.4")
set(MYCAE_VTK_FALLBACK_DIR "${MYCAE_VTK_ROOT}/install/lib/cmake/vtk-9.4")

set(MYCAE_USE_DEBUG_VTK OFF)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MYCAE_USE_DEBUG_VTK ON)
elseif (CMAKE_CONFIGURATION_TYPES AND EXISTS "${MYCAE_VTK_DEBUG_DIR}/vtk-config.cmake")
    set(MYCAE_USE_DEBUG_VTK ON)
endif()

if (NOT DEFINED VTK_DIR OR NOT EXISTS "${VTK_DIR}/vtk-config.cmake")
    if (MYCAE_USE_DEBUG_VTK AND EXISTS "${MYCAE_VTK_DEBUG_DIR}/vtk-config.cmake")
        set(VTK_DIR "${MYCAE_VTK_DEBUG_DIR}" CACHE PATH "VTK CMake package directory" FORCE)
    elseif (EXISTS "${MYCAE_VTK_RELEASE_DIR}/vtk-config.cmake")
        set(VTK_DIR "${MYCAE_VTK_RELEASE_DIR}" CACHE PATH "VTK CMake package directory" FORCE)
    elseif (EXISTS "${MYCAE_VTK_FALLBACK_DIR}/vtk-config.cmake")
        set(VTK_DIR "${MYCAE_VTK_FALLBACK_DIR}" CACHE PATH "VTK CMake package directory" FORCE)
    endif()
endif()

set(MYCAE_OCC_ROOT "D:/dev/OCC" CACHE PATH "Root directory of the local Open CASCADE installation")
set(MYCAE_OCC_CMAKE_CANDIDATES
    "${MYCAE_OCC_ROOT}/opencascade-7.9.3-vc14-64/cmake"
    "${MYCAE_OCC_ROOT}/OCC/cmake"
    "${MYCAE_OCC_ROOT}/cmake"
    "${MYCAE_OCC_ROOT}/lib/cmake"
    "${MYCAE_OCC_ROOT}/lib/cmake/opencascade"
    "${MYCAE_OCC_ROOT}/lib/cmake/OpenCASCADE"
    "${MYCAE_OCC_ROOT}/share/opencascade"
)

foreach(MYCAE_OCC_CMAKE_DIR IN LISTS MYCAE_OCC_CMAKE_CANDIDATES)
    if (EXISTS "${MYCAE_OCC_CMAKE_DIR}/OpenCASCADEConfig.cmake"
            OR EXISTS "${MYCAE_OCC_CMAKE_DIR}/OCCTConfig.cmake")
        set(OpenCASCADE_DIR "${MYCAE_OCC_CMAKE_DIR}" CACHE PATH "Open CASCADE CMake package directory" FORCE)
        break()
    endif()
endforeach()

set(MYCAE_GMSH_EXECUTABLE
    "D:/dev/Gmsh/gmsh-4.15.2-Windows64-sdk/bin/gmsh.exe"
    CACHE FILEPATH "Path to the local Gmsh executable"
)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(VTK REQUIRED COMPONENTS
    FiltersSources
    GUISupportQt
    InteractionStyle
    RenderingCore
    RenderingOpenGL2
)
find_package(OpenCASCADE REQUIRED)

message(STATUS "Using VTK_DIR=${VTK_DIR}")
message(STATUS "Using OpenCASCADE_DIR=${OpenCASCADE_DIR}")
message(STATUS "Using OpenCASCADE_INCLUDE_DIR=${OpenCASCADE_INCLUDE_DIR}")
message(STATUS "Using OpenCASCADE_BINARY_DIR=${OpenCASCADE_BINARY_DIR}")
message(STATUS "Using MYCAE_GMSH_EXECUTABLE=${MYCAE_GMSH_EXECUTABLE}")

get_filename_component(MYCAE_OCC_INSTALL_ROOT "${OpenCASCADE_BINARY_DIR}/../../.." ABSOLUTE)
set(MYCAE_OCC_THIRDPARTY_ROOT
    "${MYCAE_OCC_INSTALL_ROOT}/../3rdparty-vc14-64"
    CACHE PATH "Root directory of Open CASCADE third-party runtime dependencies"
)
message(STATUS "Using MYCAE_OCC_THIRDPARTY_ROOT=${MYCAE_OCC_THIRDPARTY_ROOT}")

set(MYCAE_OCC_LIBRARIES
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKBRep
    TKGeomAlgo
    TKTopAlgo
    TKPrim
    TKMesh
    TKXSBase
    TKDE
    TKDESTEP
)

get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE
    NAMES windeployqt6 windeployqt
    HINTS "${QT_BIN_DIR}"
    NO_DEFAULT_PATH
)

qt_standard_project_setup()

qt_add_executable(MyCAE
    src/main.cpp
    src/geometry/BoxGeometry.h
    src/geometry/CylinderGeometry.h
    src/geometry/FaceGroup.h
    src/geometry/GeometryCreationController.cpp
    src/geometry/GeometryCreationController.h
    src/geometry/GeometryDisplayController.cpp
    src/geometry/GeometryDisplayController.h
    src/geometry/GeometryManager.cpp
    src/geometry/GeometryManager.h
    src/geometry/GeometryObject.h
    src/geometry/GeometryPropertyController.cpp
    src/geometry/GeometryPropertyController.h
    src/mesh/GmshRunner.cpp
    src/mesh/GmshRunner.h
    src/mesh/MeshData.h
    src/mesh/MeshManager.cpp
    src/mesh/MeshManager.h
    src/mesh/MeshObject.h
    src/mesh/MeshToVtkConverter.cpp
    src/mesh/MeshToVtkConverter.h
    src/mesh/MeshWorkflowController.cpp
    src/mesh/MeshWorkflowController.h
    src/mesh/MshReader.cpp
    src/mesh/MshReader.h
    src/occ/OCCBoxBuilder.cpp
    src/occ/OCCBoxBuilder.h
    src/occ/OCCCylinderBuilder.cpp
    src/occ/OCCCylinderBuilder.h
    src/occ/OCCGeometryFactory.cpp
    src/occ/OCCGeometryFactory.h
    src/occ/OCCShapeConverter.cpp
    src/occ/OCCShapeConverter.h
    src/occ/OCCShapeIO.cpp
    src/occ/OCCShapeIO.h
    src/project/Project.h
    src/project/ProjectModel.cpp
    src/project/ProjectModel.h
    src/project/ProjectModelLoader.cpp
    src/project/ProjectModelLoader.h
    src/project/ProjectManager.cpp
    src/project/ProjectManager.h
    src/solver/BoundaryCondition.h
    src/solver/Load.h
    src/solver/Material.h
    src/solver/PipeCaseExample.h
    src/solver/SimulationCase.cpp
    src/solver/SimulationCase.h
    src/solver/SimulationCaseBuilder.cpp
    src/solver/SimulationCaseBuilder.h
    src/solver/SimulationCaseJsonReader.cpp
    src/solver/SimulationCaseJsonReader.h
    src/solver/SimulationCaseJsonWriter.cpp
    src/solver/SimulationCaseJsonWriter.h
    src/solver/SimulationCaseManager.cpp
    src/solver/SimulationCaseManager.h
    src/solver/plugin/ExternalProcessSolverPlugin.cpp
    src/solver/plugin/ExternalProcessSolverPlugin.h
    src/solver/plugin/ExternalSolverPluginConfig.cpp
    src/solver/plugin/ExternalSolverPluginConfig.h
    src/solver/plugin/SolverPlugin.h
    src/solver/plugin/SolverPluginManager.cpp
    src/solver/plugin/SolverPluginManager.h
    src/ui/BoxDialog.cpp
    src/ui/BoxDialog.h
    src/ui/BoundaryConditionDialog.cpp
    src/ui/BoundaryConditionDialog.h
    src/ui/CylinderDialog.cpp
    src/ui/CylinderDialog.h
    src/ui/LoadDialog.cpp
    src/ui/LoadDialog.h
    src/ui/LogPanel.cpp
    src/ui/LogPanel.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/MaterialDialog.cpp
    src/ui/MaterialDialog.h
    src/ui/ProjectTreePanel.cpp
    src/ui/ProjectTreePanel.h
    src/ui/PropertyPanel.cpp
    src/ui/PropertyPanel.h
    src/ui/RenderView.cpp
    src/ui/RenderView.h
    src/ui/SolverDataController.cpp
    src/ui/SolverDataController.h
    src/ui/VtkRenderCanvas.cpp
    src/ui/VtkRenderCanvas.h
)

target_include_directories(MyCAE PRIVATE src ${OpenCASCADE_INCLUDE_DIR})
target_compile_definitions(MyCAE PRIVATE MYCAE_GMSH_EXECUTABLE_PATH="${MYCAE_GMSH_EXECUTABLE}")
target_link_libraries(MyCAE PRIVATE Qt6::Widgets ${VTK_LIBRARIES} ${MYCAE_OCC_LIBRARIES})

vtk_module_autoinit(
    TARGETS MyCAE
    MODULES ${VTK_LIBRARIES}
)

add_custom_target(MyCAEResources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resource/solver"
        "$<TARGET_FILE_DIR:MyCAE>/resource/solver"
    COMMAND_EXPAND_LISTS
)
add_dependencies(MyCAE MyCAEResources)

if (WIN32)
    add_custom_command(TARGET MyCAE POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:MyCAE>
            $<TARGET_FILE_DIR:MyCAE>
        COMMAND_EXPAND_LISTS
    )

    if (WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET MyCAE POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                $<$<CONFIG:Debug>:--debug>
                $<$<NOT:$<CONFIG:Debug>>:--release>
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                "$<TARGET_FILE:MyCAE>"
            COMMAND_EXPAND_LISTS
        )
    else()
        message(WARNING "windeployqt was not found. Qt platform plugins may be missing at runtime.")
    endif()

    if (OpenCASCADE_BINARY_DIR)
        file(GLOB MYCAE_OCC_RUNTIME_DLLS "${OpenCASCADE_BINARY_DIR}/*.dll")
        add_custom_command(TARGET MyCAE POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${MYCAE_OCC_RUNTIME_DLLS}
                $<TARGET_FILE_DIR:MyCAE>
            COMMAND_EXPAND_LISTS
        )
    endif()

    if (EXISTS "${MYCAE_OCC_THIRDPARTY_ROOT}")
        file(GLOB MYCAE_OCC_THIRDPARTY_RUNTIME_DLLS
            "${MYCAE_OCC_THIRDPARTY_ROOT}/ffmpeg-3.3.4-64/bin/*.dll"
            "${MYCAE_OCC_THIRDPARTY_ROOT}/freeimage-3.18.0-x64/bin/*.dll"
            "${MYCAE_OCC_THIRDPARTY_ROOT}/freetype-2.13.3-x64/bin/*.dll"
            "${MYCAE_OCC_THIRDPARTY_ROOT}/jemalloc-vc14-64/bin/*.dll"
            "${MYCAE_OCC_THIRDPARTY_ROOT}/openvr-1.14.15-64/bin/win64/*.dll"
            "${MYCAE_OCC_THIRDPARTY_ROOT}/tbb-2021.13.0-x64/bin/*.dll"
            "${MYCAE_OCC_THIRDPARTY_ROOT}/angle-gles2-2.1.0-vc14-64/bin/libEGL.dll"
            "${MYCAE_OCC_THIRDPARTY_ROOT}/angle-gles2-2.1.0-vc14-64/bin/libGLESv2.dll"
            "${MYCAE_OCC_THIRDPARTY_ROOT}/angle-gles2-2.1.0-vc14-64/bin/d3dcompiler_47.dll"
            "${MYCAE_OCC_THIRDPARTY_ROOT}/tcltk-8.6.15-x64/bin/tcl86.dll"
            "${MYCAE_OCC_THIRDPARTY_ROOT}/tcltk-8.6.15-x64/bin/tk86.dll"
        )
        add_custom_command(TARGET MyCAE POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${MYCAE_OCC_THIRDPARTY_RUNTIME_DLLS}
                $<TARGET_FILE_DIR:MyCAE>
            COMMAND_EXPAND_LISTS
        )
    endif()
    
    # Copy additional Qt DLLs that may be needed by VTK plugins
    if (QT_BIN_DIR)
        file(GLOB MYCAE_EXTRA_QT_DLLS
            "${QT_BIN_DIR}/Qt6Quick.dll"
            "${QT_BIN_DIR}/Qt6Sql.dll"
        )
        add_custom_command(TARGET MyCAE POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${MYCAE_EXTRA_QT_DLLS}
                $<TARGET_FILE_DIR:MyCAE>
            COMMAND_EXPAND_LISTS
        )
    endif()
endif()

if (MSVC)
    target_compile_options(MyCAE PRIVATE /W4)
else()
    target_compile_options(MyCAE PRIVATE -Wall -Wextra -Wpedantic)
endif()
