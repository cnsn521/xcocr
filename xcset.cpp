#include "xcset.h"
#include "./ui_xcset.h"
static HHOOK hHook_eTrust;
// 记录当前已经按下的键的虚拟键码
std::vector<BYTE> pressedKeys;
xcset *cs_window = nullptr;
int Shortcut_1, Shortcut_2;
xcset::xcset(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::xcset)
{
    ui->setupUi(this);
    cs_window = this;
    Shortcut_1 = VK_F4;
    Shortcut_2 = 0;
    access_token_time = 0;
    access_token_time_ts = 0;
    is_close = true;
    baidu_ocr_key_own = baidu_ocr_key;
    baidu_ocr_select_own = baidu_ocr_select;
    baidu_ts_key_own = baidu_ts_key;
    baidu_ts_select_own = baidu_ts_select;
    ot_cnsn = new ocrts();
    //初始化半透明遮罩
    basewidget = new Shadewidget;
    basewidget->setParent(this, Qt::FramelessWindowHint | Qt::Window);
    basewidget->setWindowFlags(basewidget->windowFlags() | Qt::WindowStaysOnTopHint);
    basewidget->setGeometry(0, 0, 1, 1); //遮罩窗口位置
    basewidget->show();
    basewidget->raise();
    basewidget->hide();

    connect(basewidget, &Shadewidget::getPicSucess, [=](QImage IMG) {
        ot_cnsn->show();
        QByteArray imagesbase64 = imageToBase64(IMG);
        //        qDebug() << imagesbase64;
        QString ocr_result = imageToQstring(imagesbase64);
        ot_cnsn->setocrText(ocr_result);
    });
    connect(ot_cnsn, &ocrts::gogots, [=](QString str_ocr) {
        QString ts_result = tsLocalTotsTarget(str_ocr);
        ot_cnsn->settsText(ts_result);
    });
    QScreen *creen = QGuiApplication::primaryScreen();

    pNetManager = new QNetworkAccessManager();
    pNetManager1 = new QNetworkAccessManager();
    pNetManager->setAutoDeleteReplies(true);
    pNetManager->setTransferTimeout(10000);
    pNetManager1->setAutoDeleteReplies(true);
    pNetManager1->setTransferTimeout(10000);

    // 托盘
    m_systemTray = new QSystemTrayIcon(this);
    m_systemTray->setIcon(QIcon(":/favicon.ico"));
    m_systemTray->setToolTip("xcOCR");

    m_menu = new QMenu(this);

    m_action1 = new QAction(m_menu);
    m_action2 = new QAction(m_menu);
    m_action3 = new QAction(m_menu);
    m_action4 = new QAction(m_menu);

    m_action1->setText("截屏翻译");
    m_action2->setText("打开设置");
    m_action3->setText("更新");
    m_action4->setText("退出程序");
    connect(m_systemTray,
            &QSystemTrayIcon::activated,
            this,
            [=](QSystemTrayIcon::ActivationReason reason) {
                switch (reason) {
                case QSystemTrayIcon::Trigger:
                    /* 双击托盘显示窗口 */
                case QSystemTrayIcon::DoubleClick:
                    /* 显示正常大小 */
                    basewidget->show();
                    basewidget->setGeometry(0,
                                            0,
                                            creen->size().width(),
                                            creen->size().width()); //遮罩窗口位置
                    /* 将定时器关闭，图标设置为正常 */
                    break;
                default:
                    break;
                }
            });
    connect(m_action1, &QAction::triggered, this, [=]() {
        basewidget->show();
        basewidget->setGeometry(0, 0, creen->size().width(),
                                creen->size().width()); //遮罩窗口位置
    });
    connect(m_action2, &QAction::triggered, this, [=] { show(); });
    connect(m_action3, &QAction::triggered, this, [=] {

    });
    connect(this, &xcset::globalchortcut, [=]() {
        basewidget->show();
        basewidget->setGeometry(0, 0, creen->size().width(),
                                creen->size().width()); //遮罩窗口位置
    });
    connect(m_action4, &QAction::triggered, this, [=] {
        is_close = false;
        this->close();
    });
    m_menu->addAction(m_action1);
    m_menu->addAction(m_action2);
    m_menu->addAction(m_action3);
    m_menu->addAction(m_action4);
    m_systemTray->setContextMenu(m_menu);
    m_systemTray->show();
    tim_access_token_timeout = new QTimer();
    connect(tim_access_token_timeout, &QTimer::timeout, [&]() {
        if (access_token_time == 0)
            getBaiduAccessToken();
        access_token_time--;
        if (access_token_time_ts == 0)
            getBaiduTsAccessToken();
        access_token_time_ts--;
    });
    tim_access_token_timeout->start(1000);
    setF4();
}

