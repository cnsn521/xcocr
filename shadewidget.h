#pragma once

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QWidget>
#include <QtDebug>
namespace Ui {
class Shadewidget;
}

class Shadewidget : public QWidget
{
    Q_OBJECT

public:
    explicit Shadewidget(QWidget *parent = nullptr);
    ~Shadewidget();
private slots:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent * /*event*/);

signals:
    void getPicSucess(QImage img);

private:
    Ui::Shadewidget *ui;
    QPixmap screen; //截屏图片
    QPoint start_point;
    QPoint end_point;
    int x, y, w, h;
};

