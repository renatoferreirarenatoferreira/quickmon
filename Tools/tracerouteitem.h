#ifndef TRACEROUTEITEM_H
#define TRACEROUTEITEM_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

#define TRACEROUTE_TIMEOUT_ADDRESS "* * *"
#define TRACEROUTE_TIMEOUT_RTT -1

class TracerouteItem : public QWidget
{
    Q_OBJECT
public:
    explicit TracerouteItem(QWidget *parent = 0);
    ~TracerouteItem();
    void setTTL(int ttl);
    void setAddress(QString address);
    void setRTT(double rtt);

private slots:
    void _setTTL(int ttl);
    void _setAddress(QString address);
    void _setRTT(double rtt);

private:
    QHBoxLayout mainLayout;
    QLabel labelTTL;
    QLabel labelAddress;
    QLabel labelRTT;
    int TTL;
};

#endif // TRACEROUTEITEM_H
