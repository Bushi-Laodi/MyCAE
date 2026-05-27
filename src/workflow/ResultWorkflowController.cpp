#include "workflow/ResultWorkflowController.h"

#include "project/ProjectModel.h"
#include "project/ResultRepository.h"
#include "result/ResultAnimationController.h"
#include "result/ResultCsvExporter.h"
#include "result/ResultDisplayController.h"
#include "result/ResultManager.h"
#include "result/ResultObject.h"
#include "result/ResultProbe.h"
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
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

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
        return {zh(u8"未切换结果场：当前未选择结果。")};
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
        return {zh(u8"未修改变形比例：当前未选择结果。")};
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
        return {zh(u8"未切换网格边显示：当前未选择结果。")};
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
        return {zh(u8"未切换未变形轮廓：当前未选择结果。")};
    }

    resultObject->showUndeformedOverlay = enabled;
    saveResultIndex(logMessages);
    logMessages.append(redisplaySelectedResult());
    return logMessages;
}

QStringList ResultWorkflowController::setSelectedScalarRangeLock(bool locked)
{
    QStringList logMessages;
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {zh(u8"未切换色标范围锁定：当前未选择结果。")};
    }

    resultObject->scalarRangeLocked = locked;
    if (locked && resultObject->lockedScalarMin == resultObject->lockedScalarMax) {
        resultObject->lockedScalarMin = resultObject->scalarMin;
        resultObject->lockedScalarMax = resultObject->scalarMax;
    }
    saveResultIndex(logMessages);
    logMessages.append(redisplaySelectedResult(false));
    logMessages.append(locked ? zh(u8"结果色标范围已锁定。") : zh(u8"结果色标范围已设为自动。"));
    return logMessages;
}

QStringList ResultWorkflowController::setSelectedScalarRange(double minimum, double maximum)
{
    QStringList logMessages;
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {zh(u8"未修改色标范围：当前未选择结果。")};
    }

    resultObject->scalarRangeLocked = true;
    resultObject->lockedScalarMin = minimum;
    resultObject->lockedScalarMax = maximum;
    saveResultIndex(logMessages);
    logMessages.append(redisplaySelectedResult(false));
    logMessages.append(zh(u8"结果色标范围已锁定：[%1, %2]。")
        .arg(minimum, 0, 'g', 6)
        .arg(maximum, 0, 'g', 6));
    return logMessages;
}

QStringList ResultWorkflowController::setSelectedProbe(const ResultProbe &probe)
{
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {zh(u8"未执行结果探针：当前未选择结果。")};
    }

    if (m_resultPostprocessPanel) {
        m_resultPostprocessPanel->setProbe(probe);
    }
    if (!probe.valid) {
        return {zh(u8"结果探针已清空。")};
    }
    return {zh(u8"结果探针：节点=%1，单元=%2，U=(%3,%4,%5)，Von Mises=%6。")
        .arg(probe.nodeId)
        .arg(probe.elementId)
        .arg(probe.ux, 0, 'g', 6)
        .arg(probe.uy, 0, 'g', 6)
        .arg(probe.uz, 0, 'g', 6)
        .arg(probe.vonMisesStress, 0, 'g', 6)};
}

QStringList ResultWorkflowController::playSelectedAnimation(double speed)
{
    const ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {zh(u8"未播放动画：当前未选择结果。")};
    }

    QStringList logMessages;
    const double peakScale = resultObject->deformationScale;
    if (peakScale <= 0.0) {
        logMessages.append(zh(u8"动画已启动，但当前变形比例为 0；请增大变形比例以看到运动。"));
    }
    m_animationController.start(peakScale, speed);
    logMessages.append(zh(u8"结果动画已启动：峰值比例=%1，速度=%2 Hz。")
        .arg(peakScale, 0, 'g', 6)
        .arg(speed, 0, 'g', 6));
    return logMessages;
}

QStringList ResultWorkflowController::stopSelectedAnimation()
{
    if (!m_animationController.isRunning()) {
        return {zh(u8"未停止动画：动画当前未运行。")};
    }

    QStringList logMessages;
    m_animationController.stop();
    saveResultIndex(logMessages);
    logMessages.append(zh(u8"结果动画已停止：比例=%1。")
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
        return {zh(u8"未导出 CSV：当前未选择结果。")};
    }
    if (!m_projectModel.hasProject()) {
        return {zh(u8"导出 CSV 失败：当前未打开工程。")};
    }

    QStringList logMessages;
    const QString initialPath = QDir(m_projectModel.project().rootPath)
        .filePath(QString("solver/exports/%1_result.csv").arg(sanitizedFileStem(resultObject->name)));
    const QString selectedPath = QFileDialog::getSaveFileName(
        m_parentWidget,
        zh(u8"导出结果 CSV"),
        initialPath,
        zh(u8"CSV 文件 (*.csv)")
    );
    if (selectedPath.isEmpty()) {
        return {zh(u8"已取消导出 CSV。")};
    }

    const ResultCsvExportResult exportResult =
        ResultCsvExporter().exportResult(m_projectModel, *resultObject, ensureFileSuffix(selectedPath, ".csv"));
    logMessages.append(exportResult.warnings);
    if (!exportResult.success) {
        logMessages.append(exportResult.errorMessage);
        QMessageBox::warning(m_parentWidget, zh(u8"导出 CSV"), exportResult.errorMessage);
        return logMessages;
    }

    logMessages.append(zh(u8"节点位移 CSV 已导出：") + exportResult.nodeDisplacementCsvPath);
    logMessages.append(zh(u8"单元 Von Mises CSV 已导出：") + exportResult.elementStressCsvPath);
    return logMessages;
}

