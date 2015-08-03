#ifndef SNMPLISTITEM_H
#define SNMPLISTITEM_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#define STYLE_LABEL_NORMAL_EVEN "background: rgb(204, 204, 204);"
#define STYLE_LABEL_NORMAL_ODD "background: rgb(255, 255, 255);"

class SNMPListItem : public QWidget
{
    Q_OBJECT

public:
    SNMPListItem(QWidget *parent = 0);
    ~SNMPListItem();
    void configure(int lineNumber, QString key);
    void setValue(QString value);

private slots:
    void _setValue(QString value);

private:
    QHBoxLayout mainLayout;
    QLabel labelKey;
    QLabel labelSeparator;
    QLabel labelValue;
};

#endif // SNMPLISTITEM_H
