#include "solver/calculix/CalculiXRunDiagnostics.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

namespace
{
QString readTextFile(const QString &filePath, qsizetype maxBytes = 256 * 1024)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    QByteArray content = file.read(maxBytes);
    if (!file.atEnd()) {
        content.append("\n...[truncated]...\n");
    }
    return QString::fromLocal8Bit(content);
}

QString joinedRunText(const CalculiXCasePaths &paths, const SolverRunResult &runResult)
{
    QStringList parts;
    parts.append(runResult.standardOutput);
    parts.append(runResult.standardError);
    parts.append(readTextFile(paths.logFile));
    parts.append(readTextFile(paths.staFile));
    parts.append(readTextFile(paths.datFile));
    return parts.join('\n');
}

bool containsAny(const QString &text, const QStringList &patterns)
{
    for (const QString &pattern : patterns) {
        if (text.contains(pattern)) {
            return true;
        }
    }
    return false;
}

QString compactLine(QString line)
{
    return line.trimmed().replace(QRegularExpression("\\s+"), " ");
}

QString firstDiagnosticBlock(const QString &text, const QStringList &markers, int contextLines)
{
    const QStringList lines = text.split('\n');
    for (int index = 0; index < lines.size(); ++index) {
        const QString lowerLine = lines.at(index).toLower();
        if (!containsAny(lowerLine, markers)) {
            continue;
        }

        QStringList block;
        for (int blockIndex = index; blockIndex < lines.size() && block.size() < contextLines; ++blockIndex) {
            const QString line = compactLine(lines.at(blockIndex));
            if (!line.isEmpty()) {
                block.append(line);
            }
        }
        return block.join(" | ");
    }
    return {};
}

void appendUnique(QStringList &items, const QString &item)
{
    if (!item.trimmed().isEmpty() && !items.contains(item)) {
        items.append(item);
    }
}

QStringList missingFiles(const CalculiXCasePaths &paths)
{
    QStringList missing;
    const QList<QPair<QString, QString>> files{
        {"input", paths.inputFile},
        {"log", paths.logFile},
        {"sta", paths.staFile},
        {"dat", paths.datFile},
        {"frd", paths.frdFile},
    };
    for (const auto &file : files) {
        if (!QFileInfo::exists(file.second)) {
            missing.append(file.first + "=" + file.second);
        }
    }
    return missing;
}
}

bool CalculiXRunDiagnosticReport::hasFindings() const
{
    return !errors.isEmpty() || !warnings.isEmpty() || !hints.isEmpty();
}

CalculiXRunDiagnosticReport CalculiXRunDiagnostics::analyze(
    const CalculiXCasePaths &paths,
    const SolverRunResult &runResult
) const
{
    CalculiXRunDiagnosticReport report;
    const QString text = joinedRunText(paths, runResult);
    const QString lower = text.toLower();
    const QString firstErrorBlock = firstDiagnosticBlock(text, {"*error", "error in"}, 5);
    const QString firstWarningBlock = firstDiagnosticBlock(text, {"*warning", "warning"}, 4);

    if (runResult.exitCode == 201) {
        appendUnique(report.errors, "CalculiX diagnostic: exit code 201 usually means the input deck, mesh, material, boundary condition, or load definition is invalid.");
        appendUnique(report.hints, "CalculiX diagnostic hint: open the .dat/.sta/.log files and search for lines starting with *ERROR or *WARNING.");
    } else if (runResult.exitCode != 0) {
        appendUnique(report.errors, QString("CalculiX diagnostic: non-zero exit code %1.").arg(runResult.exitCode));
    }

    if (containsAny(lower, {"*error", "error in", "too many cutbacks", "divergence", "singular", "zero pivot"})) {
        appendUnique(report.errors, "CalculiX diagnostic: solver log contains a numerical or input error marker.");
        if (!firstErrorBlock.isEmpty()) {
            appendUnique(report.errors, "CalculiX diagnostic detail: first error block: " + firstErrorBlock);
        }
    }
    if (containsAny(lower, {"*error reading *node"})) {
        appendUnique(report.errors, "CalculiX diagnostic: CalculiX failed while reading the *NODE section.");
        appendUnique(report.hints, "CalculiX diagnostic hint: exported node coordinates may be too long or contain parser-hostile near-zero scientific notation; regenerate the input deck with compact numeric formatting.");
    }
    if (containsAny(lower, {"> nk", "value            7", "value          7"})) {
        appendUnique(report.hints, "CalculiX diagnostic hint: NSET/ELSET values greater than nk usually mean node parsing stopped early; fix the first *NODE error before checking node sets.");
    }
    if (containsAny(lower, {"zero pivot", "singular matrix", "rigid body motion"})) {
        appendUnique(report.hints, "CalculiX diagnostic hint: the model may be under-constrained. Check fixed supports and rigid body motion.");
    }
    if (containsAny(lower, {"negative jacobian", "jacobian determinant", "element is badly shaped"})) {
        appendUnique(report.hints, "CalculiX diagnostic hint: mesh quality is poor. Regenerate the mesh with smaller size or repair the geometry.");
    }
    if (containsAny(lower, {"material", "young", "elastic", "poisson"})) {
        appendUnique(report.hints, "CalculiX diagnostic hint: verify material elastic constants and units.");
    }
    if (containsAny(lower, {"dload", "cload", "boundary", "node set", "element set", "surface"})) {
        appendUnique(report.hints, "CalculiX diagnostic hint: verify boundary/load face groups are mapped to mesh surfaces.");
    }
    if (containsAny(lower, {"*warning", "warning"})) {
        appendUnique(report.warnings, "CalculiX diagnostic: solver log contains warning markers.");
        if (!firstWarningBlock.isEmpty()) {
            appendUnique(report.warnings, "CalculiX diagnostic detail: first warning block: " + firstWarningBlock);
        }
    }
    if (!lower.contains("job finished") && !lower.contains("summary of job information")) {
        appendUnique(report.warnings, "CalculiX diagnostic: completion marker was not found in .sta/.dat/.log.");
    }

    const QStringList missing = missingFiles(paths);
    if (!missing.isEmpty()) {
        appendUnique(report.warnings, "CalculiX diagnostic: missing expected files: " + missing.join("; "));
    }

    if (!QFileInfo::exists(paths.inputFile)) {
        appendUnique(report.errors, "CalculiX diagnostic: exported input deck is missing: " + paths.inputFile);
    }

    return report;
}
