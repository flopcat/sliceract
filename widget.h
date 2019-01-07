#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

class PreviewWidget;
class QProcess;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_sourceBrowse_clicked();
    void on_sourceLoad_clicked();
    void on_sliceToClipboard_clicked();
    void process_finished(QString infile, QString outfile);

private:
    Ui::Widget *ui = nullptr;
    PreviewWidget *preview = nullptr;
    QProcess *process = nullptr;
};

#endif // WIDGET_H
