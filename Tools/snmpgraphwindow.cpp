#include "snmpgraphwindow.h"
#include "ui_snmpgraphwindow.h"

SNMPGraphWindow::SNMPGraphWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SNMPGraphWindow)
{
    ui->setupUi(this);
}

SNMPGraphWindow::~SNMPGraphWindow()
{
    delete ui;
}

void SNMPGraphWindow::configure(QString templateName, QMap<QString, QVariant> templateItems)
{
    this->setWindowTitle("SNMP Graph: " + templateName);
    ui->labelTemplate->setText(templateName);

    this->templateName = templateName;
    this->templateItems = templateItems;
}
