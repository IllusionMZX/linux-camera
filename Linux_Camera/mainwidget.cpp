#include "mainwidget.h"
#include "Dashboard/widget.h"
#include "UVC/processImage.h"
#include "QMapDemo/multidemo.h"
#include "IMUProcess/IMUProcess.h"
#include "GPSProcess/GPSProcess.h"
#include "Weather/mainwindow.h"
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QFont>
#include <QPixmap>
#include <QPalette>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(480, 272);

    // 设置背景颜色为白色
    QPalette palette = this->palette();
    palette.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(palette);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 添加标题
    QLabel *title = new QLabel("M_CarBoard", this);
    QFont titleFont("Consolas", 18);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // 设置按钮布局
    QGridLayout *buttonLayout = new QGridLayout();
    buttonLayout->setSpacing(20);

    QStringList buttonIcons = {
        "./Dashboard/dashboard.png",
        "./Dashboard/camera.png",
        "./Dashboard/map.png",
        "./Dashboard/imu.png",
        "./Dashboard/locate.png",
        "./Dashboard/weather.png"
    };

    for (int i = 0; i < 6; ++i) {
        btns[i] = new QPushButton(this);

        // 设置按钮大小为正方形
        btns[i]->setFixedSize(80, 80);

        // 设置按钮为圆角
        btns[i]->setStyleSheet("QPushButton { border-radius: 10px; border: 2px solid black; }");

        // 设置按钮图标
        QPixmap pixmap(buttonIcons[i]);
        QIcon buttonIcon(pixmap);
        btns[i]->setIcon(buttonIcon);
        btns[i]->setIconSize(QSize(60, 60));

        int row = i / 3;
        int col = i % 3;
        buttonLayout->addWidget(btns[i], row, col);
    }

    mainLayout->addLayout(buttonLayout);

    // 只示例第一个按钮进入仪表盘，其它可根据需求连接
    connect(btns[0], &QPushButton::clicked, this, &MainWidget::openDashboard);
    connect(btns[1], &QPushButton::clicked, this, &MainWidget::openCamera);
    connect(btns[2], &QPushButton::clicked, this, &MainWidget::openMap);
    connect(btns[3], &QPushButton::clicked, this, &MainWidget::openIMU);
    connect(btns[4], &QPushButton::clicked, this, &MainWidget::openGPS);
    connect(btns[5], &QPushButton::clicked, this, &MainWidget::openWeather);

}

void MainWidget::openDashboard()
{
    Widget *w = new Widget();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
    this->hide();
    QObject::connect(w, &Widget::backToMenu, [this, w]() {
        w->close();
        this->show();
    });
}

void MainWidget::openCamera()
{
    ProcessImage *w = new ProcessImage();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
    this->hide();
    QObject::connect(w, &ProcessImage::backToMenu, [this, w]() {
        w->close();
        this->show();
    });
}

void MainWidget::openMap()
{
    Multidemo *w = new Multidemo();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
    w->resize(480, 270);
    this->hide();
    QObject::connect(w, &Multidemo::backToMenu, [this, w]() {
        w->close();
        this->show();
    });
}

void MainWidget::openIMU()
{
    IMUProcess *w = new IMUProcess();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
    this->hide();
    QObject::connect(w, &IMUProcess::backToMenu, [this, w]() {
        w->close();
        this->show();
    });
}

void MainWidget::openGPS()
{
    GPSProcess *w = new GPSProcess();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
    this->hide();
    QObject::connect(w, &GPSProcess::backToMenu, [this, w]() {
        w->close();
        this->show();
    });
}

void MainWidget::openWeather()
{
    MainWindow *w = new MainWindow();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
    this->hide();
    QObject::connect(w, &MainWindow::backToMenu, [this, w]() {
        w->close();
        this->show();
    });
}
