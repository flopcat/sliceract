#include <QMouseEvent>
#include <QPainter>
#include <utility>
#include "previewwidget.h"

PreviewWidget::PreviewWidget()
{
    this->setMouseTracking(true);
}

QRect PreviewWidget::imageSelection()
{
    QRect selection(selectionRect);
    if (drawnPoint.y() == 0) {
        selection.adjust(-drawnPoint.x(),0,-drawnPoint.x(),0);
    } else {
        selection.adjust(0,-drawnPoint.y(),0,-drawnPoint.y());
    }
    //qDebug() << selection << image.size();
    double factor = image.size().height() / std::max((double)drawnSize.height(), 1.0);
    double mx0 = selection.left() * factor;
    double my0 = selection.top() * factor;
    double mx1 = selection.right() * factor;
    double my1 = selection.bottom() * factor;
    //qDebug() << factor;
    selection = QRect(QPoint(mx0+0.5, my0+0.5), QSize(0.5+mx1-mx0,0.5+my1-my0));
    //qDebug() << selection;
    //qDebug() << QRect(QPoint(0,0),image.size());
    return selection.intersected(QRect(QPoint(0,0),image.size()));
}

QString PreviewWidget::imageFilename()
{
    return imageFile;
}

void PreviewWidget::loadImage(QString file)
{
    if (!image.load(file)) {
        imageFile.clear();
        image = QPixmap();
    } else {
        imageFile = file;
    }
    resizeGL(size().width(), size().height());
    update();
}

void PreviewWidget::resizeGL(int w, int h)
{
    glSize = {w,h};
    if (image.isNull()) {
        cachedPixmap = QPixmap();
        return;
    }
    if (image.width() > image.height()) {
        cachedPixmap = image.scaledToWidth(w, Qt::SmoothTransformation);
    } else {
        cachedPixmap = image.scaledToHeight(h, Qt::SmoothTransformation);
    }
}

void PreviewWidget::paintGL()
{
    QPainter p(this);
    if (image.isNull()) {
        p.fillRect(QRect(QPoint(),glSize), QBrush(QColor(0,0,255)));
        p.drawText(16, 16, "No Signal");
        return;
    }
    QSize pixSize = cachedPixmap.size();
    const int x = int((this->rect().width() - pixSize .width()) / 2.0);
    const int y = int((this->rect().height() - pixSize .height()) / 2.0);
    p.drawPixmap(x,y, pixSize.width(), pixSize.height(), cachedPixmap);
    drawnPoint = {x,y};
    drawnSize = pixSize;

    if (!selectionRect.isNull()) {
        QBrush fill(QColor(63,63,160,127));
        QPen line(QColor(32,32,192,192));
        p.setBrush(fill);
        p.setPen(line);
        p.drawRect(selectionRect);
    }

    constexpr int previewSizeX = 400;
    constexpr int previewSizeY = 200;
    constexpr int previewHalfX = previewSizeX / 2;
    constexpr int previewHalfY = previewSizeY / 2;
    // now draw the preview!
    QPoint mouse = drawnToImage(mp);
    int weightX = (mp.x() > glSize.width()/2) ? 0 : 1;
    int weightY = (mp.y() > glSize.height()/2) ? 0 : 1;
    QRect rcMag((glSize.width() - previewSizeX)*weightX,
                (glSize.height() - previewSizeY)*weightY,
                previewSizeX, previewSizeY);
    p.setClipRect(rcMag);
    p.fillRect(rcMag, QBrush(QColor(160,160,160)));
    p.drawPixmap(rcMag.left() - mouse.x() + previewHalfX,
                 rcMag.top() - mouse.y() + previewHalfY, image);
    p.drawLine((rcMag.left() + rcMag.right())/2, rcMag.top(),
               (rcMag.left() + rcMag.right())/2, rcMag.bottom());
    p.drawLine(rcMag.left(), (rcMag.top() + rcMag.bottom())/2,
               rcMag.right(), (rcMag.top() + rcMag.bottom())/2);
}

void PreviewWidget::mousePressEvent(QMouseEvent *ev)
{
    mp = {ev->x(), ev->y()};
    update();

    if (ev->button() != Qt::LeftButton)
        return;
    mp0 = mp1 = mp;
    updateSelection();
}

void PreviewWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
}

void PreviewWidget::mouseMoveEvent(QMouseEvent *ev)
{
    mp = {ev->x(), ev->y()};
    update();

    if (!ev->buttons().testFlag(Qt::LeftButton))
        return;
    mp1 = mp;
    updateSelection();
}

QPoint PreviewWidget::drawnToImage(QPoint p)
{
    p -= drawnPoint;
    p *= image.height() / std::max((double)drawnSize.height(), 1.0);
    return {std::clamp(p.x(), 0, image.width()-1),
            std::clamp(p.y(), 0, image.height()-1)};
}

void PreviewWidget::updateSelection()
{
    int m0x = std::min(mp0.x(), mp1.x());
    int m1x = std::max(mp0.x(), mp1.x());
    int m0y = std::min(mp0.y(), mp1.y());
    int m1y = std::max(mp0.y(), mp1.y());
    selectionRect = QRect(m0x, m0y, m1x-m0x, m1y-m0y);
}
