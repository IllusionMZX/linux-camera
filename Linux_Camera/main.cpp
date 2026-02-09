#include "mainwidget.h"
#include <QApplication>
#include <QNetworkProxy>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 设置 HTTP 代理
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);   // 使用 HTTP Proxy
    proxy.setHostName("192.168.137.1");            // Clash 本地监听地址
    proxy.setPort(7890);                       // Clash 默认 HTTP 代理端口
    QNetworkProxy::setApplicationProxy(proxy);

    MainWidget w;
    w.show();
    return a.exec();
}
