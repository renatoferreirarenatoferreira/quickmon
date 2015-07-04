#ifndef PINGWINDOW_H
#define PINGWINDOW_H

#include <QWidget>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QHostInfo>

#include "../hoststreeview.h"
#include "../Net/pinger.h"
#include "pingstatistics.h"

#define JITTER_PERCENT_YELLOW 15
#define JITTER_PERCENT_RED 50
#define LOSS_PERCENT_YELLOW 1
#define LOSS_PERCENT_RED 30
#define STYLE_FIELD_RED "font: 12px;\ncolor: rgb(0, 0, 0);\nbackground-color: rgb(255, 166, 166);"
#define STYLE_FIELD_YELLOW "font: 12px;\ncolor: rgb(0, 0, 0);\nbackground-color: rgb(255, 255, 178);"
#define STYLE_FIELD_NORMAL "font: 12px;\ncolor: rgb(0, 0, 0);"
#define STYLE_LABEL_RED "font: 10px;\ncolor: rgb(102, 102, 102);\nbackground-color: rgb(255, 166, 166);"
#define STYLE_LABEL_YELLOW "font: 10px;\ncolor: rgb(102, 102, 102);\nbackground-color: rgb(255, 255, 178);"
#define STYLE_LABEL_NORMAL "font: 10px;\ncolor: rgb(102, 102, 102);"

namespace Ui {
class PingWindow;
}

class PingWindow : public QWidget, public IPingReplyListener
{
    Q_OBJECT

public:
    explicit PingWindow(QWidget* parent = 0);
    ~PingWindow();
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void receivePingReply(PingContext* context);

private slots:
    void run(int hostID);
    void lookupHostReply(QHostInfo);
    void asyncTask();
    void updateUI(int replyStatus, double latency);

private:
    Ui::PingWindow* ui;
    Pinger* pingerInstance;
    QTimer* asyncWorker;
    PingContext* pigingContext;
    QHostAddress pingingAddress;
    PingStatistics latencyCalculator;
    bool waitingForReply;
    bool retryImmediatelly;
    void resetValues();
    void updateGraph(double lastLatency);
};

#endif // PINGWINDOW_H
