#pragma once

#include <QString>
#include <QStringList>

struct UiSampleScreenshotResult
{
    bool success = false;
    QString outputDirectory;
    QStringList screenshotFiles;
    QStringList messages;
};

class UiSampleScreenshotter
{
public:
    explicit UiSampleScreenshotter(QString outputDirectory = {});

    UiSampleScreenshotResult capture();

private:
    QString m_outputDirectory;
};
