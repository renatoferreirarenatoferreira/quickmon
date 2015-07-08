#ifndef TOOLSLISTWIDGET_H
#define TOOLSLISTWIDGET_H

#include <QListWidget>
#include <QMessageBox>

#include "Tools/pingwindow.h"
#include "Tools/traceroutewindow.h"

class ToolsListWidget : public QListWidget
{
    Q_OBJECT

public:
    ToolsListWidget(QWidget* parent);
    ~ToolsListWidget();
    void dragEnterEvent(QDragEnterEvent *event);

private slots:
    void toolsClicked(QListWidgetItem *);
};

#endif // TOOLSLISTWIDGET_H
