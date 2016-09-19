#ifndef QQUICKMAPBOXGLMAPPARAMETER_H
#define QQUICKMAPBOXGLMAPPARAMETER_H

#include <QObject>
#include <QQuickItem>
#include <QString>

class Q_DECL_EXPORT QQuickMapboxGLMapParameter : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName)

public:
    QQuickMapboxGLMapParameter(QQuickItem *parent = Q_NULLPTR);
    virtual ~QQuickMapboxGLMapParameter() {}

    // QObject implementation
    bool event(QEvent *) override;

    // QQmlParserStatus implementation
    void componentComplete() override;

    QString name() const;
    void setName(const QString &name);

protected:
    // QQuickItem implementation
    void itemChange(QQuickItem::ItemChange, const QQuickItem::ItemChangeData &) override;

private:
    QString m_name;
};

QML_DECLARE_TYPE(QQuickMapboxGLMapParameter)

#endif // QQUICKMAPBOXGLMAPPARAMETER_H
