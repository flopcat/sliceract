#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include "widget.h"
#include "ui_widget.h"
#include "previewwidget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    preview = new PreviewWidget;
    ui->previewHostLayout->addWidget(preview);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_sourceBrowse_clicked()
{
    static QString oldDir;
    QString fname = QFileDialog::getOpenFileName(this, "Select Image", oldDir, "Images (*.png *.jpg *.bmp)");
    if (fname.isEmpty())
        return;
    oldDir = QFileInfo(fname).absoluteDir().dirName();
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
