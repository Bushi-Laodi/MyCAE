cmake_minimum_required(VERSION 3.21)

project(MyCAE VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)

set(MYCAE_QT_ROOT "D:\\dev\\Qt\\6.6.3\\msvc2019_64" CACHE PATH "Root directory of the local Qt 6 installation")
if (EXISTS "${MYCAE_QT_ROOT}/lib/cmake/Qt6/Qt6Config.cmake")
    list(PREPEND CMAKE_PREFIX_PATH "${MYCAE_QT_ROOT}")
endif()

set(MYCAE_VTK_ROOT "D:\\dev\\VTK-9.4.2" CACHE PATH "Root directory of the local VTK installation")
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

set(MYCAE_OCC_ROOT "D:\\dev\\OCC" CACHE PATH "Root directory of the local Open CASCADE installation")
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
    "D:\\dev\\Gmsh\\gmsh-4.15.2-Windows64-sdk\\bin\\gmsh.exe"
    CACHE FILEPATH "Path to the local Gmsh executable"
)
set(MYCAE_CALCULIX_EXECUTABLE
    "E:\\tools\\Anaconda\\envs\\calculix\\Library\\bin\\ccx.exe"
    CACHE FILEPATH "Path to the local CalculiX ccx executable"
)


find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
find_package(VTK REQUIRED COMPONENTS
    FiltersSources
    GUISupportQt
    InteractionStyle
    IOLegacy
    RenderingAnnotation
    RenderingCore
    RenderingOpenGL2
)
find_package(OpenCASCADE REQUIRED)

