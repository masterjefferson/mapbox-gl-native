#include "qquickmapboxglrenderer.hpp"

#include <mbgl/util/constants.hpp>

#include <QMapbox>
#include <QQuickMapboxGL>
#include <QQuickMapboxGLMapParameter>

#include <QDebug>
#include <QQuickItem>
#include <QRegularExpression>
#include <QString>
#include <QtGlobal>
#include <QQmlListProperty>

#include <cmath>

QQuickMapboxGL::QQuickMapboxGL(QQuickItem *parent_)
    : QQuickFramebufferObject(parent_)
{
}

QQuickMapboxGL::~QQuickMapboxGL()
{
}

QQuickFramebufferObject::Renderer *QQuickMapboxGL::createRenderer() const
{
    QQuickMapboxGLRenderer *renderer = new QQuickMapboxGLRenderer();
    connect(renderer, SIGNAL(mapChanged(QMapbox::MapChange)), this, SIGNAL(mapChanged(QMapbox::MapChange)));
    return renderer;
}

void QQuickMapboxGL::setMinimumZoomLevel(qreal zoom)
{
    zoom = qMax(mbgl::util::MIN_ZOOM, zoom);
    zoom = qMin(m_maximumZoomLevel, zoom);

    if (m_minimumZoomLevel == zoom) {
        return;
    }

    m_minimumZoomLevel = zoom;
    setZoomLevel(m_zoomLevel); // Constrain.

    emit minimumZoomLevelChanged();
}

qreal QQuickMapboxGL::minimumZoomLevel() const
{
    return m_minimumZoomLevel;
}

void QQuickMapboxGL::setMaximumZoomLevel(qreal zoom)
{
    zoom = qMin(mbgl::util::MAX_ZOOM, zoom);
    zoom = qMax(m_minimumZoomLevel, zoom);

    if (m_maximumZoomLevel == zoom) {
        return;
    }

    m_maximumZoomLevel = zoom;
    setZoomLevel(m_zoomLevel); // Constrain.

    emit maximumZoomLevelChanged();
}

qreal QQuickMapboxGL::maximumZoomLevel() const
{
    return m_maximumZoomLevel;
}

void QQuickMapboxGL::setZoomLevel(qreal zoom)
{
    zoom = qMin(m_maximumZoomLevel, zoom);
    zoom = qMax(m_minimumZoomLevel, zoom);

    if (m_zoomLevel == zoom) {
        return;
    }

    m_zoomLevel = zoom;

    m_syncState |= ZoomNeedsSync;
    update();

    emit zoomLevelChanged(m_zoomLevel);
}

qreal QQuickMapboxGL::zoomLevel() const
{
    return m_zoomLevel;
}

void QQuickMapboxGL::setCenter(const QGeoCoordinate &coordinate)
{
    if (m_center == coordinate) {
        return;
    }

    m_center = coordinate;

    m_syncState |= CenterNeedsSync;
    update();

    emit centerChanged(m_center);
}

QGeoCoordinate QQuickMapboxGL::center() const
{
    return m_center;
}

QGeoServiceProvider::Error QQuickMapboxGL::error() const
{
    return QGeoServiceProvider::NoError;
}

QString QQuickMapboxGL::errorString() const
{
    return QString();
}

void QQuickMapboxGL::setVisibleRegion(const QGeoShape &shape)
{
    m_visibleRegion = shape;
}

QGeoShape QQuickMapboxGL::visibleRegion() const
{
    return m_visibleRegion;
}

void QQuickMapboxGL::setCopyrightsVisible(bool)
{
    qWarning() << Q_FUNC_INFO << "Not implemented.";
}

bool QQuickMapboxGL::copyrightsVisible() const
{
    return false;
}

void QQuickMapboxGL::setColor(const QColor &color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;

    StyleProperty change;
    change.isPaintProperty = true;
    change.layer = "background";
    change.property = "background-color";
    change.value = color;
    m_stylePropertyChanges << change;

    update();

    emit colorChanged(m_color);
}

QColor QQuickMapboxGL::color() const
{
    return m_color;
}

void QQuickMapboxGL::pan(int dx, int dy)
{
    m_pan += QPointF(dx, -dy);

    m_syncState |= PanNeedsSync;
    update();
}

void QQuickMapboxGL::setStyle(QQuickMapboxGLStyle *style)
{
    if (style == m_style) {
        return;
    }

    disconnect(style, SIGNAL(urlChanged(QString)), this, SLOT(onStyleChanged()));
    delete m_style;
    m_style = style;
    if (style) {
        style->setParentItem(this);
        connect(style, SIGNAL(urlChanged(QString)), this, SLOT(onStyleChanged()));
    }

    m_syncState |= StyleNeedsSync;
    update();

    emit styleChanged();
}

