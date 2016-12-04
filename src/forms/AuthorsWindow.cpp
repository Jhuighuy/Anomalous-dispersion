#include "AuthorsWindow.h"
#include "ui_AuthorsWindow.h"

AuthorsWindow::~AuthorsWindow()
{
    delete ui;
}

void AuthorsWindow::setupUi(QMainWindow* menuWindow)
{
    ui = new Ui::AuthorsWindow;
    ui->setupUi(this);

    connect(ui->pushButtonToMenu, &QPushButton::clicked, this, &QMainWindow::hide);
    connect(ui->pushButtonToMenu, &QPushButton::clicked, menuWindow, &QMainWindow::showFullScreen);
}