message(STATUS "Using VTK_DIR=${VTK_DIR}")
message(STATUS "Using OpenCASCADE_DIR=${OpenCASCADE_DIR}")
message(STATUS "Using OpenCASCADE_INCLUDE_DIR=${OpenCASCADE_INCLUDE_DIR}")
message(STATUS "Using OpenCASCADE_BINARY_DIR=${OpenCASCADE_BINARY_DIR}")
message(STATUS "Using MYCAE_GMSH_EXECUTABLE=${MYCAE_GMSH_EXECUTABLE}")
message(STATUS "Using MYCAE_CALCULIX_EXECUTABLE=${MYCAE_CALCULIX_EXECUTABLE}")

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
    TKBO
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
    src/geometry/FaceGroupReferenceService.cpp
    src/geometry/FaceGroupReferenceService.h
    src/geometry/FaceGroupService.cpp
    src/geometry/FaceGroupService.h
    src/geometry/FaceReferenceUtils.cpp
    src/geometry/FaceReferenceUtils.h
    src/geometry/GeometryBooleanController.cpp
    src/geometry/GeometryBooleanController.h
    src/geometry/GeometryBooleanOperation.h
    src/geometry/GeometryCreationController.cpp
    src/geometry/GeometryCreationController.h
    src/geometry/GeometryDisplayController.cpp
    src/geometry/GeometryDisplayController.h
    src/geometry/GeometryManager.cpp
    src/geometry/GeometryManager.h
    src/geometry/GeometryObject.h
    src/geometry/GeometryPropertyController.cpp
    src/geometry/GeometryPropertyController.h
    src/geometry/SphereGeometry.h
    src/commands/AppCommand.h
    src/commands/CommandUtilities.cpp
    src/commands/CommandUtilities.h
    src/commands/DisplayModeCommands.cpp
    src/commands/DisplayModeCommands.h
    src/commands/GeometryCommands.cpp
    src/commands/GeometryCommands.h
    src/commands/FaceGroupCommandUtilities.cpp
    src/commands/FaceGroupCommandUtilities.h
    src/commands/FaceGroupEditCommands.cpp
    src/commands/FaceGroupEditCommands.h
    src/commands/MeshCommands.cpp
    src/commands/MeshCommands.h
    src/commands/PickModeCommands.cpp
    src/commands/PickModeCommands.h
    src/commands/ProjectCommands.cpp
    src/commands/ProjectCommands.h
    src/commands/SolverCommands.cpp
    src/commands/SolverCommands.h
    src/commands/UndoStackController.cpp
    src/commands/UndoStackController.h
    src/commands/UtilityCommands.cpp
    src/commands/UtilityCommands.h
    src/commands/WorkflowCommandContext.h
    src/diagnostics/DiagnosticCollector.cpp
    src/diagnostics/DiagnosticCollector.h
    src/diagnostics/DiagnosticMessage.cpp
    src/diagnostics/DiagnosticMessage.h
    src/mesh/GmshCaseWriter.cpp
    src/mesh/GmshCaseWriter.h
    src/mesh/MeshBoundaryBuilder.cpp
    src/mesh/MeshBoundaryBuilder.h
    src/mesh/MeshBoundary.h
    src/mesh/GmshRunner.cpp
    src/mesh/GmshRunner.h
    src/mesh/MeshData.h
    src/mesh/MeshManager.cpp
    src/mesh/MeshManager.h
    src/mesh/MeshObject.h
    src/mesh/MeshQualityChecker.cpp
    src/mesh/MeshQualityChecker.h
    src/mesh/MeshSetup.h
    src/mesh/MeshToVtkConverter.cpp
    src/mesh/MeshToVtkConverter.h
    src/mesh/MshReader.cpp
    src/mesh/MshReader.h
    src/occ/OCCBoxBuilder.cpp
    src/occ/OCCBoxBuilder.h
    src/occ/OCCBooleanBuilder.cpp
    src/occ/OCCBooleanBuilder.h
    src/occ/OCCCylinderBuilder.cpp
    src/occ/OCCCylinderBuilder.h
    src/occ/OCCGeometryFactory.cpp
    src/occ/OCCGeometryFactory.h
    src/occ/OCCShapeConverter.cpp
    src/occ/OCCShapeConverter.h
    src/occ/OCCShapeIO.cpp
    src/occ/OCCShapeIO.h
    src/occ/OCCSphereBuilder.cpp
    src/occ/OCCSphereBuilder.h
    src/project/GeometryRepository.cpp
    src/project/GeometryRepository.h
    src/project/MeshRepository.cpp
    src/project/MeshRepository.h
    src/project/Project.h
    src/project/ProjectContext.cpp
    src/project/ProjectContext.h
    src/project/ProjectModel.cpp
    src/project/ProjectModel.h
    src/project/ProjectModelLoader.cpp
    src/project/ProjectModelLoader.h
    src/project/ProjectManager.cpp
    src/project/ProjectManager.h
    src/project/ProjectResourceManager.cpp
    src/project/ProjectResourceManager.h
    src/project/ResultRepository.cpp
    src/project/ResultRepository.h
    src/project/SelectionState.cpp
    src/project/SelectionState.h
    src/project/SolverRepository.cpp
    src/project/SolverRepository.h
    src/picking/PickController.cpp
    src/picking/PickController.h
    src/picking/PickMode.h
    src/picking/PickSelection.h
    src/render/RenderHighlightController.cpp
    src/render/RenderHighlightController.h
    src/render/VtkFaceGeometry.cpp
    src/render/VtkFaceGeometry.h
    src/render/VtkFaceReferenceResolver.cpp
    src/render/VtkFaceReferenceResolver.h
    src/render/VtkHighlightActorFactory.cpp
    src/render/VtkHighlightActorFactory.h
    src/render/VtkPickAdapter.cpp
    src/render/VtkPickAdapter.h
    src/result/ResultAnimationController.cpp
    src/result/ResultAnimationController.h
    src/result/ResultCsvExporter.cpp
    src/result/ResultCsvExporter.h
    src/result/ResultDataLoader.cpp
    src/result/ResultDataLoader.h
    src/result/ResultDisplayCache.cpp
    src/result/ResultDisplayCache.h
    src/result/ResultDisplayController.cpp
    src/result/ResultDisplayController.h
    src/result/ResultDisplaySummary.cpp
    src/result/ResultDisplaySummary.h
    src/result/ResultFieldMetadata.cpp
    src/result/ResultFieldMetadata.h
    src/result/ResultExtrema.h
    src/result/ResultExtremaCalculator.cpp
    src/result/ResultExtremaCalculator.h
    src/result/ResultManager.cpp
    src/result/ResultManager.h
    src/result/ResultObject.h
    src/result/ResultProbe.h
    src/result/ResultReportExporter.cpp
    src/result/ResultReportExporter.h
    src/solver/BoundaryCondition.h
    src/solver/BoundaryBindingInspector.cpp
    src/solver/BoundaryBindingInspector.h
    src/solver/Load.h
    src/solver/Material.h
    src/solver/SimulationCase.h
    src/solver/SimulationCaseBuilder.cpp
    src/solver/SimulationCaseBuilder.h
    src/solver/SimulationCaseJsonReader.cpp
    src/solver/SimulationCaseJsonReader.h
    src/solver/SimulationCaseJsonWriter.cpp
    src/solver/SimulationCaseJsonWriter.h
    src/solver/SimulationCaseManager.cpp
    src/solver/SimulationCaseManager.h
    src/solver/SolverDataService.cpp
    src/solver/SolverDataService.h
    src/solver/calculix/CalculiXCasePaths.cpp
    src/solver/calculix/CalculiXCasePaths.h
    src/solver/calculix/CalculiXCaseData.h
    src/solver/calculix/CalculiXCaseDataBuilder.cpp
    src/solver/calculix/CalculiXCaseDataBuilder.h
    src/solver/calculix/CalculiXCaseWriter.cpp
    src/solver/calculix/CalculiXCaseWriter.h
    src/solver/calculix/CalculiXBoundaryMapper.cpp
    src/solver/calculix/CalculiXBoundaryMapper.h
    src/solver/calculix/CalculiXDeckFormatting.cpp
    src/solver/calculix/CalculiXDeckFormatting.h
    src/solver/calculix/CalculiXDeckSectionWriter.cpp
    src/solver/calculix/CalculiXDeckSectionWriter.h
    src/solver/calculix/CalculiXDeckTypes.h
    src/solver/calculix/CalculiXDatResultReader.cpp
    src/solver/calculix/CalculiXDatResultReader.h
    src/solver/calculix/CalculiXEnvironment.cpp
    src/solver/calculix/CalculiXEnvironment.h
    src/solver/calculix/CalculiXInputDeck.cpp
    src/solver/calculix/CalculiXInputDeck.h
    src/solver/calculix/CalculiXInputDeckBuilder.cpp
    src/solver/calculix/CalculiXInputDeckBuilder.h
    src/solver/calculix/CalculiXLoadWriter.cpp
    src/solver/calculix/CalculiXLoadWriter.h
    src/solver/calculix/CalculiXMeshBoundaryResolver.cpp
    src/solver/calculix/CalculiXMeshBoundaryResolver.h
    src/solver/calculix/CalculiXPlugin.cpp
    src/solver/calculix/CalculiXPlugin.h
    src/solver/calculix/CalculiXResultReader.cpp
    src/solver/calculix/CalculiXResultReader.h
    src/solver/calculix/CalculiXResultGridBuilder.cpp
    src/solver/calculix/CalculiXResultGridBuilder.h
    src/solver/calculix/CalculiXResultMath.h
    src/solver/calculix/CalculiXRunDiagnostics.cpp
    src/solver/calculix/CalculiXRunDiagnostics.h
    src/solver/calculix/CalculiXRunner.cpp
    src/solver/calculix/CalculiXRunner.h
    src/solver/calculix/CalculiXSurfaceMapper.cpp
    src/solver/calculix/CalculiXSurfaceMapper.h
    src/solver/export/OpenFoamCaseWriter.cpp
    src/solver/export/OpenFoamCaseWriter.h
    src/solver/export/SolverCaseWriter.cpp
    src/solver/export/SolverCaseWriter.h
    src/solver/export/SolverCaseWriterResult.h
    src/solver/openfoam/OpenFoamPlugin.cpp
    src/solver/openfoam/OpenFoamPlugin.h
    src/solver/openfoam/OpenFoamServiceClient.cpp
    src/solver/openfoam/OpenFoamServiceClient.h
    src/solver/plugin/BuiltInSolverPluginRegistry.cpp
    src/solver/plugin/BuiltInSolverPluginRegistry.h
    src/solver/plugin/ExternalProcessSolverPlugin.cpp
    src/solver/plugin/ExternalProcessSolverPlugin.h
    src/solver/plugin/ExternalSolverPluginConfig.cpp
    src/solver/plugin/ExternalSolverPluginConfig.h
    src/solver/plugin/SolverCapabilities.h
    src/solver/plugin/SolverCaseContext.h
    src/solver/plugin/SolverPlugin.h
    src/solver/plugin/SolverPluginDescriptor.h
    src/solver/plugin/SolverPluginDescriptorFormatter.cpp
    src/solver/plugin/SolverPluginDescriptorFormatter.h
    src/solver/plugin/SolverPluginManager.cpp
    src/solver/plugin/SolverPluginManager.h
    src/solver/plugin/SolverResultReadResult.h
    src/solver/plugin/SolverRunResult.h
    src/ui/BoxDialog.cpp
    src/ui/BoxDialog.h
    src/ui/BooleanOperationDialog.cpp
    src/ui/BooleanOperationDialog.h
    src/ui/BoundaryConditionDialog.cpp
    src/ui/BoundaryConditionDialog.h
    src/ui/ActionRegistry.cpp
    src/ui/ActionRegistry.h
    src/ui/AppSettings.cpp
    src/ui/AppSettings.h
    src/ui/AppStyle.cpp
    src/ui/AppStyle.h
    src/ui/CylinderDialog.cpp
    src/ui/CylinderDialog.h
    src/ui/FaceGroupRestoreController.cpp
    src/ui/FaceGroupRestoreController.h
    src/ui/DiagnosticPanel.cpp
    src/ui/DiagnosticPanel.h
    src/ui/LoadDialog.cpp
    src/ui/LoadDialog.h
    src/ui/LogPanel.cpp
    src/ui/LogPanel.h
    src/ui/MainWindowActions.cpp
    src/ui/MainWindowActions.h
    src/ui/MainWindowDocks.cpp
    src/ui/MainWindowDocks.h
    src/ui/MainWindowLifecycleController.cpp
    src/ui/MainWindowLifecycleController.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/MainWindowMenuBuilder.cpp
    src/ui/MainWindowMenuBuilder.h
    src/ui/MainWindowResultController.cpp
    src/ui/MainWindowResultController.h
    src/ui/MainWindowStateController.cpp
    src/ui/MainWindowStateController.h
    src/ui/MainWindowToolController.cpp
    src/ui/MainWindowToolController.h
    src/ui/MainWindowToolBarBuilder.cpp
    src/ui/MainWindowToolBarBuilder.h
    src/ui/MainWindowViewController.cpp
    src/ui/MainWindowViewController.h
    src/ui/MaterialDialog.cpp
    src/ui/MaterialDialog.h
    src/ui/MeshSetupDialog.cpp
    src/ui/MeshSetupDialog.h
    src/ui/ProjectResourceDialog.cpp
    src/ui/ProjectResourceDialog.h
    src/ui/ProjectTreePanel.cpp
    src/ui/ProjectTreePanel.h
    src/ui/RecentProjectController.cpp
    src/ui/RecentProjectController.h
    src/ui/PropertyPanel.cpp
    src/ui/PropertyPanel.h
    src/ui/ResultPostprocessPanel.cpp
    src/ui/ResultPostprocessPanel.h
    src/ui/ResultPostprocessText.cpp
    src/ui/ResultPostprocessText.h
    src/ui/SampleValidationDialog.cpp
    src/ui/SampleValidationDialog.h
    src/ui/SelectionInteractionController.cpp
    src/ui/SelectionInteractionController.h
    src/ui/SphereDialog.cpp
    src/ui/SphereDialog.h
    src/ui/UiIconFactory.cpp
    src/ui/UiIconFactory.h
    src/ui/property/FaceGroupPropertyView.cpp
    src/ui/property/FaceGroupPropertyView.h
    src/ui/property/GeometryPropertyView.cpp
    src/ui/property/GeometryPropertyView.h
    src/ui/property/MeshPropertyView.cpp
    src/ui/property/MeshPropertyView.h
    src/ui/property/PickPropertyView.cpp
    src/ui/property/PickPropertyView.h
    src/ui/property/ResultPropertyView.cpp
    src/ui/property/ResultPropertyView.h
    src/ui/property/SolverPropertyView.cpp
    src/ui/property/SolverPropertyView.h
    src/ui/RenderView.cpp
    src/ui/RenderGeometryItem.h
    src/ui/RenderView.h
    src/ui/SolverDataController.cpp
    src/ui/SolverDataController.h
    src/ui/VtkRenderCanvas.cpp
    src/ui/VtkRenderCanvas.h
    src/workflow/GeometryWorkflowController.cpp
    src/workflow/GeometryWorkflowController.h
    src/workflow/FaceGroupWorkflowController.cpp
    src/workflow/FaceGroupWorkflowController.h
    src/workflow/MeshWorkflowController.cpp
    src/workflow/MeshWorkflowController.h
    src/workflow/ProjectWorkflowController.cpp
    src/workflow/ProjectWorkflowController.h
    src/workflow/ResultWorkflowController.cpp
    src/workflow/ResultWorkflowController.h
    src/workflow/SelectionController.cpp
    src/workflow/SelectionController.h
    src/workflow/SolverCaseWorkflowController.cpp
    src/workflow/SolverCaseWorkflowController.h
    src/workflow/SolverWorkflowController.cpp
    src/workflow/SolverWorkflowController.h
    src/validation/DemoProjectValidator.cpp
    src/validation/DemoProjectValidator.h
    src/validation/SampleProjectValidator.cpp
    src/validation/SampleProjectValidator.h
    src/validation/UiAutomationSupport.cpp
    src/validation/UiAutomationSupport.h
    src/validation/UiSmokeValidator.cpp
    src/validation/UiSmokeValidator.h
    src/validation/UiSampleScreenshotter.cpp
    src/validation/UiSampleScreenshotter.h
)

