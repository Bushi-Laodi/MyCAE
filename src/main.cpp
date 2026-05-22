#include "ui/MainWindow.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

#include <cstdio>
#include <windows.h>

// Debug output helper - writes to both OutputDebugString and console
// This helps us see where the crash occurs
#define DEBUG_MSG(msg) \
    do { \
        OutputDebugStringA("[MyCAE] " msg "\n"); \
        FILE *f = nullptr; \
        if (fopen_s(&f, "debug_log.txt", "a") == 0 && f) { \
            fprintf(f, "[MyCAE] " msg "\n"); \
            fclose(f); \
        } \
    } while(0)

int main(int argc, char *argv[])
{
    // Open console for debug output
    AllocConsole();
    FILE *fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);

    printf("[MyCAE] main() entered\n");
    DEBUG_MSG("main() entered");

    // Step 1: Set surface format
    printf("[MyCAE] Step 1: Setting surface format...\n");
    DEBUG_MSG("Step 1: Setting surface format");
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    printf("[MyCAE] Step 1: OK\n");

    // Step 2: Create QApplication
    printf("[MyCAE] Step 2: Creating QApplication...\n");
    DEBUG_MSG("Step 2: Creating QApplication");
    QApplication app(argc, argv);
    QApplication::setApplicationName("MyCAE");
    QApplication::setOrganizationName("MyCAE 学习项目");
    printf("[MyCAE] Step 2: OK\n");

    // Step 3: Create MainWindow
    printf("[MyCAE] Step 3: Creating MainWindow...\n");
    DEBUG_MSG("Step 3: Creating MainWindow");
    MainWindow window;
    printf("[MyCAE] Step 3: OK\n");

    // Step 4: Show window
    printf("[MyCAE] Step 4: Showing window...\n");
    DEBUG_MSG("Step 4: Showing window");
    window.show();
    printf("[MyCAE] Step 4: OK\n");

    // Step 5: Enter event loop
    printf("[MyCAE] Step 5: Entering event loop...\n");
    DEBUG_MSG("Step 5: Entering event loop");
    int ret = app.exec();
    printf("[MyCAE] Exited with code %d\n", ret);

    return ret;
}
