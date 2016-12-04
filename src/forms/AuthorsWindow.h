#ifndef AUTHORSWINDOW_H
#define AUTHORSWINDOW_H

#include <QMainWindow>

namespace Ui {
class AuthorsWindow;
}

class AuthorsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AuthorsWindow(QWidget *parent = Q_NULLPTR): QMainWindow(parent) {}
    ~AuthorsWindow();

    void setupUi(QMainWindow* menuWindow);

private:
    Ui::AuthorsWindow *ui;
};

#endif // AUTHORSWINDOW_H
