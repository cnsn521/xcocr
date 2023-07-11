#include "shadewidget.h"
#include "ui_shadewidget.h"

Shadewidget::Shadewidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Shadewidget)
{
    ui->setupUi(this);
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window, QColor(0, 0, 0));
    this->setPalette(palette);
    this->setWindowOpacity(0.4);                                   //设置窗口透明度
    this->setWindowFlags(Qt::FramelessWindowHint | windowFlags()); //去掉标题栏
}

Shadewidget::~Shadewidget()
{
    delete ui;
}

void Shadewidget::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
}
//实现窗口移动
void Shadewidget::mouseMoveEvent(QMouseEvent *event)
{
    end_point = event->globalPosition().toPoint() - this->pos(); //获取点击的坐标
    if (end_point.x() > start_point.x()) {
        x = start_point.x(); //比较起始坐标
        w = end_point.x() - start_point.x();
    } else {
        x = end_point.x();
        w = start_point.x() - end_point.x();
    }
    if (end_point.y() > start_point.y()) {
        y = start_point.y(); //比较起始坐标
        h = end_point.y() - start_point.y();
    } else {
        y = end_point.y();
        h = start_point.y() - end_point.y();
    }
    update();
    //    qDebug() << start_point << end_point << event->globalPos() - this->pos(); //获取点击的坐标
}
//鼠标点击事件
void Shadewidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) { //右键退出截屏
        this->setGeometry(0, 0, 1, 1);        //遮罩窗口位置
                                              //关闭之后清空
        x = y = w = h = 0;
        //        qDebug() << "start----";
        update();
        this->hide();
    }
    x = y = w = h = 0;
    update();
    start_point = event->globalPos() - this->pos(); //获取点击的坐标
}
//鼠标释放事件
void Shadewidget::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "111" << event->x() << x;
    if (event->x() >= x && event->x() <= (x + w) && event->y() >= y
        && event->y() <= (y + h)) { //画完范围确定截图
        //截屏
        this->setWindowOpacity(0); //设置窗口透明度
        //        screen = QScreen::grabWindow(QGuiApplication::primaryScreen()->handle(), x, y, w, h);
        // 获取默认屏幕对象
        QScreen *screen_image = QGuiApplication::primaryScreen();

        // 通过QScreen::grabWindow()捕获整个屏幕的内容
        QImage fullScreenImage = screen_image->grabWindow(0).toImage();

        // 定义捕获区域（例如：x=100, y=100, width=200, height=200）
        QRect captureRect(x, y, w, h);

        // 使用QImage的裁剪函数来截取所需区域
        QImage croppedImage = fullScreenImage.copy(captureRect);
        this->setWindowOpacity(0.4); //设置窗口透明度

        this->setGeometry(0, 0, 1, 1); //遮罩窗口位置
        //关闭之后清空

        x = y = w = h = 0;
        getPicSucess(croppedImage);
        update();
        this->hide();
    } else {
        x = y = w = h = 0;
        update();
        start_point = event->globalPos() - this->pos(); //获取点击的坐标
                                                        //        qDebug() << "stop----";
    }
    //    qDebug() << start_point; //获取点击的坐标
}
void Shadewidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    painter.save();
    painter.setPen(Qt::green);
    QPointF topLeftPot(x, y);
    QPointF bottomRightPot(x + w, y + h); //右上角
    QRectF barRect = QRectF(topLeftPot, bottomRightPot);
    painter.setBrush(Qt::white);
    painter.drawRect(barRect);
    painter.restore();
}
