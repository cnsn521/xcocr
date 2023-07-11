#pragma once

#include <QCloseEvent>
#include <QScreen>
#include <QWidget>
namespace Ui {
class ocrts;
}

class ocrts : public QWidget
{
    Q_OBJECT

public:
    explicit ocrts(QWidget *parent = nullptr);
    ~ocrts();
    void setocrText(QString str_ocr);
    void settsText(QString str_ts);
private slots:
    void on_btn_ts_clicked();
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);
signals:
    void gogots(QString str_ocr);

private:
    Ui::ocrts *ui;
    QScreen *screen_image;
    QRect ocrt_rect;
};

