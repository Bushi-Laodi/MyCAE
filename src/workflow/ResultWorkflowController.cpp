#include "workflow/ResultWorkflowController.h"

#include "project/ProjectModel.h"
#include "project/ResultRepository.h"
#include "result/ResultAnimationController.h"
#include "result/ResultCsvExporter.h"
#include "result/ResultDisplayController.h"
#include "result/ResultManager.h"
#include "result/ResultObject.h"
#include "result/ResultReportExporter.h"
#include "ui/AppSettings.h"
#include "ui/ProjectTreePanel.h"
#include "ui/PropertyPanel.h"
#include "ui/RenderView.h"
#include "ui/ResultPostprocessPanel.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QUrl>

#include <algorithm>

namespace
{
QString sanitizedFileStem(QString value)
{
    value = value.trimmed();
    value.replace(QRegularExpression("[\\\\/:*?\"<>|\\s]+"), "_");
    return value.isEmpty() ? QString("result") : value;
}

QString ensureFileSuffix(const QString &filePath, const QString &suffix)
{
    return QFileInfo(filePath).suffix().isEmpty() ? filePath + suffix : filePath;
}

QString reportScreenshotPath(const QString &reportPath)
{
    const QFileInfo reportInfo(reportPath);
    return reportInfo.dir().filePath(reportInfo.completeBaseName() + "_screenshot.png");
}
}

ResultWorkflowController::ResultWorkflowController(
    ProjectModel &projectModel,
    ProjectTreePanel *projectTreePanel,
    PropertyPanel *propertyPanel,
    ResultPostprocessPanel *resultPostprocessPanel,
    RenderView *renderView,
    AppSettings &appSettings,
    ResultAnimationController &animationController,
    QWidget *parentWidget
)
    : m_projectModel(projectModel)
    , m_projectTreePanel(projectTreePanel)
    , m_propertyPanel(propertyPanel)
    , m_resultPostprocessPanel(resultPostprocessPanel)
    , m_renderView(renderView)
    , m_appSettings(appSettings)
    , m_animationController(animationController)
    , m_parentWidget(parentWidget)
{
}

QStringList ResultWorkflowController::setSelectedField(const QString &fieldName)
{
    QStringList logMessages;
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {"Result field change skipped: no result is selected."};
    }

    resultObject->displayFieldName = fieldName;
    saveResultIndex(logMessages);
    logMessages.append(redisplaySelectedResult());
    return logMessages;
}

QStringList ResultWorkflowController::setSelectedDeformationScale(double scale)
{
    QStringList logMessages;
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {"Result deformation scale change skipped: no result is selected."};
    }

    resultObject->deformationScale = scale;
    saveResultIndex(logMessages);
    logMessages.append(redisplaySelectedResult());
    return logMessages;
}

QStringList ResultWorkflowController::setSelectedMeshEdges(bool enabled)
{
    QStringList logMessages;
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {"Mesh edge toggle skipped: no result is selected."};
    }

    resultObject->showMeshEdges = enabled;
    saveResultIndex(logMessages);
    logMessages.append(redisplaySelectedResult());
    return logMessages;
}

QStringList ResultWorkflowController::setSelectedUndeformedOverlay(bool enabled)
{
    QStringList logMessages;
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {"Undeformed overlay toggle skipped: no result is selected."};
    }

    resultObject->showUndeformedOverlay = enabled;
    saveResultIndex(logMessages);
    logMessages.append(redisplaySelectedResult());
    return logMessages;
}

QStringList ResultWorkflowController::playSelectedAnimation(double speed)
{
    const ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {"Play animation skipped: no result is selected."};
    }

    QStringList logMessages;
    const double peakScale = resultObject->deformationScale;
    if (peakScale <= 0.0) {
        logMessages.append("Play animation started with zero deformation scale; increase Deformation to see motion.");
    }
    m_animationController.start(peakScale, speed);
    logMessages.append(QString("Result animation started: peakScale=%1, speed=%2 Hz.")
        .arg(peakScale, 0, 'g', 6)
        .arg(speed, 0, 'g', 6));
    return logMessages;
}

QStringList ResultWorkflowController::stopSelectedAnimation()
{
    if (!m_animationController.isRunning()) {
        return {"Stop animation skipped: animation is not running."};
    }

    QStringList logMessages;
    m_animationController.stop();
    saveResultIndex(logMessages);
    logMessages.append(QString("Result animation stopped: scale=%1.")
        .arg(m_animationController.currentScale(), 0, 'g', 6));
    return logMessages;
}

