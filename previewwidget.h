#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H
#include <QOpenGLWidget>
#include <QPixmap>

class PreviewWidget : public QOpenGLWidget
{
public:
    PreviewWidget();
    QRect imageSelection();
    QString imageFilename();
    QString makeTempImage();

public slots:
    void loadImage(QString file);

protected:
    void resizeGL(int w, int h);
    void paintGL();
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

private:
    QPoint drawnToImage(QPoint p);
    void updateSelection();

    QString imageFile;
    QPixmap image;
    QPixmap cachedPixmap;
    QRect selectionRect;
    QSize glSize;

    QPoint drawnPoint;
    QSize drawnSize;

    QPoint mp, mp0, mp1;
};

#endif // PREVIEWWIDGET_H
