#include <QQuickMapboxGLStyle>

QQuickMapboxGLStyle::QQuickMapboxGLStyle(QQuickItem *parent)
    : QQuickItem(parent)
{
}

void QQuickMapboxGLStyle::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);

    switch (change) {
    case QQuickItem::ItemChildAddedChange:
        break;
    case QQuickItem::ItemChildRemovedChange:
        break;
    default:
        break;
    }
}

void QQuickMapboxGLStyle::setUrl(const QString &url)
{
    if (url == m_url) {
        return;
    }

    m_url = url;
    emit urlChanged(url);
}

QString QQuickMapboxGLStyle::url() const
{
    return m_url;
}

void QQuickMapboxGLStyle::setStyleClass(const QString &styleClass)
{
    if (styleClass == m_class) {
        return;
    }

    m_class = styleClass;
    emit classChanged(styleClass);
}

QString QQuickMapboxGLStyle::styleClass() const
{
    return m_class;
}
