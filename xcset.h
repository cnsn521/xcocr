#pragma once

#include <QAction>
#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QShortcut>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrlQuery>
#include <QWidget>
#include "ocrts.h"
#include "shadewidget.h"
#include <windows.h>
QT_BEGIN_NAMESPACE
namespace Ui { class xcset; }
QT_END_NAMESPACE
#define baidu_ocr_key "GqQK6pDuIwF3mFlMCe23XiMH"
#define baidu_ocr_select "cd3t2RF9GzWifgVaZHZP0PGVZiUeyj2F"
#define baidu_ts_key "Cca1M0uF57GVsroGtNdtFuXc"
#define baidu_ts_select "7d4zC0rXDlqSO2dZ0G1KrnTqZEky5QPh"

class xcset : public QWidget
{
    Q_OBJECT

public:
    xcset(QWidget *parent = nullptr);
    ~xcset();

private slots:
    void on_btn_ok_clicked();

    void on_btn_reset_clicked();

    QByteArray imageToBase64(QImage &img);

    QString imageToQstring(QByteArray &imgBase64);

    QString tsLocalTotsTarget(QString &strLocal);

    void getBaiduAccessToken();
    void getBaiduTsAccessToken();

    QString ts_error_result(int int_result);
    void setF4();
    void unsetF4();

    void closeEvent(QCloseEvent *event);
signals:
    void globalchortcut();

private:
    Ui::xcset *ui;
    QSystemTrayIcon *m_systemTray;
    QMenu *m_menu;
    QAction *m_action1, *m_action2, *m_action3, *m_action4;
    Shadewidget *basewidget;
    ocrts *ot_cnsn;
    QNetworkAccessManager *pNetManager;
    QNetworkAccessManager *pNetManager1;
    QString access_token, access_token_error;
    int access_token_time;
    QString access_token_ts, access_token_error_ts;
    int access_token_time_ts;
    QTimer *tim_access_token_timeout;
    QString baidu_ocr_key_own, baidu_ocr_select_own;
    QString baidu_ts_key_own, baidu_ts_select_own;
    bool is_close;
    //  QString ocr_str_language;
};
