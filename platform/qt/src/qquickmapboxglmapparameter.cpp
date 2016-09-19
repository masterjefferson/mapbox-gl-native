#include <QQuickMapboxGLMapParameter>

#include <QEvent>
#include <QDynamicPropertyChangeEvent>

QQuickMapboxGLMapParameter::QQuickMapboxGLMapParameter(QQuickItem *parent)
    : QQuickItem(parent)
{
    qDebug() << __PRETTY_FUNCTION__;
    //setFlag(QQuickItem::ItemHasContents);
}

QString QQuickMapboxGLMapParameter::name() const
{
    return m_name;
}

void QQuickMapboxGLMapParameter::setName(const QString &name)
{
    if (!m_name.isEmpty()) {
        qWarning() << "Error: Name can only be set once.";
        return;
    }

    if (name.isEmpty()) {
        qWarning() << "Error: Name cannot be empty.";
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << name;

    m_name = name;
}

void QQuickMapboxGLMapParameter::componentComplete()
{
    QQuickItem::componentComplete();
    qDebug() << __PRETTY_FUNCTION__ << dynamicPropertyNames().size();
    qDebug() << __PRETTY_FUNCTION__ << metaObject()->propertyCount();
    for (auto i = 0; i < metaObject()->propertyCount(); ++i) {
        QMetaProperty prop = metaObject()->property(i);
        qDebug() << __PRETTY_FUNCTION__ << prop.name() << property(prop.name());
    }
}

bool QQuickMapboxGLMapParameter::event(QEvent *event)
{
    qDebug() << __PRETTY_FUNCTION__ << event;
    if (event->type() == QEvent::DynamicPropertyChange) {
        QDynamicPropertyChangeEvent *change = static_cast<QDynamicPropertyChangeEvent *>(event);
        qDebug() << "[" << m_name << "] " << change->propertyName() << ": " << property(change->propertyName());
    }
    return QQuickItem::event(event);
}

void QQuickMapboxGLMapParameter::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    qDebug() << __PRETTY_FUNCTION__ << change;
    QQuickItem::itemChange(change, value);
}