QQuickMapboxGLStyle *QQuickMapboxGL::style() const
{
    return m_style;
}

void QQuickMapboxGL::setBearing(qreal angle)
{
    angle = std::fmod(angle, 360.);

    if (m_bearing == angle) {
        return;
    }

    m_bearing = angle;

    m_syncState |= BearingNeedsSync;
    update();

    emit bearingChanged(m_bearing);
}

qreal QQuickMapboxGL::bearing() const
{
    return m_bearing;
}

void QQuickMapboxGL::setPitch(qreal angle)
{
    angle = qMin(qMax(0., angle), mbgl::util::PITCH_MAX * mbgl::util::RAD2DEG);

    if (m_pitch == angle) {
        return;
    }

    m_pitch = angle;

    m_syncState |= PitchNeedsSync;
    update();

    emit pitchChanged(m_pitch);
}

qreal QQuickMapboxGL::pitch() const
{
    return m_pitch;
}

QPointF QQuickMapboxGL::swapPan()
{
    QPointF oldPan = m_pan;

    m_pan = QPointF(0, 0);

    return oldPan;
}

int QQuickMapboxGL::swapSyncState()
{
    int oldState = m_syncState;

    m_syncState = NothingNeedsSync;

    return oldState;
}

void QQuickMapboxGL::onParameterPropertyUpdated(const QString &propertyName)
{
    static QRegularExpression camelCase {"([a-z0-9])([A-Z])"};
    static QRegularExpression CamelCase {"(.)([A-Z][a-z]+)"};

    // Ignore type changes.
    if (propertyName == "type") {
        return;
    }

    QQuickMapboxGLMapParameter *param = qobject_cast<QQuickMapboxGLMapParameter *>(sender());

    QString type = param->property("type").toString();
    if (type == "paint" || type == "layout") {
        // XXX: Ignore 'layer' and 'class' changes.
        if (propertyName == "layer" || propertyName == "class") {
            return;
        }

        QString layer = param->property("layer").toString();
        if (layer.isEmpty()) {
            qWarning() << "Error: Property 'layer' should be set for style properties.";
            return;
        }

        StyleProperty change;
        change.isPaintProperty = type.at(0) == 'p';
        change.layer = layer;
        change.property = QString(propertyName).replace(CamelCase, "\\1-\\2")
                                               .replace(camelCase, "\\1-\\2")
                                               .toLower();
        change.value = param->property(propertyName.toLatin1());
        if (change.isPaintProperty) {
            change.klass = param->property("class").toString();
        }
        m_stylePropertyChanges << change;

    } else if (type == "style") {
    } else if (type == "layer") {
    } else if (type == "source") {
    } else if (type == "filter") {
    } else {
        qWarning() << "Error: Property 'type' should be one of: 'style', 'paint', 'layout', 'layer', 'source' or 'filter'.";
        return;
    }

    update();
}

void QQuickMapboxGL::onStyleChanged()
{
    m_syncState |= StyleNeedsSync;
    update();

    emit styleChanged();
}

void QQuickMapboxGL::appendParameter(QQmlListProperty<QQuickMapboxGLMapParameter> *prop, QQuickMapboxGLMapParameter *param)
{
    QQuickMapboxGL *map = static_cast<QQuickMapboxGL *>(prop->object);
    map->m_parameters.append(param);
    QObject::connect(param, SIGNAL(propertyUpdated(QString)), map, SLOT(onParameterPropertyUpdated(QString)));
}

int QQuickMapboxGL::countParameters(QQmlListProperty<QQuickMapboxGLMapParameter> *prop)
{
    QQuickMapboxGL *map = static_cast<QQuickMapboxGL *>(prop->object);
    return map->m_parameters.count();
}

QQuickMapboxGLMapParameter *QQuickMapboxGL::parameterAt(QQmlListProperty<QQuickMapboxGLMapParameter> *prop, int index)
{
    QQuickMapboxGL *map = static_cast<QQuickMapboxGL *>(prop->object);
    return map->m_parameters[index];
}

void QQuickMapboxGL::clearParameter(QQmlListProperty<QQuickMapboxGLMapParameter> *prop)
{
    QQuickMapboxGL *map = static_cast<QQuickMapboxGL *>(prop->object);
    for (auto param : map->m_parameters) {
        QObject::disconnect(param, SIGNAL(propertyUpdated(QString)), map, SLOT(onParameterPropertyUpdated(QString)));
    }
    map->m_parameters.clear();
}

QQmlListProperty<QQuickMapboxGLMapParameter> QQuickMapboxGL::parameters()
{
    return QQmlListProperty<QQuickMapboxGLMapParameter>(this,
            nullptr,
            appendParameter,
            countParameters,
            parameterAt,
            clearParameter);
}
