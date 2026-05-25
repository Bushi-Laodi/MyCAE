#include "validation/SampleProjectValidator.h"

#include "solver/calculix/CalculiXDatResultReader.h"
#include "solver/calculix/CalculiXEnvironment.h"
#include "validation/DemoProjectValidator.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>

#include <utility>

namespace
{
constexpr int SampleRunTimeoutMs = 2 * 60 * 1000;

struct CalculiXSampleDefinition
{
    QString id;
    QString deckFileName;
    bool requiresStress = true;
};

struct SampleRunOutput
{
    QString workDirectory;
    QString jobName;
    QString logFile;
    QString standardError;
    int exitCode = -1;
    bool started = false;
    bool timedOut = false;
    bool normalExit = false;
};

QVector<CalculiXSampleDefinition> sampleDefinitions()
{
    return {
        {"single_tetra", "single_tetra.inp", true},
        {"box_pressure", "box_pressure.inp", true},
    };
}

QString readTextFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromLocal8Bit(file.readAll());
}

bool containsCompletionMarker(const QString &caseDirectory, const QString &jobName)
{
    const QStringList candidateFiles = {
        QDir(caseDirectory).filePath(jobName + ".sta"),
        QDir(caseDirectory).filePath(jobName + ".dat"),
        QDir(caseDirectory).filePath(jobName + ".log"),
    };

    for (const QString &filePath : candidateFiles) {
        const QString content = readTextFile(filePath).toUpper();
        if (content.contains("JOB FINISHED") || content.contains("SUMMARY OF JOB INFORMATION")) {
            return true;
        }
    }
    return false;
}

bool copySampleDeck(const QString &sourceDeck, const QString &targetDeck, QString *errorMessage)
{
    QFile::remove(targetDeck);
    if (QFile::copy(sourceDeck, targetDeck)) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QFileInfo(sourceDeck).exists()
            ? QString("cannot copy sample input deck to %1").arg(targetDeck)
            : QString("sample input deck does not exist: %1").arg(sourceDeck);
    }
    return false;
}

