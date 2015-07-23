#ifndef SNMPTABLEWINDOW_H
#define SNMPTABLEWINDOW_H

#include <QWidget>
#include <QMap>

namespace Ui {
class SNMPTableWindow;
}

class SNMPTableWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SNMPTableWindow(QWidget *parent = 0);
    ~SNMPTableWindow();
    void configure(QString templateName, QMap<QString, QVariant> templateItems);

private:
    Ui::SNMPTableWindow *ui;
    QString templateName;
    QMap<QString, QVariant> templateItems;
};

#endif // SNMPTABLEWINDOW_H
