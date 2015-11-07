#include "snmpgraphlegenditem.h"

SNMPGraphLegendItem::SNMPGraphLegendItem(QWidget *parent) : QWidget(parent)
{
    this->mainLayout.setMargin(0);
    this->mainLayout.setSpacing(0);

    this->legendColorFrame.setMaximumWidth(10);
    this->legendColorFrame.setMinimumWidth(10);
    this->legendColorFrame.setFrameStyle(QFrame::Panel | QFrame::Raised);
    this->legendColorFrame.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

    this->legendColorLayout = new QHBoxLayout();
    this->legendColorLayout->setContentsMargins(1, 1, 4, 1);
    this->legendColorLayout->setSpacing(0);
    this->legendColorLayout->addWidget(&this->legendColorFrame);
    this->legendColorWidget.setLayout(this->legendColorLayout);

    this->legendLabel.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->legendLabel.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->legendLabel.setCursor(Qt::IBeamCursor);
    this->legendLabel.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    this->legendLabel.setStyleSheet("color: rgb(28, 28, 28);");

    this->lastLabel.setMaximumWidth(70);
    this->lastLabel.setMinimumWidth(70);
    this->lastLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    this->lastLabel.setAlignment(Qt::AlignCenter);
    this->lastLabel.setCursor(Qt::IBeamCursor);
    this->lastLabel.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    this->minLabel.setMaximumWidth(70);
    this->minLabel.setMinimumWidth(70);
    this->minLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    this->minLabel.setAlignment(Qt::AlignCenter);
    this->minLabel.setCursor(Qt::IBeamCursor);
    this->minLabel.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    this->avgLabel.setMaximumWidth(70);
    this->avgLabel.setMinimumWidth(70);
    this->avgLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    this->avgLabel.setAlignment(Qt::AlignCenter);
    this->avgLabel.setCursor(Qt::IBeamCursor);
    this->avgLabel.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    this->maxLabel.setMaximumWidth(70);
    this->maxLabel.setMinimumWidth(70);
    this->maxLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    this->maxLabel.setAlignment(Qt::AlignCenter);
    this->maxLabel.setCursor(Qt::IBeamCursor);
    this->maxLabel.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    this->mainLayout.addWidget(&this->legendColorWidget);
    this->mainLayout.addWidget(&this->legendLabel);
    this->mainLayout.addWidget(&this->lastLabel);
    this->mainLayout.addWidget(&this->minLabel);
    this->mainLayout.addWidget(&this->avgLabel);
    this->mainLayout.addWidget(&this->maxLabel);
    this->setLayout(&this->mainLayout);
}

SNMPGraphLegendItem::~SNMPGraphLegendItem()
{
    this->legendColorLayout->removeWidget(&this->legendColorFrame);
    delete this->legendColorLayout;
}

void SNMPGraphLegendItem::configure(int lineNumber, QString key, QString unit)
{
    this->legendLabel.setText(key);
    this->unit = unit;

    if (lineNumber % 2)
    {
        this->legendColorWidget.setStyleSheet(STYLE_NORMAL_EVEN);
        this->legendLabel.setStyleSheet(STYLE_NORMAL_EVEN);
        this->lastLabel.setStyleSheet(STYLE_NORMAL_EVEN);
        this->minLabel.setStyleSheet(STYLE_NORMAL_EVEN);
        this->avgLabel.setStyleSheet(STYLE_NORMAL_EVEN);
        this->maxLabel.setStyleSheet(STYLE_NORMAL_EVEN);
    } else
    {
        this->legendColorWidget.setStyleSheet(STYLE_NORMAL_ODD);
        this->legendLabel.setStyleSheet(STYLE_NORMAL_ODD);
        this->lastLabel.setStyleSheet(STYLE_NORMAL_ODD);
        this->minLabel.setStyleSheet(STYLE_NORMAL_ODD);
        this->avgLabel.setStyleSheet(STYLE_NORMAL_ODD);
        this->maxLabel.setStyleSheet(STYLE_NORMAL_ODD);
    }

    switch (lineNumber)
    {
        case 0:
            this->legendColorFrame.setStyleSheet("background: rgb(39, 113, 164);");
            this->legendColor.setRgb(39, 113, 164);
            break;

        case 1:
            this->legendColorFrame.setStyleSheet("background: rgb(255, 127, 14);");
            this->legendColor.setRgb(255, 127, 14);
            break;

        case 2:
            this->legendColorFrame.setStyleSheet("background: rgb(44, 160, 44);");
            this->legendColor.setRgb(44, 160, 44);
            break;

        case 3:
            this->legendColorFrame.setStyleSheet("background: rgb(214, 39, 40);");
            this->legendColor.setRgb(214, 39, 40);
            break;

        case 4:
            this->legendColorFrame.setStyleSheet("background: rgb(148, 103, 189);");
            this->legendColor.setRgb(148, 103, 189);
            break;

        case 5:
            this->legendColorFrame.setStyleSheet("background: rgb(140, 86, 75);");
            this->legendColor.setRgb(140, 86, 75);
            break;

        case 6:
            this->legendColorFrame.setStyleSheet("background: rgb(227, 119, 194);");
            this->legendColor.setRgb(227, 119, 194);
            break;

        case 7:
            this->legendColorFrame.setStyleSheet("background: rgb(0, 0, 61);");
            this->legendColor.setRgb(0, 0, 61);
            break;

        case 8:
            this->legendColorFrame.setStyleSheet("background: rgb(148, 148, 148);");
            this->legendColor.setRgb(148, 148, 148);
            break;
    }

    this->formatAsBytes = (this->unit == "B");

    this->reset();
}

QColor SNMPGraphLegendItem::getColor()
{
    return this->legendColor;
}

void SNMPGraphLegendItem::updateLegend(double lastValue, double averageValue, double maxValue, double minValue)
{
    this->lastLabel.setText(formatValue(lastValue));
    this->minLabel.setText(formatValue(minValue));
    this->avgLabel.setText(formatValue(averageValue));
    this->maxLabel.setText(formatValue(maxValue));
}

QString SNMPGraphLegendItem::formatValue(double value)
{
    if (this->formatAsBytes)
        if (value >= 1073741824000)
            return QString::number((double)value/1073741824000, 'f', 1).append('T').append(this->unit);
        else if (value >= 1073741824)
            return QString::number((double)value/1073741824, 'f', 1).append('G').append(this->unit);
        else if (value >= 1048576)
            return QString::number((double)value/1048576, 'f', 1).append('M').append(this->unit);
        else if (value >= 1024)
            return QString::number((double)value/1024, 'f', 1).append('K').append(this->unit);
        else
            return QString::number(value, 'f', 1).append(this->unit);
    else
        if (value >= 1000000000000)
            return QString::number((double)value/1000000000000, 'f', 1).append('T').append(this->unit);
        else if (value >= 1000000000)
            return QString::number((double)value/1000000000, 'f', 1).append('B').append(this->unit);
        else if (value >= 1000000)
            return QString::number((double)value/1000000, 'f', 1).append('M').append(this->unit);
        else if (value >= 1000)
            return QString::number((double)value/1000, 'f', 1).append('K').append(this->unit);
        else
            return QString::number(value, 'f', 1).append(this->unit);
}

void SNMPGraphLegendItem::reset()
{
    this->lastLabel.setText(formatValue(0.0));
    this->minLabel.setText(formatValue(0.0));
    this->avgLabel.setText(formatValue(0.0));
    this->maxLabel.setText(formatValue(0.0));
}
