/* Copyright (c) 2012, STANISLAW ADASZEWSKI
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of STANISLAW ADASZEWSKI nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL STANISLAW ADASZEWSKI BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "qneport.h"
#include "qneblock.h"

#include <QGraphicsScene>
#include <QFontMetrics>

#include <QPen>
#include <QPainter>

#include "qneconnection.h"

QNEPort::QNEPort(QGraphicsItem *parent):
    QGraphicsPathItem(parent)
{
    m_label = new QGraphicsTextItem(this);

    m_radius = 5;
    m_margin = 2;

	QPainterPath p;
    p.addEllipse(-m_radius, -m_radius, 2*m_radius, 2*m_radius);
	setPath(p);

    setPen(QPen(Qt::black));
	setBrush(Qt::red);

	setFlag(QGraphicsItem::ItemSendsScenePositionChanges);

    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
}

QNEPort::~QNEPort()
{
    disconnectAll();
}

void QNEPort::disconnectAll()
{
    foreach(QNEConnection *conn, m_connections)
    {
        delete conn;
    }
    m_connections.clear();
}

void QNEPort::setNEBlock(QNEBlock *b)
{
	m_block = b;
}

void QNEPort::setId(const QString &n)
{
    m_id = n;
}

void QNEPort::setName(const QString &n)
{
    m_name = n;
    m_label->setPlainText(n);
    m_label->setDefaultTextColor(QColor(Qt::black));

    if (m_isOutput)
    {
        m_label->setPos(-m_radius - m_margin - m_label->boundingRect().width(), -m_label->boundingRect().height()/2);
    }
    else
    {
        m_label->setPos(m_radius + m_margin, -m_label->boundingRect().height()/2);
    }
}

void QNEPort::setIsOutput(bool o)
{
    m_isOutput = o;
}

void QNEPort::setDir(QVector2D dir)
{
    m_dir = dir;
}

QVector2D QNEPort::dir() const
{
    return m_dir;
}

int QNEPort::radius()
{
    return m_radius;
}

bool QNEPort::isOutput()
{
    return m_isOutput;
}

QVector<QNEConnection*>& QNEPort::connections()
{
	return m_connections;
}

void QNEPort::setPortType(uint32_t type)
{
    m_portType = type;
}

void QNEPort::setPortRate(uint32_t rate)
{
    m_portRate = rate;
}

QNEBlock* QNEPort::block() const
{
	return m_block;
}

bool QNEPort::isConnected(QNEPort *other)
{
	foreach(QNEConnection *conn, m_connections)
    {
        if (conn->port1() == other || conn->port2() == other)
        {
            return true;
        }
    }

	return false;
}

QVariant QNEPort::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemScenePositionHasChanged)
	{
		foreach(QNEConnection *conn, m_connections)
		{
			conn->updatePosFromPorts();
			conn->updatePath();
		}
	}
	return value;
}

void QNEPort::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QPen p = pen();
    QBrush b = brush();

    if (isSelected() || m_block->isSelected())
    {
        p.setColor(p.color().lighter(150));
        b.setColor(b.color().lighter(150));
    }
    if (isUnderMouse())
    {
        p.setColor(p.color().lighter(130));
        b.setColor(b.color().lighter(130));
    }

    painter->setPen(p);
    painter->setBrush(b);
    painter->drawPath(path());
}

