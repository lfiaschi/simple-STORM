#include "previewimage.h"

#include "guiparams.h"

#include <QEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QStyleOption>
#include <QDir>
#include <QtConcurrentRun>
#include <QFutureWatcher>

PreviewImage::PreviewImage(QWidget *parent)
: QWidget(parent), m_lastProcessed(std::chrono::steady_clock::now()), m_updateInterval(std::chrono::milliseconds(1000)), m_detections(0), m_intensityScaleFactor(0.1), m_scale(1), m_geometry(0,0,0,0), m_pixmap(), m_pixmapGeometry(m_geometry), m_needRepaint(false), m_painter(), m_results(0), m_params(0), m_initialized(false)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

PreviewImage::~PreviewImage()
{
}

void PreviewImage::setResults(const std::vector<std::set<Coord<float>>> *results)
{
    m_results = results;
}

void PreviewImage::setUpdateInterval(const std::chrono::milliseconds &interval)
{
    m_updateInterval = interval;
}

void PreviewImage::setParams(const GuiParams *params)
{
    m_params = params;
    vigra::Shape2 resultShape(m_params->shape(0) * m_params->getPixelSize() / m_params->getReconstructionResolution(), m_params->shape(1) * m_params->getPixelSize() / m_params->getReconstructionResolution());
    m_sizeFactor = m_params->getPixelSize() / (m_params->getReconstructionResolution() * m_params->getFactor());
    m_size = m_resultSize = QSize(resultShape[0], resultShape[1]);
    setBaseSize(m_resultSize);
    QFuture<void> f = QtConcurrent::run([this, resultShape](){m_result.reshape(resultShape, 0.);});
    QFutureWatcher<void> *w = new QFutureWatcher<void>();
    connect(w, SIGNAL(finished()), this, SLOT(initializationFinished()));
    w->setFuture(f);
}

void PreviewImage::frameCompleted(int frame)
{
    m_unprocessed.push_back(frame);
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (m_initialized && now - m_lastProcessed >= m_updateInterval) {
        updateImage();
    }
}

void PreviewImage::updateImage()
{
    if (!m_params || !m_results || !m_initialized)
        return;
    m_lastProcessed = std::chrono::steady_clock::now();
    for (int frame : m_unprocessed) {
        for (const Coord<float> &c : m_results->at(frame)) {
            int x = std::floor(c.x * m_sizeFactor), y = std::floor(c.y * m_sizeFactor);
            float val = m_result(x, y);
            if (val > 0)
                m_resultsForScaling.erase(m_resultsForScaling.find(val));
            val += c.val;
            m_resultsForScaling.insert(val);
            m_result(x, y) = val;
            ++m_detections;
        }
    }

    // some maxima are very strong so we scale the image as appropriate :
    m_limits.first = m_limits.second = 0;
    auto it = --m_resultsForScaling.end();
    for (size_t i = m_resultsForScaling.size(); i >= m_resultsForScaling.size() * 0.996; --i, --it)
        m_limits.second = *it;
    m_intensityFactor = 255 / (m_limits.second - m_limits.first);
    if (m_scale < 1) {
        for (int frame : m_unprocessed) {
            for (const Coord<float> &c : m_results->at(frame)) {
                int x = std::floor(c.x * m_sizeFactor), y = std::floor(c.y * m_sizeFactor);
                m_toPaint.insert(QPair<int, int>(x, y));
            }
        }
    } else
        m_toPaint.clear();

    m_needRepaint = true;
    m_unprocessed.clear();
    emit detections(QString("%1 detections").arg(m_detections));
}

void PreviewImage::updatePixmap(const QRect &rect)
{
    if (!m_params || !m_results || !m_initialized)
        return;
    QPoint ul = rect.topLeft();
    QPoint lr = rect.bottomRight();
    m_painter.translate(-ul);
    m_painter.scale(m_scale, m_scale);
    if (m_toPaint.empty()) {
        for (int y = ul.y() / m_scale; y < lr.y() / m_scale; ++y) {
            for (int x = ul.x() / m_scale; x < lr.x() / m_scale; ++x) {
                float v = m_result(x, y);
                uchar c = (v > m_limits.second) ? 255 : v * m_intensityFactor;
                m_painter.fillRect(x, y, 1, 1, QColor(c, c, c));
            }
        }
    } else {
        for (const QPair<int, int> &p : m_toPaint) {
            m_painter.fillRect(p.first, p.second, 1, 1, QColor(255, 255, 255));
        }
        m_toPaint.clear();
    }
    m_painter.resetTransform();
}

void PreviewImage::saveImage(const QString &file)
{
    vigra::exportImage(srcImageRange(m_result), QDir::toNativeSeparators(file).toStdString().c_str());
}

void PreviewImage::resizeEvent(QResizeEvent *event)
{
    event->accept();
    m_size.scale(event->size(), Qt::KeepAspectRatio);
    updateGeometry();
    if (m_params && m_results) {
        m_scale = m_size.width() /(float)m_resultSize.width();
        m_needRepaint = true;
    }
    m_toPaint.clear();
}

void PreviewImage::moveEvent(QMoveEvent *event)
{
    m_needRepaint = true;
    QWidget::moveEvent(event);
}

void PreviewImage::paintEvent(QPaintEvent *event)
{
    event->accept();
    QRect rect = event->rect();
    init(rect);
    if (m_needRepaint) {
        updatePixmap(rect);
        m_needRepaint = false;
    }
    if (!isEnabled()) {
        QPixmap p(rect.width(), rect.height());
        QPainter(&p).drawImage(m_pixmapGeometry, m_pixmap, m_pixmapGeometry);
        QStyleOption opt;
        opt.initFrom(this);
        p = style()->generatedIconPixmap(QIcon::Disabled, p, &opt);
        QPainter(this).drawPixmap(rect, p);
    } else
        QPainter(this).drawImage(rect, m_pixmap, m_pixmapGeometry);
}

void PreviewImage::init(const QRect &rect)
{
    if (rect != m_geometry) {
        m_painter.end();
        m_geometry = m_pixmapGeometry = rect;
        m_pixmapGeometry.moveTo(0, 0);
        int width = std::max(rect.width(), m_pixmap.width());
        int height = std::max(rect.height(), m_pixmap.height());
        if (width > m_pixmap.width() || height > m_pixmap.height())
            m_pixmap = QImage(width, height, QImage::Format_RGB32);
        m_painter.begin(&m_pixmap);
        m_needRepaint = true;
    }
}

float PreviewImage::scale() const
{
    return m_scale;
}

QSize PreviewImage::sizeHint() const
{
    return m_size;
}

void PreviewImage::setIntensityScaleFactor(float factor)
{
    m_intensityScaleFactor = factor;
}

void PreviewImage::initializationFinished()
{
    delete sender();
    m_initialized = true;
    emit initialized();
}