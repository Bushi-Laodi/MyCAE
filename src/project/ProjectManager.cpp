#include "ProjectManager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

namespace
{
QString resolvedProjectRootPath(const QFileInfo &projectFileInfo, const QString &storedRootPath)
{
    const QDir projectFileDir = projectFileInfo.dir();
    if (storedRootPath.trimmed().isEmpty()) {
        return projectFileDir.absolutePath();
    }

    const QFileInfo storedRootInfo(storedRootPath);
    const QString candidateRootPath = storedRootInfo.isAbsolute()
        ? storedRootInfo.absoluteFilePath()
        : projectFileDir.absoluteFilePath(storedRootPath);
    const QFileInfo candidateProjectFile(QDir(candidateRootPath).filePath("project.json"));

    if (candidateProjectFile.exists()
        && candidateProjectFile.canonicalFilePath() == projectFileInfo.canonicalFilePath()) {
        return QDir::cleanPath(candidateRootPath);
    }

    return projectFileDir.absolutePath();
}
}

bool ProjectManager::createProject(const QString &projectPath, Project *project, QString *errorMessage) const
{
    if (!project) {
        if (errorMessage) {
            *errorMessage = "内部错误：工程输出对象为空。";
        }
        return false;
    }

    const QFileInfo projectInfo(projectPath);
    const QString absoluteProjectPath = projectInfo.absoluteFilePath();
    const QString projectName = projectInfo.fileName().isEmpty() ? "未命名工程" : projectInfo.fileName();
    const QString projectFilePath = QDir(absoluteProjectPath).filePath("project.json");

    if (QFileInfo::exists(projectFilePath)) {
        if (errorMessage) {
            *errorMessage = "该目录已存在 project.json。请打开该工程，或选择其他目录。";
        }
        return false;
    }

    if (!ensureProjectDirectories(absoluteProjectPath, errorMessage)) {
        return false;
    }

    Project createdProject;
    createdProject.name = projectName;
    createdProject.rootPath = absoluteProjectPath;
    createdProject.projectFilePath = projectFilePath;

    if (!writeProjectFile(createdProject, errorMessage)) {
        return false;
    }

    *project = createdProject;
    return true;
}

bool ProjectManager::openProject(const QString &projectFilePath, Project *project, QString *errorMessage) const
{
    if (!project) {
        if (errorMessage) {
            *errorMessage = "内部错误：工程输出对象为空。";
        }
        return false;
    }

    QFile file(projectFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "打开工程文件失败：" + file.errorString();
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "工程文件无效：根节点不是 JSON 对象。";
        }
        return false;
    }

    const QJsonObject object = document.object();
    const QFileInfo fileInfo(projectFilePath);

    Project openedProject;
    openedProject.name = object.value("name").toString(fileInfo.dir().dirName());
    openedProject.rootPath = resolvedProjectRootPath(fileInfo, object.value("rootPath").toString());
    openedProject.projectFilePath = fileInfo.absoluteFilePath();

    if (openedProject.name.isEmpty() || openedProject.rootPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "工程文件无效：缺少工程名称或工程根目录。";
        }
        return false;
    }

    *project = openedProject;
    return true;
}

bool ProjectManager::ensureProjectDirectories(const QString &projectPath, QString *errorMessage) const
{
    QDir projectDir(projectPath);
    if (!projectDir.exists() && !QDir().mkpath(projectPath)) {
        if (errorMessage) {
            *errorMessage = "创建工程目录失败：" + projectPath;
        }
        return false;
    }

    const QStringList subdirectories = {"geometry", "mesh", "solver", "logs"};
    for (const QString &subdirectory : subdirectories) {
        if (!projectDir.mkpath(subdirectory)) {
            if (errorMessage) {
                *errorMessage = "创建子目录失败：" + subdirectory;
            }
            return false;
        }
    }

    return true;
}

bool ProjectManager::writeProjectFile(const Project &project, QString *errorMessage) const
{
    QJsonObject object;
    object.insert("name", project.name);
    object.insert("rootPath", project.rootPath);
    object.insert("version", "0.1.0");
    object.insert("createdAt", QDateTime::currentDateTime().toString(Qt::ISODate));

    QFile file(project.projectFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "写入工程文件失败：" + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}