bool writeRunLog(
    const QString &logFile,
    const QString &command,
    const QString &workingDirectory,
    int exitCode,
    const QString &standardOutput,
    const QString &standardError
)
{
    QFile file(logFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream stream(&file);
    stream << "Command: " << command << '\n';
    stream << "Working directory: " << workingDirectory << '\n';
    stream << "Exit code: " << exitCode << "\n\n";
    stream << "[stdout]\n" << standardOutput << "\n\n";
    stream << "[stderr]\n" << standardError << '\n';
    return true;
}

class SampleValidationSession
{
public:
    SampleValidationSession(QString samplesRoot, QString workRoot)
        : m_samplesRoot(std::move(samplesRoot))
        , m_workRoot(std::move(workRoot))
    {
    }

    SampleValidationReport validate()
    {
        validateRoots();
        if (m_report.failedCount() > 0) {
            return m_report;
        }

        DemoProjectValidator(m_samplesRoot).validate(m_report);
        validateCalculiXExecutable();
        if (m_report.failedCount() > 0) {
            return m_report;
        }

        for (const CalculiXSampleDefinition &sample : sampleDefinitions()) {
            validateSample(sample);
        }
        return m_report;
    }

private:
    void addBooleanStep(bool passed, const QString &name, const QString &detail = {})
    {
        m_report.addStep(passed ? SampleValidationStatus::Pass : SampleValidationStatus::Fail, name, detail);
    }

    void validateRoots()
    {
        addBooleanStep(QFileInfo::exists(m_samplesRoot), "sample root found", m_samplesRoot);
        if (!QFileInfo::exists(m_samplesRoot)) {
            return;
        }

        if (!QDir().mkpath(m_workRoot)) {
            m_report.addStep(SampleValidationStatus::Fail, "sample work root created", m_workRoot);
            return;
        }
        m_report.addStep(SampleValidationStatus::Pass, "sample work root created", m_workRoot);
    }

    void validateCalculiXExecutable()
    {
        m_program = CalculiXEnvironment::executablePath();
        if (CalculiXEnvironment::isExplicitExecutablePath(m_program) && !QFileInfo::exists(m_program)) {
            m_report.addStep(SampleValidationStatus::Fail, "CalculiX executable found", m_program);
            return;
        }
        m_report.addStep(SampleValidationStatus::Pass, "CalculiX executable found", m_program);
    }

    void validateSample(const CalculiXSampleDefinition &sample)
    {
        const QString sourceDeck = QDir(m_samplesRoot).filePath("calculix/" + sample.deckFileName);
        if (!QFileInfo::exists(sourceDeck)) {
            m_report.addStep(SampleValidationStatus::Fail, sample.id + " input deck found", sourceDeck);
            return;
        }
        m_report.addStep(SampleValidationStatus::Pass, sample.id + " input deck found", sourceDeck);

        const QString sampleWorkDir = prepareWorkDirectory(sample);
        if (sampleWorkDir.isEmpty()) {
            return;
        }

        const QString targetDeck = QDir(sampleWorkDir).filePath(sample.deckFileName);
        QString copyError;
        if (!copySampleDeck(sourceDeck, targetDeck, &copyError)) {
            m_report.addStep(SampleValidationStatus::Fail, sample.id + " input deck copied", copyError);
            return;
        }

        const SampleRunOutput runOutput = runSample(sample, sampleWorkDir);
        if (!validateRun(sample, runOutput)) {
            return;
        }
        if (!validateResultFiles(sample, runOutput)) {
            return;
        }
        validateResultFields(sample, runOutput);
    }

    QString prepareWorkDirectory(const CalculiXSampleDefinition &sample)
    {
        const QString sampleWorkDir = QDir(m_workRoot).filePath(sample.id);
        QDir(sampleWorkDir).removeRecursively();
        if (!QDir().mkpath(sampleWorkDir)) {
            m_report.addStep(SampleValidationStatus::Fail, sample.id + " work directory created", sampleWorkDir);
            return {};
        }
        return sampleWorkDir;
    }

    SampleRunOutput runSample(const CalculiXSampleDefinition &sample, const QString &sampleWorkDir)
    {
        SampleRunOutput output;
        output.workDirectory = sampleWorkDir;
        output.jobName = QFileInfo(sample.deckFileName).completeBaseName();
        output.logFile = QDir(sampleWorkDir).filePath(output.jobName + ".log");

        QProcess process;
        const QStringList arguments = {"-i", output.jobName};
        process.setProgram(m_program);
        process.setArguments(arguments);
        process.setWorkingDirectory(sampleWorkDir);
        process.setProcessEnvironment(CalculiXEnvironment::processEnvironment(m_program));

        process.start();
        output.started = process.waitForStarted();
        if (!output.started) {
            output.standardError = process.errorString();
            return output;
        }

        output.timedOut = !process.waitForFinished(SampleRunTimeoutMs);
        if (output.timedOut) {
            process.kill();
            process.waitForFinished();
        }

        output.exitCode = process.exitCode();
        output.normalExit = process.exitStatus() == QProcess::NormalExit;
        const QString standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
        output.standardError = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();

        const QString command = m_program + " " + arguments.join(' ');
        if (!writeRunLog(output.logFile, command, sampleWorkDir, output.exitCode, standardOutput, output.standardError)) {
            m_report.logMessages.append("Sample validation warning: cannot write run log: " + output.logFile);
        }
        return output;
    }

    bool validateRun(const CalculiXSampleDefinition &sample, const SampleRunOutput &runOutput)
    {
        if (!runOutput.started) {
            m_report.addStep(SampleValidationStatus::Fail, sample.id + " solved", runOutput.standardError);
            return false;
        }
        if (runOutput.timedOut) {
            m_report.addStep(SampleValidationStatus::Fail, sample.id + " solved", "CalculiX sample run timed out.");
            return false;
        }
        if (!runOutput.normalExit || runOutput.exitCode != 0) {
            m_report.addStep(SampleValidationStatus::Fail, sample.id + " solved",
                QString("exitCode=%1; log=%2").arg(runOutput.exitCode).arg(runOutput.logFile));
            if (!runOutput.standardError.isEmpty()) {
                m_report.logMessages.append("Sample validation failed: " + runOutput.standardError);
            }
            return false;
        }
        if (!containsCompletionMarker(runOutput.workDirectory, runOutput.jobName)) {
            m_report.addStep(SampleValidationStatus::Fail, sample.id + " solved",
                "completion marker was not found in .sta/.dat/.log");
            return false;
        }
        m_report.addStep(SampleValidationStatus::Pass, sample.id + " solved", runOutput.workDirectory);
        return true;
    }

    bool validateResultFiles(const CalculiXSampleDefinition &sample, const SampleRunOutput &runOutput)
    {
        const QString datFile = resultFile(runOutput, "dat");
        const QString staFile = resultFile(runOutput, "sta");
        const QString frdFile = resultFile(runOutput, "frd");
        if (!QFileInfo::exists(datFile) || !QFileInfo::exists(staFile) || !QFileInfo::exists(frdFile)) {
            m_report.addStep(SampleValidationStatus::Fail, sample.id + " result files generated",
                QString(".dat=%1, .sta=%2, .frd=%3")
                    .arg(QFileInfo::exists(datFile) ? "yes" : "no")
                    .arg(QFileInfo::exists(staFile) ? "yes" : "no")
                    .arg(QFileInfo::exists(frdFile) ? "yes" : "no"));
            return false;
        }
        m_report.addStep(SampleValidationStatus::Pass, sample.id + " result files generated", runOutput.workDirectory);
        return true;
    }

    void validateResultFields(const CalculiXSampleDefinition &sample, const SampleRunOutput &runOutput)
    {
        const CalculiXDatReadResult datRead = CalculiXDatResultReader().read(resultFile(runOutput, "dat"));
        if (!datRead.success) {
            m_report.addStep(SampleValidationStatus::Fail, sample.id + " displacement field available",
                datRead.errors.join("; "));
            return;
        }
        m_report.addStep(SampleValidationStatus::Pass, sample.id + " displacement field available",
            QString("nodes=%1").arg(datRead.result.displacements.size()));

        if (sample.requiresStress && datRead.result.stresses.empty()) {
            m_report.addStep(SampleValidationStatus::Fail, sample.id + " Von Mises field available",
                datRead.warnings.join("; "));
            return;
        }
        m_report.addStep(SampleValidationStatus::Pass, sample.id + " Von Mises field available",
            QString("elements=%1").arg(datRead.result.stresses.size()));
    }

    static QString resultFile(const SampleRunOutput &runOutput, const QString &suffix)
    {
        return QDir(runOutput.workDirectory).filePath(runOutput.jobName + "." + suffix);
    }

    QString m_samplesRoot;
    QString m_workRoot;
    QString m_program;
    SampleValidationReport m_report;
};
}

