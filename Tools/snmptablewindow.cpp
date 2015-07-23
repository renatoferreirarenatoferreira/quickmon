#include "snmptablewindow.h"
#include "ui_snmptablewindow.h"

SNMPTableWindow::SNMPTableWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SNMPTableWindow)
{
    ui->setupUi(this);
}

SNMPTableWindow::~SNMPTableWindow()
{
    delete ui;
}

void SNMPTableWindow::configure(QString templateName, QMap<QString, QVariant> templateItems)
{
    this->setWindowTitle("SNMP Table: " + templateName);
    ui->labelTemplate->setText(templateName);

    this->templateName = templateName;
    this->templateItems = templateItems;
}
