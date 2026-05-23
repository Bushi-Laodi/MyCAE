#include "ui/MainWindow.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char *argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);
    QApplication::setApplicationName("MyCAE");
    QApplication::setOrganizationName("MyCAE");

    MainWindow window;
    window.show();

    return app.exec();
}