void SampleValidationReport::addStep(SampleValidationStatus status, const QString &name, const QString &detail)
{
    steps.append(SampleValidationStep{status, name, detail});
    logMessages.append(QString("[%1] %2%3")
        .arg(sampleValidationStatusName(status), name, detail.isEmpty() ? QString() : ": " + detail));
}

int SampleValidationReport::passedCount() const
{
    int count = 0;
    for (const SampleValidationStep &step : steps) {
        if (step.status == SampleValidationStatus::Pass) {
            ++count;
        }
    }
    return count;
}

int SampleValidationReport::failedCount() const
{
    int count = 0;
    for (const SampleValidationStep &step : steps) {
        if (step.status == SampleValidationStatus::Fail) {
            ++count;
        }
    }
    return count;
}

bool SampleValidationReport::success() const
{
    return !steps.isEmpty() && failedCount() == 0;
}

QString sampleValidationStatusName(SampleValidationStatus status)
{
    switch (status) {
    case SampleValidationStatus::Pass:
        return "PASS";
    case SampleValidationStatus::Fail:
        return "FAIL";
    case SampleValidationStatus::Skip:
        return "SKIP";
    }
    return "SKIP";
}

SampleProjectValidator::SampleProjectValidator(QString samplesRoot, QString workRoot)
    : m_samplesRoot(samplesRoot.isEmpty() ? defaultSamplesRoot() : std::move(samplesRoot))
    , m_workRoot(workRoot.isEmpty() ? defaultWorkRoot() : std::move(workRoot))
{
}

SampleValidationReport SampleProjectValidator::validate() const
{
    return SampleValidationSession(m_samplesRoot, m_workRoot).validate();
}

QString SampleProjectValidator::defaultSamplesRoot()
{
    const QString appSamplesRoot = QDir(QCoreApplication::applicationDirPath()).filePath("samples");
    if (QFileInfo::exists(appSamplesRoot)) {
        return appSamplesRoot;
    }

#ifdef MYCAE_SOURCE_DIR
    const QString sourceSamplesRoot = QDir(QString::fromUtf8(MYCAE_SOURCE_DIR)).filePath("samples");
    if (QFileInfo::exists(sourceSamplesRoot)) {
        return sourceSamplesRoot;
    }
#endif

    return appSamplesRoot;
}

QString SampleProjectValidator::defaultWorkRoot()
{
    const QString tempRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    return QDir(tempRoot).filePath("MyCAE/sample_validation/" + stamp);
}
