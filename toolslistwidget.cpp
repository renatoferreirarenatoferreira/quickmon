#include "toolslistwidget.h"

ToolsListWidget::ToolsListWidget(QWidget* parent = 0) : QListWidget(parent)
{
    connect(this, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(toolsClicked(QListWidgetItem *)));
}

ToolsListWidget::~ToolsListWidget()
{

}

void ToolsListWidget::dragEnterEvent(QDragEnterEvent*)
{

}

void ToolsListWidget::toolsClicked(QListWidgetItem * toolsItem)
{
    if (toolsItem->text() == "Ping")
    {
        PingWindow * newPingWindow = new PingWindow();
        newPingWindow->show();
    } else {
        QMessageBox::warning(this, "Error", "Tool not ready to use.");
    }
}