xcset::~xcset()
{
    delete m_systemTray;
    delete m_menu;
    delete m_action1;
    delete m_action2;
    delete m_action3;
    delete m_action4;
    delete basewidget;
    delete ot_cnsn;
    delete pNetManager;
    delete pNetManager1;
    delete tim_access_token_timeout;
    delete ui;
    unsetF4();
}

void xcset::on_btn_ok_clicked() {}

void xcset::on_btn_reset_clicked() {}

QByteArray xcset::imageToBase64(QImage &img)
{
    if (img.isNull())
        return nullptr;
    QByteArray ba, imageBase;
    QBuffer buf(&ba);
    img.save(&buf, "png");
    imageBase = ba.toBase64();
    buf.close();
    return imageBase;
}

QString xcset::imageToQstring(QByteArray &imgBase64)
{
    if (access_token.isEmpty() && access_token_error.isEmpty()) {
        return "access_token is empty";
    } else if (access_token.isEmpty() && !access_token_error.isEmpty()) {
        return "access_token:" + access_token_error;
    }
    if (imgBase64.isEmpty())
        return "图片base64未空值!";
    QUrl url;
    url.setUrl("https://aip.baidubce.com/rest/2.0/ocr/v1/general_basic?access_token="
               + access_token);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    //    QJsonObject json;
    //    json.insert("image", QString::fromLatin1(imgBase64.toPercentEncoding()));
    //    // 构建 JSON 文档
    //    QJsonDocument document;
    //    document.setObject(json);
    //    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QByteArray byteArray = "image=" + imgBase64.toPercentEncoding();
    QEventLoop eventloop;
    QString ocrresult;
    QNetworkReply *reply = pNetManager->post(request, byteArray);
    QObject::connect(reply, &QNetworkReply::finished, this, [&]() {
        QNetworkReply::NetworkError err = reply->error();
        if (err != QNetworkReply::NoError) {
            ocrresult = "获取配置请求出现错误: " + reply->readAll();
        } else {
            // 获取内容

            QString rP_a = reply->readAll();
            QJsonParseError parseJsonErr;
            QJsonDocument document = QJsonDocument::fromJson(rP_a.toUtf8(), &parseJsonErr);
            if (!(parseJsonErr.error == QJsonParseError::NoError)) {
                ocrresult = "获取配置请求出现错误: " + parseJsonErr.errorString();
            }

            QJsonObject jsonObject = document.object();
            qDebug() << jsonObject;
            if (jsonObject.contains("words_result_num")) {
                int reault_num = jsonObject.value("words_result_num").toInt();
                if (reault_num <= 0) {
                    ocrresult = "未检测到文字或者检查接口是否正确";
                }
                //   ocr_str_language = jsonObject.value("language").toString();
                QJsonArray wordsResultArray = jsonObject.value("words_result").toArray();
                for (int i = 0; i < reault_num; ++i) {
                    QJsonObject wordsResultObj = wordsResultArray.at(i).toObject();
                    QString words = wordsResultObj["words"].toString();
                    ocrresult.append(words + "  ");
                }
            } else if (jsonObject.contains("error_msg")) {
                ocrresult = jsonObject.value("error_msg").toString();
            } else {
                ocrresult = "未知错误！";
            }
        }
        eventloop.quit();
    });
    eventloop.exec();
    return ocrresult;
}

QString xcset::tsLocalTotsTarget(QString &strLocal)
{
    if (access_token_ts.isEmpty() && access_token_error_ts.isEmpty()) {
        return "access_token is empty";
    } else if (access_token_ts.isEmpty() && !access_token_error_ts.isEmpty()) {
        return access_token_error_ts;
    }
    QUrl url;
    url.setUrl("https://aip.baidubce.com/rpc/2.0/mt/texttrans/v1?access_token=" + access_token_ts);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject json;
    json.insert("q", strLocal);
    json.insert("from", "auto");
    json.insert("to", "zh");
    // 构建 JSON 文档
    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QEventLoop eventloop;
    QString tsresult;
    QNetworkReply *reply = pNetManager->post(request, byteArray);
    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        QNetworkReply::NetworkError err = reply->error();
        if (err != QNetworkReply::NoError) {
            tsresult = "获取配置请求出现错误: " + reply->readAll();
            return;
        } else {
            // 获取内容

            QString rP_a = reply->readAll();
            QJsonParseError parseJsonErr;
            QJsonDocument document = QJsonDocument::fromJson(rP_a.toUtf8(), &parseJsonErr);
            if (!(parseJsonErr.error == QJsonParseError::NoError)) {
                tsresult = "获取配置请求出现错误: " + parseJsonErr.errorString();
                return;
            }
            QJsonObject jsonObject = document.object();
            qDebug() << jsonObject;
            if (jsonObject.contains("result")) {
                QJsonObject jsonvalue = jsonObject.value("result").toObject();
                QJsonArray wordsResultArray = jsonvalue.value("trans_result").toArray();
                for (int i = 0; i < wordsResultArray.size(); ++i) {
                    QJsonObject wordsResultObj = wordsResultArray.at(i).toObject();
                    QString dst = wordsResultObj["dst"].toString();
                    tsresult.append(dst + "  ");
                }
            } else if (jsonObject.contains("error_code")) {
                int error_code = jsonObject.value("error_code").toInt();
                tsresult = ts_error_result(error_code);
            } else
                tsresult = "未检测到文字或者检查接口是否正确";
        }
        eventloop.quit();
    });
    eventloop.exec();
    return tsresult;
}

