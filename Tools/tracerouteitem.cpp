#include "tracerouteitem.h"

TracerouteItem::TracerouteItem(QWidget *parent) : QWidget(parent)
{
    this->mainLayout.setMargin(0);
    this->mainLayout.setSpacing(0);

    this->labelTTL.setMaximumWidth(30);
    this->labelTTL.setMinimumWidth(30);
    this->labelTTL.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    this->labelTTL.setAlignment(Qt::AlignCenter);
    this->labelTTL.setCursor(Qt::IBeamCursor);
    this->labelTTL.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    this->labelAddress.setAlignment(Qt::AlignCenter);
    this->labelAddress.setCursor(Qt::IBeamCursor);
    this->labelAddress.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    this->labelRTT.setMaximumWidth(50);
    this->labelRTT.setMinimumWidth(50);
    this->labelRTT.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    this->labelRTT.setAlignment(Qt::AlignCenter);
    this->labelRTT.setCursor(Qt::IBeamCursor);
    this->labelRTT.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    this->mainLayout.addWidget(&this->labelTTL);
    this->mainLayout.addWidget(&this->labelAddress);
    this->mainLayout.addWidget(&this->labelRTT);
    this->setLayout(&this->mainLayout);
}

TracerouteItem::~TracerouteItem()
{

}

void TracerouteItem::setTTL(int ttl)
{
    QMetaObject::invokeMethod(this, "_setTTL", Qt::QueuedConnection, Q_ARG(int, ttl));
}

void TracerouteItem::setAddress(QString address)
{
    QMetaObject::invokeMethod(this, "_setAddress", Qt::QueuedConnection, Q_ARG(QString, address));
}

void TracerouteItem::setRTT(double rtt)
{
    QMetaObject::invokeMethod(this, "_setRTT", Qt::QueuedConnection, Q_ARG(double, rtt));
}

void TracerouteItem::_setTTL(int ttl)
{
    this->labelTTL.setText(QString::number(ttl));

    if (ttl % 2)
        this->setStyleSheet("background: rgb(204, 204, 204)");

    this->TTL = ttl;
}

void TracerouteItem::_setAddress(QString address)
{
    this->labelAddress.setText(address);
}

void TracerouteItem::_setRTT(double rtt)
{
    if (rtt == TRACEROUTE_TIMEOUT_RTT)
    {
        this->labelRTT.setText("Timeout!");

        if (this->TTL % 2)
            this->setStyleSheet("background: rgb(255, 178, 178)");
        else
            this->setStyleSheet("background: rgb(255, 230, 230)");
    }
    else
        this->labelRTT.setText(QString::number(rtt,'f',2)+" ms");
}
