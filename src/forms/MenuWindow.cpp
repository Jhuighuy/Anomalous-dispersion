#include "MenuWindow.h"
#include "ui_MenuWindow.h"

MenuWindow::~MenuWindow()
{
    delete ui;
}

void MenuWindow::setupUi(QMainWindow* sceneWindow, QMainWindow* authorsWindows)
{
    ui = new Ui::MenuWindow();
    ui->setupUi(this);

    connect(ui->pushButtonStart, &QPushButton::clicked, this, &QMainWindow::hide);
#ifndef __APPLE__
    connect(ui->pushButtonStart, &QPushButton::clicked, sceneWindow, &QMainWindow::showFullScreen);
#else
    connect(ui->pushButtonStart, &QPushButton::clicked, sceneWindow, &QMainWindow::show);
#endif

    connect(ui->pushButtonAuthors, &QPushButton::clicked, this, &QMainWindow::hide);
    connect(ui->pushButtonAuthors, &QPushButton::clicked, authorsWindows, &QMainWindow::showFullScreen);

    connect(ui->pushButtonExit, &QPushButton::clicked, qApp, &QCoreApplication::quit);
}
