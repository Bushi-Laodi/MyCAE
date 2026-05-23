// Minimal test to isolate the crash
// Compile: cl /EHsc /I"D:\Tools\Qt\6.5.3\msvc2019_64\include" /I"D:\Tools\VTK\vtk-9.5.2-qt-release\include\vtk-9.5" test_minimal.cpp /link /LIBPATH:"D:\Tools\Qt\6.5.3\msvc2019_64\lib" /LIBPATH:"D:\Tools\VTK\vtk-9.5.2-qt-release\lib" Qt6Widgets.lib Qt6Core.lib vtkRenderingCore-9.5.lib vtkRenderingOpenGL2-9.5.lib vtkGUISupportQt-9.5.lib

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QMainWindow>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>

int main(int argc, char *argv[])
{
    // Test 1: Just create QApplication
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    QApplication app(argc, argv);

    // Test 2: Create VTK objects without widget
    auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // Test 3: Create QVTKOpenGLNativeWidget
    QMainWindow window;
    auto *vtkWidget = new QVTKOpenGLNativeWidget(&window);
    vtkWidget->setRenderWindow(renderWindow);
    window.setCentralWidget(vtkWidget);
    window.show();

    return app.exec();
}