target_include_directories(MyCAE PRIVATE src ${OpenCASCADE_INCLUDE_DIR})
target_compile_definitions(MyCAE PRIVATE
    MYCAE_GMSH_EXECUTABLE_PATH="${MYCAE_GMSH_EXECUTABLE}"
    MYCAE_CALCULIX_EXECUTABLE_PATH="${MYCAE_CALCULIX_EXECUTABLE}"
    MYCAE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
target_link_libraries(MyCAE PRIVATE Qt6::Widgets Qt6::Network ${VTK_LIBRARIES} ${MYCAE_OCC_LIBRARIES})

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

add_custom_target(MyCAESamples ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/samples"
        "$<TARGET_FILE_DIR:MyCAE>/samples"
    COMMAND_EXPAND_LISTS
)
add_dependencies(MyCAE MyCAESamples)

if (BUILD_TESTING)
    add_test(
        NAME MyCAEValidateSamples
        COMMAND $<TARGET_FILE:MyCAE> --validate-samples
    )
    set_tests_properties(MyCAEValidateSamples PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:MyCAE>"
        TIMEOUT 180
    )

    add_test(
        NAME MyCAEValidateUi
        COMMAND $<TARGET_FILE:MyCAE> --validate-ui
    )
    set_tests_properties(MyCAEValidateUi PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:MyCAE>"
        TIMEOUT 60
    )

    add_test(
        NAME MyCAECaptureUiSample
        COMMAND $<TARGET_FILE:MyCAE> --capture-ui-sample --ui-screenshot-dir "$<TARGET_FILE_DIR:MyCAE>/ui_screenshots"
    )
    set_tests_properties(MyCAECaptureUiSample PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:MyCAE>"
        TIMEOUT 90
    )
endif()

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
