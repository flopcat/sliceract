#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QScrollBar>
#include "widget.h"
#include "ui_widget.h"
#include "previewwidget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    preview = new PreviewWidget();
    ui->previewHostLayout->addWidget(preview);

    int step = QFontMetrics(preview->font()).height();
    ui->horizontalScrollBar->setSingleStep(step);
    ui->verticalScrollBar->setSingleStep(step);

    connect(preview, &PreviewWidget::imageSizeChanged,
            this, &Widget::preview_imageSizeChanged);
    connect(preview, &PreviewWidget::wheelHorizontal,
            this, &Widget::preview_wheelHorizontal);
    connect(preview, &PreviewWidget::wheelVertical,
            this, &Widget::preview_wheelVertical);
    connect(ui->horizontalScrollBar, &QScrollBar::valueChanged,
            this, &Widget::scrollArea_offsetChanged);
    connect(ui->verticalScrollBar, &QScrollBar::valueChanged,
            this, &Widget::scrollArea_offsetChanged);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev);
    preview_imageSizeChanged();
}

void Widget::on_sourceBrowse_clicked()
{
    static QString oldDir;
    QString fname = QFileDialog::getOpenFileName(this, "Select Image", oldDir, "Images (*.png *.jpg *.bmp)");
    if (fname.isEmpty())
        return;
    auto oldDirSplit = fname.split('/');
    oldDirSplit.takeLast();
    oldDir = oldDirSplit.join('/');
    ui->source->setText(fname);
}

void Widget::on_sourceLoad_clicked()
{
    preview->loadImage(ui->source->text());
}

QString makeTempImage(QString filename, QRect selection)
{
    QString outname = QString("/tmp/sliceract.input.%1.png").arg(QCoreApplication::applicationPid());
    QImage source;
    if (!source.load(filename))
        return QString();
    QImage destination = source.copy(selection);
    destination.save(outname);
    return QString(outname);
}

void Widget::on_sliceToClipboard_clicked()
{
    if (process)
        return;
    QString tempImage = makeTempImage(preview->imageFilename(), preview->imageSelection());
    QString outfile = QString("/tmp/sliceract.output.%1").arg(QCoreApplication::applicationPid());
    process = new QProcess();
    process->setProgram("tesseract");
    process->setArguments({tempImage, outfile, "--psm", "4"});
    connect(process,QOverload<int>::of(&QProcess::finished),
            this, [tempImage,outfile,this](int) -> void {
        process_finished(tempImage, outfile + ".txt");
    });
    process->start();
    ui->status->setText("Running");
}

void Widget::preview_imageSizeChanged()
{
    QSize imageSize = preview->imageSize();
    QSize previewSize = preview->size();
    QSize scrollSize = imageSize - previewSize;
    ui->horizontalScrollBar->setRange(0, std::max(scrollSize.width(), 0));
    ui->verticalScrollBar->setRange(0, std::max(scrollSize.height(), 0));
    ui->horizontalScrollBar->setPageStep(preview->width());
    ui->verticalScrollBar->setPageStep(preview->height());
}

void Widget::preview_wheelHorizontal(int delta)
{
    int p = ui->horizontalScrollBar->sliderPosition();
    ui->horizontalScrollBar->setSliderPosition(p + delta);
}

void Widget::preview_wheelVertical(int delta)
{
    int p = ui->verticalScrollBar->sliderPosition();
    ui->verticalScrollBar->setSliderPosition(p + delta);
}

void Widget::process_finished(QString infile, QString outfile)
{
    delete process;
    process = nullptr;
    QFile in(infile);
    QFile out(outfile);
    in.remove();
    if (!out.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Could not open tesseract output",
                             "Perhaps the process failed.");
        return;
    }
    QByteArray data = out.readAll();
    QApplication::clipboard()->setText(QString::fromUtf8(data));
    out.remove();
    ui->status->setText("");
}

void Widget::scrollArea_offsetChanged()
{
    preview->offsetImage({ ui->horizontalScrollBar->value(),
                           ui->verticalScrollBar->value() });
}
