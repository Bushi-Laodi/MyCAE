cmake_minimum_required(VERSION 3.21)

project(MyCAE VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MYCAE_QT_ROOT "D:/Qt/6.6.3/msvc2019_64" CACHE PATH "Root directory of the local Qt 6 installation")
if (EXISTS "${MYCAE_QT_ROOT}/lib/cmake/Qt6/Qt6Config.cmake")
    list(PREPEND CMAKE_PREFIX_PATH "${MYCAE_QT_ROOT}")
endif()

set(MYCAE_VTK_ROOT "D:/VTK" CACHE PATH "Root directory of the local VTK installation")
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

set(MYCAE_OCC_ROOT "D:/cpp_lib/OCC" CACHE PATH "Root directory of the local Open CASCADE installation")
set(MYCAE_OCC_CMAKE_CANDIDATES
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
    src/geometry/GeometryManager.cpp
    src/geometry/GeometryManager.h
    src/occ/OCCBoxBuilder.cpp
    src/occ/OCCBoxBuilder.h
    src/occ/OCCGeometryFactory.cpp
    src/occ/OCCGeometryFactory.h
    src/occ/OCCShapeConverter.cpp
    src/occ/OCCShapeConverter.h
    src/occ/OCCShapeIO.cpp
    src/occ/OCCShapeIO.h
    src/project/Project.h
    src/project/ProjectManager.cpp
    src/project/ProjectManager.h
    src/ui/BoxDialog.cpp
    src/ui/BoxDialog.h
    src/ui/LogPanel.cpp
    src/ui/LogPanel.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/ProjectTreePanel.cpp
    src/ui/ProjectTreePanel.h
    src/ui/PropertyPanel.cpp
    src/ui/PropertyPanel.h
    src/ui/RenderView.cpp
    src/ui/RenderView.h
    src/ui/VtkRenderCanvas.cpp
    src/ui/VtkRenderCanvas.h
)

target_include_directories(MyCAE PRIVATE src ${OpenCASCADE_INCLUDE_DIR})
target_link_libraries(MyCAE PRIVATE Qt6::Widgets ${VTK_LIBRARIES} ${OpenCASCADE_LIBRARIES})

vtk_module_autoinit(
    TARGETS MyCAE
    MODULES ${VTK_LIBRARIES}
)

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
endif()

if (MSVC)
    target_compile_options(MyCAE PRIVATE /W4)
else()
    target_compile_options(MyCAE PRIVATE -Wall -Wextra -Wpedantic)
endif()
