#include "ui/MainWindow.h"
#include "validation/SampleProjectValidator.h"
#include "validation/UiSmokeValidator.h"

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

int runUiValidation()
{
    const UiValidationReport report = UiSmokeValidator().validate();

    QTextStream out(stdout);
    out << "MyCAE UI validation\n";
    out << "Passed: " << report.passedCount() << "\n";
    out << "Failed: " << report.failedCount() << "\n\n";
    for (const UiValidationStep &step : report.steps) {
        out << "[" << uiValidationStatusName(step.passed) << "] " << step.name;
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
    parser.addOption(QCommandLineOption("validate-ui", "Run UI smoke validation, then exit."));
    parser.addOption(QCommandLineOption("samples-root", "Samples root directory.", "path"));
    parser.addOption(QCommandLineOption("validation-work-root", "Validation work directory.", "path"));
    parser.process(app);

    if (parser.isSet("validate-samples")) {
        return runSampleValidation(parser);
    }
    if (parser.isSet("validate-ui")) {
        return runUiValidation();
    }

    MainWindow window;
    window.show();

    return app.exec();
}
