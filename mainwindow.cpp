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

    //toolbar spacer for right alignet items
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //add spacer to toolbar
    ui->toolBar->addWidget(spacer);
    //help menu
    QMenu* helpMenu = new QMenu(this);
    helpMenu->addAction(ui->actionAbout);
    //help icon
    QIcon helpIcon;
    helpIcon.addFile(QStringLiteral(":/Icons/Help"), QSize(), QIcon::Normal, QIcon::Off);
    //help button
    QToolButton* helpButton = new QToolButton();
    helpButton->setMenu(helpMenu);
    helpButton->setPopupMode(QToolButton::InstantPopup);
    helpButton->setStyleSheet("QToolButton::menu-indicator{ image: url(none.jpg); }");
    helpButton->setIcon(helpIcon);
    helpButton->setToolTip("Help");
    //help menu to toolbar
    ui->toolBar->addWidget(helpButton);
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

void MainWindow::closeEvent(QCloseEvent*)
{
    this->destroy(true, true);
    QCoreApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QString aboutText = QString("<b>").append(APPLICATION_NAME).append(" ").append(APPLICATION_VERSION).append("</b><br>\n<br>\n");
    aboutText.append("Access <a href='https://github.com/renatoferreirarenatoferreira/quickmon'>").append(APPLICATION_NAME).append(" Project on GitHub</a>!");
    QMessageBox::about(this, QString("About ").append(APPLICATION_NAME), aboutText);
}
