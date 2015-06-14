#ifndef HOSTEDITOR_H
#define HOSTEDITOR_H

#include <QDialog>
#include <QMessageBox>

#include "localdata.h"

namespace Ui {
class HostEditor;
}

class HostEditor : public QDialog
{
    Q_OBJECT

public:
    explicit HostEditor(QWidget *parent = 0);
    ~HostEditor();
    void addHost();
    void editHost(int hostID);

private slots:
    void updateUI();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    bool addingMode;
    int editingID;
    Ui::HostEditor *ui;
};

#endif // HOSTEDITOR_H
