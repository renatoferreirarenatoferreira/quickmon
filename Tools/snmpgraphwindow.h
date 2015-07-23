#ifndef SNMPGRAPHWINDOW_H
#define SNMPGRAPHWINDOW_H

#include <QWidget>
#include <QMap>

namespace Ui {
class SNMPGraphWindow;
}

class SNMPGraphWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SNMPGraphWindow(QWidget *parent = 0);
    ~SNMPGraphWindow();
    void configure(QString templateName, QMap<QString, QVariant> templateItems);

private:
    Ui::SNMPGraphWindow *ui;
    QString templateName;
    QMap<QString, QVariant> templateItems;
};

#endif // SNMPGRAPHWINDOW_H
