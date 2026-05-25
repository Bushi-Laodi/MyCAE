#include "ui/MainWindow.h"
#include "validation/SampleProjectValidator.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QVTKOpenGLNativeWidget.h>

#include <cstdio>

namespace
{
int runSampleValidation(const QCommandLineParser &parser)
{
    const QString samplesRoot = parser.value("samples-root");
    const QString workRoot = parser.value("validation-work-root");
    const SampleValidationReport report = SampleProjectValidator(samplesRoot, workRoot).validate();

    QTextStream out(stdout);
    out << "MyCAE sample validation\n";
    out << "Passed: " << report.passedCount() << "\n";
    out << "Failed: " << report.failedCount() << "\n\n";
    for (const SampleValidationStep &step : report.steps) {
        out << "[" << sampleValidationStatusName(step.status) << "] " << step.name;
        if (!step.detail.isEmpty()) {
            out << ": " << step.detail;
        }
        out << '\n';
    }
    out.flush();

    return report.success() ? 0 : 1;
}
}

int main(int argc, char *argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);
    QApplication::setApplicationName("MyCAE");
    QApplication::setOrganizationName("MyCAE");

    QCommandLineParser parser;
    parser.setApplicationDescription("MyCAE desktop CAE integration tool.");
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("validate-samples", "Run demo and CalculiX sample validation, then exit."));
    parser.addOption(QCommandLineOption("samples-root", "Samples root directory.", "path"));
    parser.addOption(QCommandLineOption("validation-work-root", "Validation work directory.", "path"));
    parser.process(app);

    if (parser.isSet("validate-samples")) {
        return runSampleValidation(parser);
    }

    MainWindow window;
    window.show();

    return app.exec();
}