void xcset::getBaiduAccessToken()
{
    access_token.clear();
    access_token_error.clear();
    QUrl url;
    url.setUrl(QString("https://aip.baidubce.com/oauth/2.0/"
                       "token?grant_type=client_credentials&client_id=%1&client_secret=%2")
                   .arg(baidu_ocr_key_own)
                   .arg(baidu_ocr_select_own));
    QNetworkRequest request;
    request.setUrl(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");

    QEventLoop eventloop;
    QString ocrresult;
    QNetworkReply *reply = pNetManager->post(request, "");
    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        QNetworkReply::NetworkError err = reply->error();
        if (err != QNetworkReply::NoError) {
            qDebug() << "获取配置请求出现错误: " << reply->readAll();
        } else {
            // 获取内容

            QString rP_a = reply->readAll();
            QJsonParseError parseJsonErr;
            QJsonDocument document = QJsonDocument::fromJson(rP_a.toUtf8(), &parseJsonErr);
            if (!(parseJsonErr.error == QJsonParseError::NoError)) {
                qDebug() << "获取配置请求出现错误: " << parseJsonErr.errorString();
                return;
            }
            QJsonObject jsonObject = document.object();
            if (jsonObject.contains("access_token")) {
                QJsonValue arrayValue = jsonObject.value("access_token");
                QJsonValue arrayValue1 = jsonObject.value("expires_in");
                access_token = arrayValue.toString();
                access_token_time = arrayValue1.toInt();
            } else if (jsonObject.contains("error_msg")) {
                QJsonValue arrayValue = jsonObject.value("error_description");
                QString err_or = arrayValue.toString();
                if (err_or.contains("unknown client id"))
                    access_token_error = "API Key不正确";
                else if (err_or.contains("Client authentication failed"))
                    access_token_error = "Secret Key不正确";
                else
                    access_token_error = "未知错误";
            } else {
                access_token_error = "未知错误";
            }
        }
        eventloop.quit();
    });
    eventloop.exec();
}

void xcset::getBaiduTsAccessToken()
{
    access_token_ts.clear();
    access_token_error_ts.clear();
    QUrl url;
    url.setUrl(QString("https://aip.baidubce.com/oauth/2.0/"
                       "token?grant_type=client_credentials&client_id=%1&client_secret=%2")
                   .arg(baidu_ts_key_own)
                   .arg(baidu_ts_select_own));
    QNetworkRequest request;
    request.setUrl(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");

    QEventLoop eventloop;
    //    QString ocrresult;
    QNetworkReply *reply = pNetManager->post(request, "");
    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        QNetworkReply::NetworkError err = reply->error();
        if (err != QNetworkReply::NoError) {
            qDebug() << "获取配置请求出现错误: " << reply->readAll();
        } else {
            // 获取内容

            QString rP_a = reply->readAll();
            QJsonParseError parseJsonErr;
            QJsonDocument document = QJsonDocument::fromJson(rP_a.toUtf8(), &parseJsonErr);
            if (!(parseJsonErr.error == QJsonParseError::NoError)) {
                qDebug() << "获取配置请求出现错误: " << parseJsonErr.errorString();
                return;
            }
            QJsonObject jsonObject = document.object();
            if (jsonObject.contains("access_token")) {
                QJsonValue arrayValue = jsonObject.value("access_token");
                QJsonValue arrayValue1 = jsonObject.value("expires_in");
                access_token_ts = arrayValue.toString();
                access_token_time_ts = arrayValue1.toInt();
            } else if (jsonObject.contains("error_msg")) {
                QJsonValue arrayValue = jsonObject.value("error_description");
                QString err_or = arrayValue.toString();
                if (err_or.contains("unknown client id"))
                    access_token_error_ts = "API Key不正确";
                else if (err_or.contains("Client authentication failed"))
                    access_token_error_ts = "Secret Key不正确";
                else
                    access_token_error_ts = "未知错误";
            } else {
                access_token_error_ts = "未知错误";
            }
        }
        eventloop.quit();
    });
    eventloop.exec();
}

