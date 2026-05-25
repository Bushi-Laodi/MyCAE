#include "ui/MainWindowToolBarBuilder.h"

#include "ui/MainWindowActions.h"

#include <QMainWindow>
#include <QToolBar>

void MainWindowToolBarBuilder::build(QMainWindow *window, const MainWindowActions &actions)
{
    auto *toolBar = window->addToolBar("Main Toolbar");
    toolBar->setMovable(false);
    toolBar->addAction(actions.newProject);
    toolBar->addAction(actions.openProject);
    toolBar->addSeparator();
    toolBar->addAction(actions.createBox);
    toolBar->addAction(actions.createCylinder);
    toolBar->addSeparator();
    toolBar->addAction(actions.pickFace);
    toolBar->addAction(actions.createFaceGroupFromPick);
    toolBar->addAction(actions.addPickedFacesToFaceGroup);
    toolBar->addAction(actions.removePickedFacesFromFaceGroup);
}
