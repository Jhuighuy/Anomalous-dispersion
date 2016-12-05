#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QMainWindow>

namespace Ui {
class MenuWindow;
}

class MenuWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MenuWindow(QWidget* parent = nullptr): QMainWindow(parent) {}
    ~MenuWindow();

    void setupUi(QMainWindow* sceneWindow, QMainWindow* authorsWindows);

private:
    Ui::MenuWindow* ui;
};

#endif // MENUWINDOW_H