QStringList ResultWorkflowController::exportSelectedReport()
{
    const ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {zh(u8"未导出报告：当前未选择结果。")};
    }
    if (!m_projectModel.hasProject()) {
        return {zh(u8"导出报告失败：当前未打开工程。")};
    }
    if (!m_renderView) {
        return {zh(u8"导出报告失败：渲染视图不可用。")};
    }

    QStringList logMessages;
    const QString initialPath = QDir(m_projectModel.project().rootPath)
        .filePath(QString("solver/reports/%1_report.md").arg(sanitizedFileStem(resultObject->name)));
    const QString selectedPath = QFileDialog::getSaveFileName(
        m_parentWidget,
        zh(u8"导出结果报告"),
        initialPath,
        zh(u8"Markdown 文件 (*.md)")
    );
    if (selectedPath.isEmpty()) {
        return {zh(u8"已取消导出报告。")};
    }

    const QString reportPath = ensureFileSuffix(selectedPath, ".md");
    const QString screenshotPath = reportScreenshotPath(reportPath);
    if (!QDir().mkpath(QFileInfo(reportPath).absolutePath())) {
        const QString message = zh(u8"导出报告失败：无法创建目录 ") + QFileInfo(reportPath).absolutePath();
        QMessageBox::warning(m_parentWidget, zh(u8"导出报告"), message);
        return {message};
    }
    if (!m_renderView->saveScreenshot(screenshotPath)) {
        const QString message = zh(u8"导出报告失败：无法保存截图 ") + screenshotPath;
        QMessageBox::warning(m_parentWidget, zh(u8"导出报告"), message);
        return {message};
    }

    const ResultReportExportResult exportResult =
        ResultReportExporter().exportMarkdown(m_projectModel.project(), *resultObject, reportPath, screenshotPath);
    if (!exportResult.success) {
        QMessageBox::warning(m_parentWidget, zh(u8"导出报告"), exportResult.errorMessage);
        return {exportResult.errorMessage};
    }

    logMessages.append(zh(u8"结果报告已导出：") + exportResult.reportPath);
    logMessages.append(zh(u8"结果报告截图已导出：") + screenshotPath);
    return logMessages;
}

QStringList ResultWorkflowController::exportRenderScreenshot()
{
    if (!m_renderView) {
        return {zh(u8"导出截图失败：渲染视图不可用。")};
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
        zh(u8"导出渲染截图"),
        initialPath,
        zh(u8"PNG 图像 (*.png);;JPEG 图像 (*.jpg *.jpeg);;位图图像 (*.bmp)")
    );
    if (filePath.isEmpty()) {
        return {zh(u8"已取消导出截图。")};
    }

    if (!m_renderView->saveScreenshot(filePath)) {
        return {zh(u8"导出截图失败：") + filePath};
    }
    m_appSettings.setRecentExportDirectory(QFileInfo(filePath).absolutePath());
    return {zh(u8"渲染截图已导出：") + filePath};
}

QStringList ResultWorkflowController::openSelectedResultDirectory()
{
    const ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {zh(u8"未打开结果目录：当前未选择结果。")};
    }
    if (resultObject->casePath.isEmpty()) {
        return {zh(u8"打开结果目录失败：结果没有算例路径。")};
    }
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(resultObject->casePath))) {
        return {zh(u8"打开结果目录失败：") + resultObject->casePath};
    }
    return {};
}

QStringList ResultWorkflowController::renameSelectedResult()
{
    ResultObject *resultObject = m_projectModel.resultForSelection();
    if (!resultObject) {
        return {zh(u8"未重命名结果：当前未选择结果。")};
    }

    bool accepted = false;
    const QString newName = QInputDialog::getText(
        m_parentWidget,
        zh(u8"重命名结果"),
        zh(u8"结果名称"),
        QLineEdit::Normal,
        resultObject->name,
        &accepted
    ).trimmed();
    if (!accepted || newName.isEmpty()) {
        return {zh(u8"已取消重命名结果。")};
    }

    QStringList logMessages;
    resultObject->name = newName;
    saveResultIndex(logMessages);
    if (m_projectTreePanel) {
        m_projectTreePanel->setResultItems(m_projectModel.resultRepository().results());
    }
    logMessages.append(redisplaySelectedResult());
    logMessages.append(zh(u8"结果已重命名：") + newName);
    return logMessages;
}

QStringList ResultWorkflowController::deleteSelectedResultHistory()
{
    const ResultObject *selectedResult = m_projectModel.resultForSelection();
    if (!selectedResult) {
        return {zh(u8"未删除结果：当前未选择结果。")};
    }

    const QString resultId = selectedResult->id;
    const QMessageBox::StandardButton answer = QMessageBox::question(
        m_parentWidget,
        zh(u8"删除结果历史"),
        zh(u8"从工程结果列表中移除此结果？磁盘上的求解文件不会被删除。")
    );
    if (answer != QMessageBox::Yes) {
        return {zh(u8"已取消删除结果。")};
    }

    QStringList logMessages;
    std::vector<ResultObject> &results = m_projectModel.resultRepository().results();
    results.erase(std::remove_if(results.begin(), results.end(), [&resultId](const ResultObject &result) {
        return result.id == resultId;
    }), results.end());
    m_projectModel.clearSelectionIfKind(SelectionKind::Result);
    saveResultIndex(logMessages);
    refreshResultPanels();
    logMessages.append(zh(u8"结果历史已删除：") + resultId);
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
        logMessages.append(zh(u8"保存结果索引失败：") + saveError);
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
