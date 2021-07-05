#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QScrollBar>
#include <QStandardPaths>
#include "widget.h"
#include "ui_widget.h"
#include "previewwidget.h"

const char tesseractBinary[] =
#if defined(Q_OS_WIN)
    "C:\\Program Files\\Tesseract-OCR\\tesseract.exe"
#else
    "tesseract"
#endif
;



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

    QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString appPid = QString::number(QCoreApplication::applicationPid());
    tempImageFileName = QString("%1/sliceract.input.%2.png").arg(tmpDir, appPid);
    tempTextFileName = QString("%1/sliceract.output.%2").arg(tmpDir, appPid);
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

bool Widget::makeTempImage(QString filename, QRect selection)
{
    QImage source;
    if (!source.load(filename))
        return false;
    QImage destination = source.copy(selection);
    bool success = destination.save(tempImageFileName);
    return success;
}

void Widget::on_sliceToClipboard_clicked()
{
    if (process)
        return;
    ui->status->setText("Slicing");
    qApp->processEvents(QEventLoop::AllEvents, 5);
    if (!makeTempImage(preview->imageFilename(), preview->imageSelection())) {
        ui->status->setText("");
        return;
    }
    process = new QProcess();
    process->setProgram(tesseractBinary);
    process->setArguments({tempImageFileName, tempTextFileName, "--psm", QString::number(1)});
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Widget::process_finished);
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

void Widget::process_finished()
{
    if (process) {
        delete process;
        process = nullptr;
    }
    QFile in(tempImageFileName);
    QFile out(tempTextFileName + ".txt");
    if (!in.remove()) {
        QMessageBox::warning(this, "Could not remove the input image",
                             "Perhaps the data is wrong");
    }
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
