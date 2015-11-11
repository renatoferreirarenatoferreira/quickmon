#include "snmplistitem.h"

SNMPListItem::SNMPListItem(QWidget *parent) : QWidget(parent)
{
    this->mainLayout.setMargin(0);
    this->mainLayout.setSpacing(0);

    this->labelKey.setMaximumWidth(160);
    this->labelKey.setMinimumWidth(160);
    this->labelKey.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    this->labelKey.setAlignment(Qt::AlignRight);
    this->labelKey.setCursor(Qt::IBeamCursor);
    this->labelKey.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    this->labelKey.setStyleSheet("color: rgb(28, 28, 28);");

    this->labelSeparator.setMaximumWidth(10);
    this->labelSeparator.setMinimumWidth(10);
    this->labelSeparator.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    this->labelSeparator.setAlignment(Qt::AlignHCenter);
    this->labelSeparator.setStyleSheet("color: rgb(28, 28, 28);");
    this->labelSeparator.setText(":");

    this->labelValue.setMaximumWidth(424);
    this->labelValue.setMinimumWidth(424);
    this->labelValue.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    this->labelValue.setAlignment(Qt::AlignLeft);
    this->labelValue.setCursor(Qt::IBeamCursor);
    this->labelValue.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    this->labelValue.setStyleSheet("color: rgb(28, 28, 28);");
    this->labelValue.setWordWrap(true);

    this->mainLayout.addWidget(&this->labelKey);
    this->mainLayout.addWidget(&this->labelSeparator);
    this->mainLayout.addWidget(&this->labelValue);
    this->setLayout(&this->mainLayout);
}

SNMPListItem::~SNMPListItem()
{

}

void SNMPListItem::configure(int lineNumber, QString key)
{
    this->labelKey.setText(key);

    if (lineNumber % 2)
    {
        this->labelKey.setStyleSheet(STYLE_LABEL_NORMAL_EVEN);
        this->labelSeparator.setStyleSheet(STYLE_LABEL_NORMAL_EVEN);
        this->labelValue.setStyleSheet(STYLE_LABEL_NORMAL_EVEN);
    } else
    {
        this->labelKey.setStyleSheet(STYLE_LABEL_NORMAL_ODD);
        this->labelSeparator.setStyleSheet(STYLE_LABEL_NORMAL_ODD);
        this->labelValue.setStyleSheet(STYLE_LABEL_NORMAL_ODD);
    }
}

void SNMPListItem::setValue(QString value)
{
    QMetaObject::invokeMethod(this, "_setValue", Qt::QueuedConnection, Q_ARG(QString, value));
}

void SNMPListItem::_setValue(QString value)
{
    if (value.size() > 0)
        this->labelValue.setText(value);
    else
        this->labelValue.clear();
}
