#ifndef QQUICKMAPBOXGLMAPPARAMETER_H
#define QQUICKMAPBOXGLMAPPARAMETER_H

#include <QObject>
#include <QQmlParserStatus>
#include <qqml.h>

class Q_DECL_EXPORT QQuickMapboxGLMapParameter : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    QQuickMapboxGLMapParameter(QObject *parent = 0);
    virtual ~QQuickMapboxGLMapParameter() {};

protected:
    // QQmlParserStatus implementation
    void classBegin() override {}
    void componentComplete() override;

public slots:
    void propertyChanged(int id);

private:
    int m_metaPropertyOffset;
};

QML_DECLARE_TYPE(QQuickMapboxGLMapParameter)

#endif // QQUICKMAPBOXGLMAPPARAMETER_H
