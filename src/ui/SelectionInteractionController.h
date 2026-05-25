#pragma once

#include <QString>
#include <QStringList>

#include <functional>

class PickController;
class ProjectModel;
class ResultAnimationController;
struct MainWindowDockWidgets;
struct PickSelection;
struct Selection;

struct SelectionInteractionCallbacks
{
    std::function<void()> stopResultAnimation;
    std::function<void(const QStringList &)> writeLogMessages;
    std::function<void(const QString &)> showStatusMessage;
    std::function<void()> updateActionStates;
};

struct SelectionInteractionContext
{
    ProjectModel &projectModel;
    PickController &pickController;
    ResultAnimationController &resultAnimationController;
    MainWindowDockWidgets &docks;
};

class SelectionInteractionController
{
public:
    SelectionInteractionController(SelectionInteractionContext context, SelectionInteractionCallbacks callbacks);

    void applySelection(const Selection &selection) const;
    void handleFacePicked(const PickSelection &selection) const;

private:
    SelectionInteractionContext m_context;
    SelectionInteractionCallbacks m_callbacks;
};
