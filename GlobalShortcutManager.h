#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QDebug>
#include <QKeySequence>
#include <windows.h>

class GlobalShortcutManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    GlobalShortcutManager(QObject *parent = nullptr)
        : QObject(parent)
    {
        // 绑定全局快捷键
        RegisterHotKey(nullptr, 1, MOD_CONTROL, 'T');
    }

    ~GlobalShortcutManager()
    {
        // 释放全局快捷键
        UnregisterHotKey(nullptr, 1);
    }

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result)
    {
        // 处理 Windows 消息
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY && msg->wParam == 1) {
            qDebug() << "全局快捷键触发！";
            return true;
        }

        return false;
    }
};
