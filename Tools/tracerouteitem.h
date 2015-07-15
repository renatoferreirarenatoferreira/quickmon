#ifndef TRACEROUTEITEM_H
#define TRACEROUTEITEM_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

#define TRACEROUTE_TIMEOUT_ADDRESS "* * *"
#define JITTER_PERCENT_YELLOW 15
#define JITTER_PERCENT_RED 50
#define LOSS_PERCENT_YELLOW 1
#define LOSS_PERCENT_RED 30
#define STYLE_LABEL_RED_EVEN "background: rgb(224, 133, 102);"
#define STYLE_LABEL_RED_ODD "background: rgb(255, 194, 178);"
#define STYLE_LABEL_YELLOW_EVEN "background: rgb(184, 184, 0);"
#define STYLE_LABEL_YELLOW_ODD "background: rgb(255, 255, 0);"
#define STYLE_LABEL_NORMAL_EVEN "background: rgb(204, 204, 204);"
#define STYLE_LABEL_NORMAL_ODD "background: rgb(255, 255, 255);"

class TracerouteItem : public QWidget
{
    Q_OBJECT
public:
    explicit TracerouteItem(QWidget *parent = 0);
    ~TracerouteItem();
    void setTTL(int ttl);
    void setAddress(QString address);
    void setLastRTT(double rtt);
    void setAvgRTT(double rtt);
    void setJitter(double jitter, float percentJitter);
    void setPctLoss(float loss);

private slots:
    void _setTTL(int ttl);
    void _setAddress(QString address);
    void _setLastRTT(double rtt);
    void _setAvgRTT(double rtt);
    void _setJitter(double jitter, float percentJitter);
    void _setPctLoss(float pctLoss);

private:
    QHBoxLayout mainLayout;
    QLabel labelTTL;
    QLabel labelAddress;
    QLabel labelLastRTT;
    QLabel labelAvgRTT;
    QLabel labelAvgJitter;
    QLabel labelPctLoss;
    int TTL;
};

#endif // TRACEROUTEITEM_H