QString xcset::ts_error_result(int int_result)

{
    QString str_result;
    switch (int_result) {
    case 31001:
        str_result = "其他错误,请重试";
        break;
    case 31005:
        str_result = "用户用量超限,请检查当前可用字符/次数包额度";
        break;
    case 31006:
        str_result = "内部错误,请重试";
        break;
    case 31101:
        str_result = "请求超时,请重试";
        break;
    case 31102:
        str_result = "系统错误,请重试";
        break;
    case 31103:
        str_result = "必填参数为空或固定参数有误,请检查参数是否为空或误传";
        break;
    case 31104:
        str_result = "访问频率受限,请降低您的调用频率";
        break;
    case 31105:
        str_result = "译文语言方向不支持,请检查译文语言是否在语言列表里";
        break;
    case 31106:
        str_result = "query字符超过最大长度,请减少翻译原文的长度（最长不得超过6000字节）";
        break;
    case 31201:
        str_result = "请求翻译的原文太长,请减少翻译原文的长度（最长不得超过6000字节）";
        break;
    case 31202:
        str_result = "请求翻译的原文为空,请检查翻译原文内容是否为空";
        break;
    case 31203:
        str_result = "请求翻译的参数有误(目前校验header/"
                     "param中鉴权必要参数不能为空),请检查参数是否为空或误传";
        break;
    case 282000:
        str_result = "内部错误,请重试";
        break;
    case 282003:
        str_result = "请求翻译时存在必填参数为空,请检查必填参数（比如q、from、to等）是否为空";
        break;
    case 282004:
        str_result = "请求翻译时请求体无法解析,请检查请求体格式等是否正确";
        break;
    default:
        str_result = "其他错误,请重试";
        break;
    }
    return str_result;
}
LRESULT CALLBACK KeyboardHookProc1(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0) {
        return CallNextHookEx(hHook_eTrust, nCode, wParam, lParam);
    }
    try {
        // 解析键盘消息结构体
        KBDLLHOOKSTRUCT *kbStruct = (KBDLLHOOKSTRUCT *) lParam;

        //  // 判断是否收到按键按下或松开消息
        //  if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN || wParam ==
        //  WM_KEYUP ||
        //      wParam == WM_SYSKEYUP) {
        // 获取虚拟键码
        BYTE vkCode = (BYTE) (kbStruct->vkCode);

        // 更新按键状态列表
        if (Shortcut_2 != 0) {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                pressedKeys.push_back(vkCode);
            } else {
                for (auto it = pressedKeys.begin(); it != pressedKeys.end(); ++it) {
                    if (*it == vkCode) {
                        pressedKeys.erase(it);
                        break;
                    }
                }
            }

            // 检查组合键状态
            bool isCtrlPressed = false;
            bool isShiftPressed = false;
            for (auto it = pressedKeys.begin(); it != pressedKeys.end(); ++it) {
                if (*it == VK_LWIN) {
                    isCtrlPressed = true;
                } else if (*it == 0x4C) {
                    isShiftPressed = true;
                } else {
                    isShiftPressed = false;
                    isCtrlPressed = false;
                }
            }
            if (isCtrlPressed && isShiftPressed && (vkCode == Shortcut_1 || vkCode == Shortcut_2)) {
                emit cs_window->globalchortcut();
                qDebug() << "WINDOW + L 组合键被按下!";
            }
        } else {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                if (vkCode == Shortcut_1)
                    emit cs_window->globalchortcut();
            }
        }
        //  }
    } catch (const char *&e) {
        qDebug() << QString("获取按键异常，异常代码:%1").arg(e);
    }
    // 继续传递消息
    return CallNextHookEx(hHook_eTrust, nCode, wParam, lParam);
}

HMODULE ModuleFromAddress1(PVOID pv)
{
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) {
        return (HMODULE) mbi.AllocationBase;
    } else {
        return NULL;
    }
}
void xcset::setF4()
{
    if (hHook_eTrust) {
        qDebug() << "hook is installing";
        return;
    }
    hHook_eTrust = SetWindowsHookExW(WH_KEYBOARD_LL,
                                     KeyboardHookProc1,
                                     ModuleFromAddress1((PVOID) KeyboardHookProc1),
                                     0);
    int error = GetLastError();
    if (error != 0)
        qDebug() << "install hook fail!";
    else
        qDebug() << "install hook sucess!";
}
void xcset::unsetF4()
{
    if (hHook_eTrust)
        UnhookWindowsHookEx(hHook_eTrust);
}

void xcset::closeEvent(QCloseEvent *event)
{
    if (is_close) {
        event->ignore();
        this->hide();
    }
}
