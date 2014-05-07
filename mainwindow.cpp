#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QShortcut>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()), 0, Qt::ApplicationShortcut);
}

MainWindow::~MainWindow()
{
    delete ui;
}
