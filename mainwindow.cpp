#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //get geometry
    QDesktopWidget desktop;
    QRect primaryScreenGeometry = desktop.availableGeometry(desktop.primaryScreen());
    int frameHeightDifference = desktop.frameGeometry().height() - primaryScreenGeometry.height();
    //define window parameters
    this->setGeometry(0,0,250,primaryScreenGeometry.height() - frameHeightDifference);
    this->move(primaryScreenGeometry.topLeft());
    //window flags
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionAdd_New_Host_triggered()
{
    ui->treeView_Hosts->addHost();
}

void MainWindow::on_actionEdit_Selected_Host_triggered()
{
    ui->treeView_Hosts->editHost();
}

void MainWindow::on_actionRemove_Host_triggered()
{
    ui->treeView_Hosts->removeHost();
}
