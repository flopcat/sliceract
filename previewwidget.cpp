#include <QMouseEvent>
#include <QPainter>
#include <utility>
#include "previewwidget.h"

PreviewWidget::PreviewWidget(QWidget *owner) : QOpenGLWidget(owner)
{
    this->setMouseTracking(true);
    this->setFont(QFont("Monospace", 12));
}

QSize PreviewWidget::imageSize()
{
    return image.size();
}

QRect PreviewWidget::imageSelection()
{
    return selectionRect.intersected(QRect(QPoint(0,0),image.size()));
}

QString PreviewWidget::imageFilename()
{
    return imageFile;
}

void PreviewWidget::loadImage(QString file)
{
    QSize size0 = imageSize();
    if (image.load(file))
        imageFile = file;
    QSize size1 = imageSize();
    if (size0 != size1) {
        imageOffset = QPoint();
        emit imageSizeChanged(size1);
    }
    resizeGL(size().width(), size().height());
    update();
}

void PreviewWidget::offsetImage(QPoint topLeft)
{
    imageOffset = topLeft;
    update();
}

void PreviewWidget::resizeGL(int w, int h)
{
    glSize = {w,h};
}

void PreviewWidget::paintGL()
{
    QPainter p(this);
    if (image.isNull()) {
        p.fillRect(QRect(QPoint(),glSize), QBrush(QColor(0,0,255)));

        p.setPen(QColor(255,255,255));
        const QString text = "NO SIGNAL";
        QFontMetrics fm(font());
        int x = (glSize.width() - fm.width(text))/2;
        int y = (glSize.height() - fm.height())/2;
        p.drawText(x, y, text);
        return;
    }

    p.translate(-imageOffset.x(), -imageOffset.y());
    p.drawPixmap(QPoint(), image);

    if (!selectionRect.isNull()) {
        QBrush fill(QColor(63,63,160,127));
        QPen line(QColor(32,32,192,192));
        p.setBrush(fill);
        p.setPen(line);
        p.drawRect(selectionRect);
    }
}

void PreviewWidget::mousePressEvent(QMouseEvent *ev)
{
    mp = {ev->x(), ev->y()};

    if (ev->button() == Qt::LeftButton)
        mp0 = mp + imageOffset;
    if (ev->button() == Qt::RightButton)
        mp1 = mp + imageOffset;
    updateSelection();
    update();
}

void PreviewWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
}

void PreviewWidget::mouseMoveEvent(QMouseEvent *ev)
{
    mp = {ev->x(), ev->y()};

    if (ev->buttons().testFlag(Qt::LeftButton))
        mp0 = mp + imageOffset;
    if (ev->buttons().testFlag(Qt::RightButton))
        mp1 = mp + imageOffset;
    updateSelection();
    update();
}

void PreviewWidget::wheelEvent(QWheelEvent *ev)
{
    if (ev->modifiers().testFlag(Qt::ShiftModifier))
        emit wheelHorizontal(-ev->delta());
    else
        emit wheelVertical(-ev->delta());
}

void PreviewWidget::updateSelection()
{
    int m0x = std::min(mp0.x(), mp1.x());
    int m1x = std::max(mp0.x(), mp1.x());
    int m0y = std::min(mp0.y(), mp1.y());
    int m1y = std::max(mp0.y(), mp1.y());
    selectionRect = QRect(m0x, m0y, m1x-m0x, m1y-m0y);
}
