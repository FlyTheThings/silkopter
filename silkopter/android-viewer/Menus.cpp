#include "Menus.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <QTimer>


Menus::Menus(QObject *parent)
    : QObject(parent)
{
}

void Menus::init(QQuickView& view)
{
    m_view = &view;
}

void Menus::pop()
{
    Q_ASSERT(m_stack.size() >= 1);
    Q_ASSERT(!m_isLocked);
    if (!m_isLocked)
    {
        m_isLocked = true;
        if (m_stack.size() < 1)
        {
            m_stack.clear();
        }
        else
        {
            m_stack.pop_back();
            QTimer::singleShot(0, this, SLOT(setView()));
        }
    }
}

void Menus::push(const QString& qml)
{
    Q_ASSERT(!m_isLocked);
    if (!m_isLocked)
    {
        m_isLocked = true;

//#ifdef RASPBERRY_PI
        m_stack.push_back(QUrl("qrc:/qml/" + qml));
//#else
        //m_stack.push_back(QUrl("qml/" + qml));
//#endif
        QTimer::singleShot(0, this, SLOT(setView()));
    }
}

void Menus::setView()
{
    Q_ASSERT(m_isLocked);
    m_view->setSource(m_stack.back());
    m_isLocked = false;

    //recover from bad screens
    if (m_view->status() == QQuickView::Error && m_stack.size() >= 2)
    {
        pop();
    }
}

