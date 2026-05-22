// Minimal Qt-only test - no VTK, no OCC
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QMainWindow window;
    window.resize(400, 300);
    window.setWindowTitle("Qt Only Test");
    
    auto *label = new QLabel("Hello Qt!");
    label->setAlignment(Qt::AlignCenter);
    window.setCentralWidget(label);
    window.show();
    
    return app.exec();
}