QStringList ResultWorkflowController::applyAnimatedDeformationScale(double scale)
{
    QStringList logMessages;
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        m_animationController.stop();
        return logMessages;
    }

    resultObject->deformationScale = scale;
    const ResultDisplayResult displayResult =
        ResultDisplayController().displayResult(m_projectModel, *resultObject, m_renderView, false);
    if (!displayResult.success) {
        logMessages.append(displayResult.logMessages);
        m_animationController.stop();
        return logMessages;
    }

    if (m_resultPostprocessPanel) {
        m_resultPostprocessPanel->setResult(resultObject);
    }
    return logMessages;
}

QStringList ResultWorkflowController::exportSelectedCsv()
{
    const ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {"Export CSV skipped: no result is selected."};
    }
    if (!m_projectModel.hasProject()) {
        return {"Export CSV failed: no project is open."};
    }

    QStringList logMessages;
    const QString initialPath = QDir(m_projectModel.project().rootPath)
        .filePath(QString("solver/exports/%1_result.csv").arg(sanitizedFileStem(resultObject->name)));
    const QString selectedPath = QFileDialog::getSaveFileName(
        m_parentWidget,
        "Export Result CSV",
        initialPath,
        "CSV File (*.csv)"
    );
    if (selectedPath.isEmpty()) {
        return {"Export CSV canceled."};
    }

    const ResultCsvExportResult exportResult =
        ResultCsvExporter().exportResult(m_projectModel, *resultObject, ensureFileSuffix(selectedPath, ".csv"));
    logMessages.append(exportResult.warnings);
    if (!exportResult.success) {
        logMessages.append(exportResult.errorMessage);
        QMessageBox::warning(m_parentWidget, "Export CSV", exportResult.errorMessage);
        return logMessages;
    }

    logMessages.append("Node displacement CSV exported: " + exportResult.nodeDisplacementCsvPath);
    logMessages.append("Element Von Mises CSV exported: " + exportResult.elementStressCsvPath);
    return logMessages;
}

QStringList ResultWorkflowController::exportSelectedReport()
{
    const ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {"Export report skipped: no result is selected."};
    }
    if (!m_projectModel.hasProject()) {
        return {"Export report failed: no project is open."};
    }
    if (!m_renderView) {
        return {"Export report failed: render view is not available."};
    }

    QStringList logMessages;
    const QString initialPath = QDir(m_projectModel.project().rootPath)
        .filePath(QString("solver/reports/%1_report.md").arg(sanitizedFileStem(resultObject->name)));
    const QString selectedPath = QFileDialog::getSaveFileName(
        m_parentWidget,
        "Export Result Report",
        initialPath,
        "Markdown File (*.md)"
    );
    if (selectedPath.isEmpty()) {
        return {"Export report canceled."};
    }

    const QString reportPath = ensureFileSuffix(selectedPath, ".md");
    const QString screenshotPath = reportScreenshotPath(reportPath);
    if (!QDir().mkpath(QFileInfo(reportPath).absolutePath())) {
        const QString message = "Export report failed: cannot create " + QFileInfo(reportPath).absolutePath();
        QMessageBox::warning(m_parentWidget, "Export Report", message);
        return {message};
    }
    if (!m_renderView->saveScreenshot(screenshotPath)) {
        const QString message = "Export report failed: cannot save screenshot " + screenshotPath;
        QMessageBox::warning(m_parentWidget, "Export Report", message);
        return {message};
    }

    const ResultReportExportResult exportResult =
        ResultReportExporter().exportMarkdown(m_projectModel.project(), *resultObject, reportPath, screenshotPath);
    if (!exportResult.success) {
        QMessageBox::warning(m_parentWidget, "Export Report", exportResult.errorMessage);
        return {exportResult.errorMessage};
    }

    logMessages.append("Result report exported: " + exportResult.reportPath);
    logMessages.append("Result report screenshot exported: " + screenshotPath);
    return logMessages;
}

