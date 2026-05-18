#include "GmshRunner.h"

#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QStringList>

#include <utility>

GmshRunner::GmshRunner(QString gmshExecutablePath)
    : m_gmshExecutablePath(std::move(gmshExecutablePath))
{
}

QString GmshRunner::defaultGmshExecutablePath()
{
    return QStringLiteral("D:/cpp_lib/Gmsh/gmsh-4.15.2-Windows64/gmsh.exe");
}

QString GmshRunner::gmshExecutablePath() const
{
    return m_gmshExecutablePath;
}

GmshRunResult GmshRunner::checkVersion() const
{
    GmshRunResult result;

    if (!QFileInfo::exists(m_gmshExecutablePath)) {
        result.errorMessage = QStringLiteral("Gmsh 环境检查失败：找不到 gmsh.exe，请检查路径。");
        return result;
    }

    QProcess process;
    process.setProgram(m_gmshExecutablePath);
    process.setArguments(QStringList{QStringLiteral("--version")});
    process.start();

    if (!process.waitForStarted()) {
        result.errorMessage = QStringLiteral("Gmsh 环境检查失败：无法启动 gmsh.exe：") + process.errorString();
        return result;
    }

    if (!process.waitForFinished(10000)) {
        process.kill();
        process.waitForFinished();
        result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
        result.standardError = QString::fromLocal8Bit(process.readAllStandardError());
        result.errorMessage = QStringLiteral("Gmsh 环境检查失败：gmsh --version 执行超时。");
        return result;
    }

    result.exitCode = process.exitCode();
    result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    result.standardError = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();

    if (process.exitStatus() != QProcess::NormalExit) {
        result.errorMessage = QStringLiteral("Gmsh 环境检查失败：gmsh.exe 异常退出。");
        return result;
    }

    if (result.exitCode != 0) {
        result.errorMessage = QStringLiteral("Gmsh 环境检查失败：gmsh --version 返回非零退出码。");
        return result;
    }

    result.success = true;
    return result;
}

GmshRunResult GmshRunner::generate3DMesh(const QString &inputGeometryFile, const QString &outputMeshFile) const
{
    GmshRunResult result;

    if (!QFileInfo::exists(m_gmshExecutablePath)) {
        result.errorMessage = QStringLiteral("Gmsh 网格生成失败：找不到 gmsh.exe，请检查路径。");
        return result;
    }

    const QFileInfo inputInfo(inputGeometryFile);
    if (!inputInfo.isAbsolute()) {
        result.errorMessage = QStringLiteral("Gmsh 网格生成失败：输入几何文件必须是绝对路径。");
        return result;
    }
    if (!inputInfo.exists()) {
        result.errorMessage = QStringLiteral("Gmsh 网格生成失败：输入 STEP 文件不存在：") + inputGeometryFile;
        return result;
    }

    const QFileInfo outputInfo(outputMeshFile);
    if (!outputInfo.isAbsolute()) {
        result.errorMessage = QStringLiteral("Gmsh 网格生成失败：输出网格文件必须是绝对路径。");
        return result;
    }
    if (!QDir().mkpath(outputInfo.absolutePath())) {
        result.errorMessage = QStringLiteral("Gmsh 网格生成失败：无法创建输出目录：") + outputInfo.absolutePath();
        return result;
    }

    QProcess process;
    process.setProgram(m_gmshExecutablePath);
    process.setArguments(QStringList{
        inputGeometryFile,
        QStringLiteral("-3"),
        QStringLiteral("-format"),
        QStringLiteral("msh2"),
        QStringLiteral("-o"),
        outputMeshFile
    });
    process.start();

    if (!process.waitForStarted()) {
        result.errorMessage = QStringLiteral("Gmsh 网格生成失败：无法启动 gmsh.exe：") + process.errorString();
        return result;
    }

    if (!process.waitForFinished(60000)) {
        process.kill();
        process.waitForFinished();
        result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
        result.standardError = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        result.errorMessage = QStringLiteral("Gmsh 网格生成失败：gmsh 执行超时。");
        return result;
    }

    result.exitCode = process.exitCode();
    result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    result.standardError = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();

    if (process.exitStatus() != QProcess::NormalExit) {
        result.errorMessage = QStringLiteral("Gmsh 网格生成失败：gmsh.exe 异常退出。");
        return result;
    }

    if (result.exitCode != 0) {
        result.errorMessage = QStringLiteral("Gmsh 网格生成失败：gmsh 返回非零退出码。");
        return result;
    }

    if (!QFileInfo::exists(outputMeshFile)) {
        result.errorMessage = QStringLiteral("Gmsh 网格生成失败：命令成功返回，但没有生成输出文件。");
        return result;
    }

    result.success = true;
    return result;
}
