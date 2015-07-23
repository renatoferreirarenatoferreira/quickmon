#include "snmplistwindow.h"
#include "ui_snmplistwindow.h"

SNMPListWindow::SNMPListWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SNMPListWindow)
{
    ui->setupUi(this);
}

SNMPListWindow::~SNMPListWindow()
{
    delete ui;
}

void SNMPListWindow::configure(QString templateName, QMap<QString, QVariant> templateItems)
{
    this->setWindowTitle("SNMP List: " + templateName);
    ui->labelTemplate->setText(templateName);

    this->templateName = templateName;
    this->templateItems = templateItems;
}