QStringList ResultWorkflowController::exportRenderScreenshot()
{
    if (!m_renderView) {
        return {"Export screenshot failed: render view is not available."};
    }

    QString initialPath;
    const QString recentExportDirectory = m_appSettings.recentExportDirectory();
    if (!recentExportDirectory.isEmpty()) {
        initialPath = QDir(recentExportDirectory).filePath("render_screenshot.png");
    } else if (m_projectModel.hasProject()) {
        initialPath = QDir(m_projectModel.project().rootPath).filePath("solver/render_screenshot.png");
    }
    const QString filePath = QFileDialog::getSaveFileName(
        m_parentWidget,
        "Export Render Screenshot",
        initialPath,
        "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;Bitmap Image (*.bmp)"
    );
    if (filePath.isEmpty()) {
        return {"Export screenshot canceled."};
    }

    if (!m_renderView->saveScreenshot(filePath)) {
        return {"Export screenshot failed: " + filePath};
    }
    m_appSettings.setRecentExportDirectory(QFileInfo(filePath).absolutePath());
    return {"Render screenshot exported: " + filePath};
}

QStringList ResultWorkflowController::openSelectedResultDirectory()
{
    const ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {"Open result directory skipped: no result is selected."};
    }
    if (resultObject->casePath.isEmpty()) {
        return {"Open result directory failed: result has no case path."};
    }
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(resultObject->casePath))) {
        return {"Open result directory failed: " + resultObject->casePath};
    }
    return {};
}

QStringList ResultWorkflowController::renameSelectedResult()
{
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {"Rename result skipped: no result is selected."};
    }

    bool accepted = false;
    const QString newName = QInputDialog::getText(
        m_parentWidget,
        "Rename Result",
        "Result name",
        QLineEdit::Normal,
        resultObject->name,
        &accepted
    ).trimmed();
    if (!accepted || newName.isEmpty()) {
        return {"Rename result canceled."};
    }

    QStringList logMessages;
    resultObject->name = newName;
    saveResultIndex(logMessages);
    if (m_projectTreePanel) {
        m_projectTreePanel->setResultItems(m_projectModel.resultRepository().results());
    }
    logMessages.append(redisplaySelectedResult());
    logMessages.append("Result renamed: " + newName);
    return logMessages;
}

QStringList ResultWorkflowController::deleteSelectedResultHistory()
{
    const ResultObject *selectedResult = m_projectModel.resultForSelection();
    if (!selectedResult) {
        return {"Delete result skipped: no result is selected."};
    }

    const QString resultId = selectedResult->id;
    const QMessageBox::StandardButton answer = QMessageBox::question(
        m_parentWidget,
        "Delete Result History",
        "Remove this result from the project result list? Solver files on disk will not be deleted."
    );
    if (answer != QMessageBox::Yes) {
        return {"Delete result canceled."};
    }

    QStringList logMessages;
    std::vector<ResultObject> &results = m_projectModel.resultRepository().results();
    results.erase(std::remove_if(results.begin(), results.end(), [&resultId](const ResultObject &result) {
        return result.id == resultId;
    }), results.end());
    m_projectModel.clearSelectionIfKind(SelectionKind::Result);
    saveResultIndex(logMessages);
    refreshResultPanels();
    logMessages.append("Result history deleted: " + resultId);
    return logMessages;
}

QStringList ResultWorkflowController::redisplaySelectedResult(bool resetCamera)
{
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {};
    }

    m_projectModel.setSelection(Selection::item(SelectionKind::Result, resultObject->id, resultObject->name));
    if (m_propertyPanel) {
        m_propertyPanel->showResult(*resultObject);
    }

    const ResultDisplayResult displayResult =
        ResultDisplayController().displayResult(m_projectModel, *resultObject, m_renderView, resetCamera);
    if (m_propertyPanel) {
        m_propertyPanel->showResult(*resultObject);
    }
    if (m_resultPostprocessPanel) {
        m_resultPostprocessPanel->setResult(resultObject);
    }
    return displayResult.logMessages;
}

void ResultWorkflowController::saveResultIndex(QStringList &logMessages)
{
    QString saveError;
    if (m_projectModel.hasProject()
            && !ResultManager().save(m_projectModel.project(), m_projectModel.resultRepository().results(), &saveError)) {
        logMessages.append("Save result index failed: " + saveError);
    }
}

void ResultWorkflowController::refreshResultPanels()
{
    if (m_projectTreePanel) {
        m_projectTreePanel->setResultItems(m_projectModel.resultRepository().results());
    }
    if (m_propertyPanel) {
        m_propertyPanel->showResultCategory(m_projectModel.resultRepository().results());
    }
    if (m_resultPostprocessPanel) {
        m_resultPostprocessPanel->setResult(m_projectModel.resultForSelection());
    }
}
