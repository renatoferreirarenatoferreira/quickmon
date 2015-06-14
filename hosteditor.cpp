#include "hosteditor.h"
#include "ui_hosteditor.h"

#include <QDebug>

HostEditor::HostEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HostEditor)
{
    ui->setupUi(this);

    //define window parameters
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    //prepare ui automation
    connect(ui->radioButton_SNMPVersion_None,SIGNAL(clicked()),this,SLOT(updateUI()));
    connect(ui->radioButton_SNMPVersion_1,SIGNAL(clicked()),this,SLOT(updateUI()));
    connect(ui->radioButton_SNMPVersion_2c,SIGNAL(clicked()),this,SLOT(updateUI()));
    connect(ui->radioButton_SNMPVersion_3,SIGNAL(clicked()),this,SLOT(updateUI()));
    connect(ui->comboBox_SecurityLevel,SIGNAL(currentIndexChanged(int)),this,SLOT(updateUI()));
}

HostEditor::~HostEditor()
{
    delete ui;
}

void HostEditor::addHost()
{
    //clean up fields
    QString value;
    ui->lineEdit_Name->setText(value);
    ui->lineEdit_Address->setText(value);
    ui->radioButton_SNMPVersion_None->setChecked(true);
    ui->lineEdit_Community->setText(value);
    ui->comboBox_SecurityLevel->setCurrentIndex(0);
    ui->comboBox_Authentication->setCurrentIndex(0);
    ui->lineEdit_Authentication->setText(value);
    ui->comboBox_Privacy->setCurrentIndex(0);
    ui->lineEdit_Privacy->setText(value);

    addingMode = true;
    setWindowTitle("Add new host");
    updateUI();
    show();
}

void HostEditor::editHost(int hostID)
{
    //get data from database
    QSqlQuery query = LocalData::Instance()->createQuery("SELECT * FROM hosts WHERE ID=:ID");
    query.bindValue(":ID", hostID);
    LocalData::Instance()->executeQuery(query);

    //fill fields with data from database
    if (query.next())
    {
        ui->lineEdit_Name->setText(query.value("name").toString());
        ui->lineEdit_Address->setText(query.value("address").toString());

        int snmpVersion = query.value("snmpversion").toInt();
        if (snmpVersion == 1)
            ui->radioButton_SNMPVersion_1->setChecked(true);
        else if (snmpVersion == 2)
            ui->radioButton_SNMPVersion_2c->setChecked(true);
        else if (snmpVersion == 3)
            ui->radioButton_SNMPVersion_3->setChecked(true);
        else
            ui->radioButton_SNMPVersion_None->setChecked(true);

        ui->lineEdit_Community->setText(query.value("snmpcommunity").toString());

        QString value = query.value("snmpv3seclevel").toString();
        if (value == "authNoPriv")
            ui->comboBox_SecurityLevel->setCurrentIndex(1);
        else if (value == "authPriv")
            ui->comboBox_SecurityLevel->setCurrentIndex(2);
        else
            ui->comboBox_SecurityLevel->setCurrentIndex(0);

        value = query.value("snmpv3authprotocol").toString();
        if (value == "SHA")
            ui->comboBox_Authentication->setCurrentIndex(1);
        else
            ui->comboBox_Authentication->setCurrentIndex(0);

        ui->lineEdit_Authentication->setText(query.value("snmpv3authpassphrase").toString());

        value = query.value("snmpv3privprotocol").toString();
        if (value == "DES")
            ui->comboBox_Privacy->setCurrentIndex(1);
        else
            ui->comboBox_Privacy->setCurrentIndex(0);

        ui->lineEdit_Privacy->setText(query.value("snmpv3privpassphrase").toString());
    }

    addingMode = false;
    editingID = hostID;
    setWindowTitle("Edit host");
    updateUI();
    show();
}

void HostEditor::updateUI()
{
    if (ui->radioButton_SNMPVersion_None->isChecked())
    {
        ui->lineEdit_Community->setEnabled(false);
        ui->comboBox_SecurityLevel->setEnabled(false);
        ui->comboBox_Authentication->setEnabled(false);
        ui->lineEdit_Authentication->setEnabled(false);
        ui->comboBox_Privacy->setEnabled(false);
        ui->lineEdit_Privacy->setEnabled(false);
    } else if (ui->radioButton_SNMPVersion_1->isChecked() || ui->radioButton_SNMPVersion_2c->isChecked()) {
        ui->lineEdit_Community->setEnabled(true);
        ui->comboBox_SecurityLevel->setEnabled(false);
        ui->comboBox_Authentication->setEnabled(false);
        ui->lineEdit_Authentication->setEnabled(false);
        ui->comboBox_Privacy->setEnabled(false);
        ui->lineEdit_Privacy->setEnabled(false);
    } else {
        ui->lineEdit_Community->setEnabled(false);
        ui->comboBox_SecurityLevel->setEnabled(true);

        if (ui->comboBox_SecurityLevel->currentIndex() == 0)
        {
            ui->comboBox_Authentication->setEnabled(false);
            ui->lineEdit_Authentication->setEnabled(false);
            ui->comboBox_Privacy->setEnabled(false);
            ui->lineEdit_Privacy->setEnabled(false);
        } else if (ui->comboBox_SecurityLevel->currentIndex() == 1) {
            ui->comboBox_Authentication->setEnabled(true);
            ui->lineEdit_Authentication->setEnabled(true);
            ui->comboBox_Privacy->setEnabled(false);
            ui->lineEdit_Privacy->setEnabled(false);
        } else {
            ui->comboBox_Authentication->setEnabled(true);
            ui->lineEdit_Authentication->setEnabled(true);
            ui->comboBox_Privacy->setEnabled(true);
            ui->lineEdit_Privacy->setEnabled(true);
        }
    }
}

