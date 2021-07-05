#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H
#include <QOpenGLWidget>
#include <QPixmap>

class PreviewWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QSize imageSize READ imageSize NOTIFY imageSizeChanged)

public:
    explicit PreviewWidget(QWidget *owner = nullptr);
    QSize imageSize();
    QRect imageSelection();
    QString imageFilename();
    QString makeTempImage();

public slots:
    void loadImage(QString file);
    void offsetImage(QPoint topLeft);

signals:
    void imageSizeChanged(QSize size);
    void wheelHorizontal(int delta);
    void wheelVertical(int delta);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);

private:
    void updateSelection();

    QString imageFile;
    QPixmap image;
    QPoint imageOffset;
    QRect selectionRect;
    QSize glSize;

    QPoint mp, mp0, mp1;
};

#endif // PREVIEWWIDGET_H
