#include "qquickmapboxglrenderer.hpp"

#include <QMapboxGL>
#include <QQuickMapboxGL>

#include <QSize>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickWindow>

QQuickMapboxGLRenderer::QQuickMapboxGLRenderer()
{
    QMapbox::initializeGLExtensions();

    QMapboxGLSettings settings;
    settings.setAccessToken(qgetenv("MAPBOX_ACCESS_TOKEN"));
    settings.setCacheDatabasePath("/tmp/mbgl-cache.db");
    settings.setCacheDatabaseMaximumSize(20 * 1024 * 1024);
    settings.setViewportMode(QMapboxGLSettings::FlippedYViewport);

    m_map.reset(new QMapboxGL(nullptr, settings));
    connect(m_map.data(), SIGNAL(mapChanged(QMapbox::MapChange)), this, SLOT(onMapChanged(QMapbox::MapChange)));
}

QQuickMapboxGLRenderer::~QQuickMapboxGLRenderer()
{
}

void QQuickMapboxGLRenderer::onMapChanged(QMapbox::MapChange change)
{
    switch (change) {
    case QMapbox::MapChangeWillStartLoadingMap:
    case QMapbox::MapChangeDidFailLoadingMap:
        m_styleLoaded = false;
        break;
    case QMapbox::MapChangeDidFinishLoadingMap:
        m_styleLoaded = true;
        break;
    default:
        break;
    }

    emit mapChanged(change);
}

QOpenGLFramebufferObject* QQuickMapboxGLRenderer::createFramebufferObject(const QSize &size)
{
    m_map->resize(size);

    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

    return new QOpenGLFramebufferObject(size, format);
}

void QQuickMapboxGLRenderer::render()
{
    m_map->render();
}

void QQuickMapboxGLRenderer::synchronize(QQuickFramebufferObject *item)
{
    if (!m_initialized) {
        auto qquickMapbox = static_cast<QQuickMapboxGL*>(item);

        QObject::connect(m_map.data(), &QMapboxGL::needsRendering, qquickMapbox, &QQuickMapboxGL::update);
        QObject::connect(this, &QQuickMapboxGLRenderer::centerChanged, qquickMapbox, &QQuickMapboxGL::setCenter);

        m_initialized = true;
    }

    auto quickMap = static_cast<QQuickMapboxGL*>(item);
    auto syncStatus = quickMap->swapSyncState();

    if (syncStatus & QQuickMapboxGL::CenterNeedsSync || syncStatus & QQuickMapboxGL::ZoomNeedsSync) {
        const auto& center = quickMap->center();
        m_map->setCoordinateZoom({ center.latitude(), center.longitude() }, quickMap->zoomLevel());
    }

    if (syncStatus & QQuickMapboxGL::StyleNeedsSync && !quickMap->m_styleUrl.isEmpty()) {
        m_map->setStyleUrl(quickMap->m_styleUrl);
        m_styleLoaded = false;
    }

    if (syncStatus & QQuickMapboxGL::PanNeedsSync) {
        m_map->moveBy(quickMap->swapPan());
        emit centerChanged(QGeoCoordinate(m_map->latitude(), m_map->longitude()));
    }

    if (syncStatus & QQuickMapboxGL::BearingNeedsSync) {
        m_map->setBearing(quickMap->bearing());
    }

    if (syncStatus & QQuickMapboxGL::PitchNeedsSync) {
        m_map->setPitch(quickMap->pitch());
    }

    if (m_styleLoaded) {
        if (!quickMap->m_stylePropertyChanges.empty()) {
            for (const auto& change: quickMap->m_stylePropertyChanges) {
                if (change.isPaintProperty) {
                    m_map->setPaintProperty(change.layer, change.property, change.value, change.klass);
                } else {
                    m_map->setLayoutProperty(change.layer, change.property, change.value);
                }
            }
            quickMap->m_stylePropertyChanges.clear();
        }
    }
}
