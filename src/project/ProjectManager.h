#pragma once

#include "Project.h"

#include <QString>

class ProjectManager
{
public:
    bool createProject(const QString &projectPath, Project *project, QString *errorMessage) const;
    bool openProject(const QString &projectFilePath, Project *project, QString *errorMessage) const;

private:
    bool ensureProjectDirectories(const QString &projectPath, QString *errorMessage) const;
    bool writeProjectFile(const Project &project, QString *errorMessage) const;
};
