#ifndef SNMPGRAPHLEGENDITEM_H
#define SNMPGRAPHLEGENDITEM_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#define STYLE_NORMAL_EVEN "background: rgb(204, 204, 204);"
#define STYLE_NORMAL_ODD "background: rgb(255, 255, 255);"

#define STYLE_LEGEND_0 "background: rgb(39, 113, 164);"
#define STYLE_LEGEND_1 "background: rgb(255, 127, 14);"
#define STYLE_LEGEND_2 "background: rgb(44, 160, 44);"
#define STYLE_LEGEND_3 "background: rgb(214, 39, 40);"
#define STYLE_LEGEND_4 "background: rgb(148, 103, 189);"
#define STYLE_LEGEND_5 "background: rgb(140, 86, 75);"
#define STYLE_LEGEND_6 "background: rgb(227, 119, 194);"
#define STYLE_LEGEND_7 "background: rgb(0, 0, 61);"
#define STYLE_LEGEND_8 "background: rgb(148, 148, 148);"

class SNMPGraphLegendItem : public QWidget
{
    Q_OBJECT

public:
    explicit SNMPGraphLegendItem(QWidget *parent = 0);
    ~SNMPGraphLegendItem();
    void configure(int lineNumber, QString key, QString unit);
    QColor getColor();
    void updateLegend(double lastValue, double averageValue, double maxValue, double minValue);
    void reset();

signals:

public slots:

private:
    QHBoxLayout mainLayout;
    QHBoxLayout* legendColorLayout;
    QWidget legendColorWidget;
    QFrame legendColorFrame;
    QLabel legendLabel;
    QLabel lastLabel;
    QLabel minLabel;
    QLabel avgLabel;
    QLabel maxLabel;
    QColor legendColor;
    QString unit;
    bool formatAsBytes;
    QString formatValue(double value);
};

#endif // SNMPGRAPHLEGENDITEM_H
