#pragma once

class QMainWindow;
struct MainWindowActions;

class MainWindowToolBarBuilder
{
public:
    static void build(QMainWindow *window, const MainWindowActions &actions);
};
