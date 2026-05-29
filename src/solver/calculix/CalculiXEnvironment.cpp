#include "solver/calculix/CalculiXEnvironment.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

QString CalculiXEnvironment::executablePath()
{
    const QString fromEnvironment =
        QProcessEnvironment::systemEnvironment().value("MYCAE_CALCULIX_EXECUTABLE").trimmed();
    if (!fromEnvironment.isEmpty()) {
        return fromEnvironment;
    }

#ifdef MYCAE_CALCULIX_EXECUTABLE_PATH
    const QString fromCMake = QString::fromUtf8(MYCAE_CALCULIX_EXECUTABLE_PATH).trimmed();
    if (!fromCMake.isEmpty()) {
        return fromCMake;
    }
#endif

    return "ccx";
}

bool CalculiXEnvironment::isExplicitExecutablePath(const QString &executablePath)
{
    const QFileInfo executableInfo(executablePath);
    return executableInfo.isAbsolute() || executablePath.contains('/') || executablePath.contains('\\');
}

bool CalculiXEnvironment::executableAvailable(QString *resolvedPath)
{
    const QString executable = executablePath();
    if (isExplicitExecutablePath(executable)) {
        const bool exists = QFileInfo::exists(executable);
        if (exists && resolvedPath) {
            *resolvedPath = QFileInfo(executable).absoluteFilePath();
        }
        return exists;
    }

    const QString resolved = QStandardPaths::findExecutable(executable);
    if (resolvedPath) {
        *resolvedPath = resolved;
    }
    return !resolved.isEmpty();
}

QProcessEnvironment CalculiXEnvironment::processEnvironment(const QString &executablePath)
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    if (!isExplicitExecutablePath(executablePath)) {
        return environment;
    }

    const QFileInfo executableInfo(executablePath);
    QDir envRootDir = executableInfo.absoluteDir();
    if (envRootDir.dirName().compare("bin", Qt::CaseInsensitive) == 0) {
        envRootDir.cdUp();
        if (envRootDir.dirName().compare("Library", Qt::CaseInsensitive) == 0) {
            envRootDir.cdUp();
        }
    }

    const QString envRoot = envRootDir.absolutePath();
    const QStringList condaRuntimePaths = {
        envRoot,
        envRoot + "/Library/mingw-w64/bin",
        envRoot + "/Library/usr/bin",
        envRoot + "/Library/bin",
        envRoot + "/Scripts",
        envRoot + "/bin",
    };

    QStringList pathEntries;
    for (const QString &path : condaRuntimePaths) {
        if (QDir(path).exists()) {
            pathEntries.append(QDir::toNativeSeparators(path));
        }
    }

    const QString existingPath = environment.value("PATH");
    if (!existingPath.isEmpty()) {
        pathEntries.append(existingPath);
    }
    environment.insert("PATH", pathEntries.join(';'));
    environment.insert("CONDA_PREFIX", QDir::toNativeSeparators(envRoot));
    return environment;
}
