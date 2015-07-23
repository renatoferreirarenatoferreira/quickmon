#ifndef SNMPLISTWINDOW_H
#define SNMPLISTWINDOW_H

#include <QWidget>
#include <QMap>

namespace Ui {
class SNMPListWindow;
}

class SNMPListWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SNMPListWindow(QWidget *parent = 0);
    ~SNMPListWindow();
    void configure(QString templateName, QMap<QString, QVariant> templateItems);

private:
    Ui::SNMPListWindow *ui;
    QString templateName;
    QMap<QString, QVariant> templateItems;
};

#endif // SNMPLISTWINDOW_H
