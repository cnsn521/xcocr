#include "ocrts.h"
#include "ui_ocrts.h"

ocrts::ocrts(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ocrts)
{
    ui->setupUi(this);
    ocrt_rect = this->rect();
    screen_image = QGuiApplication::primaryScreen();
    this->setGeometry((screen_image->geometry().width() - ocrt_rect.width()) * 0.5,
                      (screen_image->geometry().height() - ocrt_rect.height()) * 0.5,
                      ocrt_rect.width() * 0.5,
                      ocrt_rect.height());
}

ocrts::~ocrts()
{
    delete ui;
    delete screen_image;
}

void ocrts::setocrText(QString str_ocr)
{
    ui->textEdit_0cr->setText(str_ocr);
}

void ocrts::settsText(QString str_ts)
{
    ui->textEdit_ts->setText(str_ts);
}

void ocrts::on_btn_ts_clicked()
{
    this->setGeometry((screen_image->geometry().width() - ocrt_rect.width()) * 0.5,
                      (screen_image->geometry().height() - ocrt_rect.height()) * 0.5,
                      ocrt_rect.width(),
                      ocrt_rect.height());
    emit gogots(ui->textEdit_0cr->toPlainText());
}

void ocrts::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
}

void ocrts::showEvent(QShowEvent *event)
{
    this->setGeometry((screen_image->geometry().width() - ocrt_rect.width()) * 0.5,
                      (screen_image->geometry().height() - ocrt_rect.height()) * 0.5,
                      ocrt_rect.width() * 0.5,
                      ocrt_rect.height());
    ui->textEdit_0cr->clear();
    ui->textEdit_ts->clear();
}
