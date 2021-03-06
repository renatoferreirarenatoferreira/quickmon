#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QCoreApplication>
#include <QMenu>
#include <QToolButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionAdd_New_Host_triggered();
    void on_actionEdit_Selected_Host_triggered();
    void on_actionRemove_Host_triggered();
    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    void closeEvent(QCloseEvent*);
};

#endif // MAINWINDOW_H
