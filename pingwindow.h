#ifndef PINGWINDOW_H
#define PINGWINDOW_H

#include <QWidget>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QHostInfo>

#include "hoststreeview.h"
#include "pinger.h"

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

private:
    Ui::PingWindow* ui;
    Pinger* pingerInstance;
    QTimer* asyncWorker;
    QHostAddress pingingAddress;

};

#endif // PINGWINDOW_H
