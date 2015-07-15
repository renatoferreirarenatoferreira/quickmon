#include "tracerouteitem.h"

TracerouteItem::TracerouteItem(QWidget *parent) : QWidget(parent)
{
    this->mainLayout.setMargin(0);
    this->mainLayout.setSpacing(0);

    this->labelTTL.setMaximumWidth(25);
    this->labelTTL.setMinimumWidth(25);
    this->labelTTL.setMinimumHeight(15);
    this->labelTTL.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    this->labelTTL.setAlignment(Qt::AlignCenter);
    this->labelTTL.setCursor(Qt::IBeamCursor);
    this->labelTTL.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    this->labelTTL.setStyleSheet("color: rgb(28, 28, 28);");

    this->labelAddress.setMinimumHeight(15);
    this->labelAddress.setAlignment(Qt::AlignCenter);
    this->labelAddress.setCursor(Qt::IBeamCursor);
    this->labelAddress.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    this->labelAddress.setStyleSheet("color: rgb(28, 28, 28);");

    this->labelLastRTT.setMaximumWidth(60);
    this->labelLastRTT.setMinimumWidth(60);
    this->labelLastRTT.setMinimumHeight(15);
    this->labelLastRTT.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    this->labelLastRTT.setAlignment(Qt::AlignCenter);
    this->labelLastRTT.setCursor(Qt::IBeamCursor);
    this->labelLastRTT.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    this->labelLastRTT.setStyleSheet("color: rgb(28, 28, 28);");
    this->labelLastRTT.setText("-");

    this->labelAvgRTT.setMaximumWidth(60);
    this->labelAvgRTT.setMinimumWidth(60);
    this->labelAvgRTT.setMinimumHeight(15);
    this->labelAvgRTT.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    this->labelAvgRTT.setAlignment(Qt::AlignCenter);
    this->labelAvgRTT.setCursor(Qt::IBeamCursor);
    this->labelAvgRTT.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    this->labelAvgRTT.setStyleSheet("color: rgb(28, 28, 28);");
    this->labelAvgRTT.setText("-");

    this->labelAvgJitter.setMaximumWidth(60);
    this->labelAvgJitter.setMinimumWidth(60);
    this->labelAvgJitter.setMinimumHeight(15);
    this->labelAvgJitter.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    this->labelAvgJitter.setAlignment(Qt::AlignCenter);
    this->labelAvgJitter.setCursor(Qt::IBeamCursor);
    this->labelAvgJitter.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    this->labelAvgJitter.setStyleSheet("color: rgb(28, 28, 28);");
    this->labelAvgJitter.setText("-");

    this->labelPctLoss.setMaximumWidth(60);
    this->labelPctLoss.setMinimumWidth(60);
    this->labelPctLoss.setMinimumHeight(15);
    this->labelPctLoss.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    this->labelPctLoss.setAlignment(Qt::AlignCenter);
    this->labelPctLoss.setCursor(Qt::IBeamCursor);
    this->labelPctLoss.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    this->labelPctLoss.setStyleSheet("color: rgb(28, 28, 28);");
    this->labelPctLoss.setText("-");

    this->mainLayout.addWidget(&this->labelTTL);
    this->mainLayout.addWidget(&this->labelAddress);
    this->mainLayout.addWidget(&this->labelLastRTT);
    this->mainLayout.addWidget(&this->labelAvgRTT);
    this->mainLayout.addWidget(&this->labelAvgJitter);
    this->mainLayout.addWidget(&this->labelPctLoss);
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

void TracerouteItem::setLastRTT(double rtt)
{
    QMetaObject::invokeMethod(this, "_setLastRTT", Qt::QueuedConnection, Q_ARG(double, rtt));
}

void TracerouteItem::setAvgRTT(double rtt)
{
    QMetaObject::invokeMethod(this, "_setAvgRTT", Qt::QueuedConnection, Q_ARG(double, rtt));
}

void TracerouteItem::setJitter(double jitter, float percentJitter)
{
    QMetaObject::invokeMethod(this, "_setJitter", Qt::QueuedConnection, Q_ARG(double, jitter), Q_ARG(float, percentJitter));
}

void TracerouteItem::setPctLoss(float pctLoss)
{
    QMetaObject::invokeMethod(this, "_setPctLoss", Qt::QueuedConnection, Q_ARG(float, pctLoss));
}

void TracerouteItem::_setTTL(int ttl)
{
    this->labelTTL.setText(QString::number(ttl));

    if (ttl % 2)
        this->setStyleSheet(STYLE_LABEL_NORMAL_EVEN);
    else
        this->setStyleSheet(STYLE_LABEL_NORMAL_ODD);

    this->TTL = ttl;
}

void TracerouteItem::_setAddress(QString address)
{
    if (address == TRACEROUTE_TIMEOUT_ADDRESS)
    {
        if (this->TTL % 2)
            this->setStyleSheet(STYLE_LABEL_RED_EVEN);
        else
            this->setStyleSheet(STYLE_LABEL_RED_ODD);

        this->labelLastRTT.setText("");
        this->labelAvgRTT.setText("");
        this->labelAvgJitter.setText("");
        this->labelPctLoss.setText("");
    }

    this->labelAddress.setText(address);
}

void TracerouteItem::_setLastRTT(double rtt)
{
    this->labelLastRTT.setText(QString::number(rtt,'f',2)+" ms");
}

void TracerouteItem::_setAvgRTT(double rtt)
{
    this->labelAvgRTT.setText(QString::number(rtt,'f',2)+" ms");
}

void TracerouteItem::_setJitter(double jitter, float percentJitter)
{
    this->labelAvgJitter.setText(QString::number(jitter,'f',2)+" ms");

    if (percentJitter >= JITTER_PERCENT_RED)
        if (this->TTL % 2)
            this->labelAvgJitter.setStyleSheet(STYLE_LABEL_RED_EVEN);
        else
            this->labelAvgJitter.setStyleSheet(STYLE_LABEL_RED_ODD);
    else if (percentJitter >= JITTER_PERCENT_YELLOW)
        if (this->TTL % 2)
            this->labelAvgJitter.setStyleSheet(STYLE_LABEL_YELLOW_EVEN);
        else
            this->labelAvgJitter.setStyleSheet(STYLE_LABEL_YELLOW_ODD);
    else
        if (this->TTL % 2)
            this->labelAvgJitter.setStyleSheet(STYLE_LABEL_NORMAL_EVEN);
        else
            this->labelAvgJitter.setStyleSheet(STYLE_LABEL_NORMAL_ODD);
}

void TracerouteItem::_setPctLoss(float pctLoss)
{
    this->labelPctLoss.setText(QString::number(pctLoss,'f',2)+" %");

    if (pctLoss >= LOSS_PERCENT_RED)
        if (this->TTL % 2)
            this->labelPctLoss.setStyleSheet(STYLE_LABEL_RED_EVEN);
        else
            this->labelPctLoss.setStyleSheet(STYLE_LABEL_RED_ODD);
    else if (pctLoss >= LOSS_PERCENT_YELLOW)
        if (this->TTL % 2)
            this->labelPctLoss.setStyleSheet(STYLE_LABEL_YELLOW_EVEN);
        else
            this->labelPctLoss.setStyleSheet(STYLE_LABEL_YELLOW_ODD);
    else
        if (this->TTL % 2)
            this->labelPctLoss.setStyleSheet(STYLE_LABEL_NORMAL_EVEN);
        else
            this->labelPctLoss.setStyleSheet(STYLE_LABEL_NORMAL_ODD);
}
