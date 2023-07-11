#include "xcset.h"

#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    xcset w;
    QSharedMemory shared("apa");
    if (shared.attach()) //共享内存被占用则直接返回
    {
        QMessageBox::information(NULL,
                                 QStringLiteral("Warning"),
                                 QStringLiteral("Application is alreadly running!"));
        return 0;
    }
    shared.create(1); //共享内存没有被占用则创建UI
    //    w.show();
    return a.exec();
}