void HostEditor::on_buttonBox_accepted()
{
    //validate some things before applying
    if (ui->lineEdit_Name->text().size() <= 0 ||
        ui->lineEdit_Address->text().size() <= 0)
        QMessageBox::critical(this, "Error", "A host needs at least a name and an address!");
    else
    {
        if (addingMode)
        {
            QSqlQuery query = LocalData::Instance()->createQuery("INSERT INTO hosts ("
                                                                    "name, "
                                                                    "address, "
                                                                    "snmpversion, "
                                                                    "snmpcommunity, "
                                                                    "snmpv3seclevel, "
                                                                    "snmpv3authprotocol, "
                                                                    "snmpv3authpassphrase, "
                                                                    "snmpv3privprotocol, "
                                                                    "snmpv3privpassphrase"
                                                                 ") VALUES ("
                                                                    ":name, "
                                                                    ":address, "
                                                                    ":snmpversion, "
                                                                    ":snmpcommunity, "
                                                                    ":snmpv3seclevel, "
                                                                    ":snmpv3authprotocol, "
                                                                    ":snmpv3authpassphrase, "
                                                                    ":snmpv3privprotocol, "
                                                                    ":snmpv3privpassphrase"
                                                                 ")");

            query.bindValue(":name", ui->lineEdit_Name->text());
            query.bindValue(":address", ui->lineEdit_Address->text());
            if (ui->radioButton_SNMPVersion_None->isChecked())
                query.bindValue(":snmpversion", 0);
            else if (ui->radioButton_SNMPVersion_1->isChecked())
                query.bindValue(":snmpversion", 1);
            else if (ui->radioButton_SNMPVersion_2c->isChecked())
                query.bindValue(":snmpversion", 2);
            else
                query.bindValue(":snmpversion", 3);
            query.bindValue(":snmpcommunity", ui->lineEdit_Community->text());
            query.bindValue(":snmpv3seclevel", ui->comboBox_SecurityLevel->currentText());
            query.bindValue(":snmpv3authprotocol", ui->comboBox_Authentication->currentText());
            query.bindValue(":snmpv3authpassphrase", ui->lineEdit_Authentication->text());
            query.bindValue(":snmpv3privprotocol", ui->comboBox_Privacy->currentText());
            query.bindValue(":snmpv3privpassphrase", ui->lineEdit_Privacy->text());

            LocalData::Instance()->executeQuery(query);
        }  else {
            QSqlQuery query = LocalData::Instance()->createQuery("UPDATE hosts set "
                                                                    "name=:name, "
                                                                    "address=:address, "
                                                                    "snmpversion=:snmpversion, "
                                                                    "snmpcommunity=:snmpcommunity, "
                                                                    "snmpv3seclevel=:snmpv3seclevel, "
                                                                    "snmpv3authprotocol=:snmpv3authprotocol, "
                                                                    "snmpv3authpassphrase=:snmpv3authpassphrase, "
                                                                    "snmpv3privprotocol=:snmpv3privprotocol, "
                                                                    "snmpv3privpassphrase=:snmpv3privpassphrase "
                                                                 "WHERE id=:ID");

            query.bindValue(":name", ui->lineEdit_Name->text());
            query.bindValue(":address", ui->lineEdit_Address->text());
            if (ui->radioButton_SNMPVersion_None->isChecked())
                query.bindValue(":snmpversion", 0);
            else if (ui->radioButton_SNMPVersion_1->isChecked())
                query.bindValue(":snmpversion", 1);
            else if (ui->radioButton_SNMPVersion_2c->isChecked())
                query.bindValue(":snmpversion", 2);
            else
                query.bindValue(":snmpversion", 3);
            query.bindValue(":snmpcommunity", ui->lineEdit_Community->text());
            query.bindValue(":snmpv3seclevel", ui->comboBox_SecurityLevel->currentText());
            query.bindValue(":snmpv3authprotocol", ui->comboBox_Authentication->currentText());
            query.bindValue(":snmpv3authpassphrase", ui->lineEdit_Authentication->text());
            query.bindValue(":snmpv3privprotocol", ui->comboBox_Privacy->currentText());
            query.bindValue(":snmpv3privpassphrase", ui->lineEdit_Privacy->text());

            query.bindValue(":ID", editingID);
            LocalData::Instance()->executeQuery(query);

        }

        accept();
    }
}

void HostEditor::on_buttonBox_rejected()
{
    reject();
}
