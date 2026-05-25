#include "result/ResultReportExporter.h"

#include "project/Project.h"
#include "result/ResultObject.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>

namespace
{
QString currentFieldName(const ResultObject &resultObject)
{
    return resultObject.displayFieldName.isEmpty()
        ? resultObject.primaryFieldName
        : resultObject.displayFieldName;
}

QString relativeProjectPath(const Project &project, const QString &path)
{
    if (path.isEmpty()) {
        return {};
    }
    return QDir::toNativeSeparators(QDir(project.rootPath).relativeFilePath(path));
}

QString markdownRelativePath(const QString &reportPath, const QString &targetPath)
{
    QString relativePath = QDir(QFileInfo(reportPath).absolutePath()).relativeFilePath(targetPath);
    relativePath.replace('\\', '/');
    return relativePath;
}

QString nodeExtremeText(const ResultNodeExtreme &extreme)
{
    if (!extreme.valid) {
        return "N/A";
    }
    return QString("node %1, value %2, coordinate (%3, %4, %5)")
        .arg(extreme.nodeId)
        .arg(extreme.value, 0, 'g', 10)
        .arg(extreme.x, 0, 'g', 10)
        .arg(extreme.y, 0, 'g', 10)
        .arg(extreme.z, 0, 'g', 10);
}

QString elementExtremeText(const ResultElementExtreme &extreme)
{
    if (!extreme.valid) {
        return "N/A";
    }
    return QString("element %1, value %2, centroid (%3, %4, %5)")
        .arg(extreme.elementId)
        .arg(extreme.value, 0, 'g', 10)
        .arg(extreme.x, 0, 'g', 10)
        .arg(extreme.y, 0, 'g', 10)
        .arg(extreme.z, 0, 'g', 10);
}

QString completenessText(const ResultObject &resultObject)
{
    QStringList lines;
    lines.append(QString("- Result files: %1").arg(resultObject.resultFilesComplete ? "complete" : "incomplete"));
    lines.append(QString("- Node displacement coverage: %1/%2")
        .arg(resultObject.matchedNodeCount)
        .arg(resultObject.meshNodeCount));
    lines.append(QString("- Element stress coverage: %1/%2")
        .arg(resultObject.matchedElementCount)
        .arg(resultObject.meshElementCount));

    if (resultObject.checkMessages.isEmpty()) {
        lines.append("- Checks: OK");
    } else {
        for (const QString &message : resultObject.checkMessages) {
            lines.append("- Check: " + message);
        }
    }
    return lines.join('\n');
}
}

ResultReportExportResult ResultReportExporter::exportMarkdown(
    const Project &project,
    const ResultObject &resultObject,
    const QString &reportPath,
    const QString &screenshotPath
) const
{
    ResultReportExportResult exportResult;
    exportResult.reportPath = reportPath;

    if (reportPath.isEmpty()) {
        exportResult.errorMessage = "Report path is empty.";
        return exportResult;
    }

    const QString reportDir = QFileInfo(reportPath).absolutePath();
    if (!QDir().mkpath(reportDir)) {
        exportResult.errorMessage = "Create report directory failed: " + reportDir;
        return exportResult;
    }

    QFile file(reportPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        exportResult.errorMessage = "Open report failed: " + file.errorString();
        return exportResult;
    }

    QTextStream stream(&file);
    stream << "# MyCAE Result Report\n\n";
    stream << "## Project\n\n";
    stream << "- Project name: " << project.name << '\n';
    stream << "- Solver: " << resultObject.solverName << '\n';
    stream << "- Case path: " << relativeProjectPath(project, resultObject.casePath) << '\n';
    stream << "- Mesh: " << resultObject.meshName << '\n';
    stream << "- Result: " << resultObject.name << "\n\n";

    stream << "## Display State\n\n";
    stream << "- Current field: " << currentFieldName(resultObject) << '\n';
    stream << "- Scalar min: " << resultObject.scalarMin << '\n';
    stream << "- Scalar max: " << resultObject.scalarMax << '\n';
    stream << "- Deformation scale: " << resultObject.deformationScale << "\n\n";

    stream << "## Extrema\n\n";
    stream << "- Maximum displacement: " << nodeExtremeText(resultObject.extrema.maxDisplacementMagnitude) << '\n';
    stream << "- Maximum Von Mises stress: " << elementExtremeText(resultObject.extrema.maxVonMisesStress) << "\n\n";

    stream << "## Completeness\n\n";
    stream << completenessText(resultObject) << "\n\n";

    stream << "## Screenshot\n\n";
    if (screenshotPath.isEmpty()) {
        stream << "- Screenshot path: N/A\n";
    } else {
        stream << "- Screenshot path: " << markdownRelativePath(reportPath, screenshotPath) << "\n\n";
        stream << "![Current result](" << markdownRelativePath(reportPath, screenshotPath) << ")\n";
    }

    exportResult.success = true;
    return exportResult;
}
