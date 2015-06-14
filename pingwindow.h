#ifndef PINGWINDOW_H
#define PINGWINDOW_H

#include <QWidget>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QHostInfo>

#include "hoststreeview.h"

namespace Ui {
class PingWindow;
}

class PingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PingWindow(QWidget* parent = 0);
    ~PingWindow();
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

private slots:
    void run(int hostID);

private:
    Ui::PingWindow* ui;
    QHostInfo hostInfo;
};

#endif // PINGWINDOW_H
