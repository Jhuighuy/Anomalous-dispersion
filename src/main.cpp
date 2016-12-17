#include <QApplication>
#include "forms/MenuWindow.h"
#include "forms/AuthorsWindow.h"
#include "forms/SceneWindow.h"

int main(int argc, char** argv)
{
    QApplication application(argc, argv);
    MenuWindow menuWindow;
    AuthorsWindow authorsWindow;
    SceneWindow sceneWindow;

    menuWindow.setupUi(&sceneWindow, &authorsWindow);
    authorsWindow.setupUi(&menuWindow);
    sceneWindow.setupUi(&menuWindow);
#ifndef __APPLE__
    menuWindow.showFullScreen();
#else
    menuWindow.show();
#endif
    return application.exec();
}
