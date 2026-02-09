#include <QWidget>
#include <QLabel>
#include <QImage>
#include <QHBoxLayout>
#include <QPainter>
#include <QTimer>
#include <QPushButton>
#include <QMessageBox>
#include "processImage.h"
#include "videodevice.h"

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

ProcessImage::ProcessImage(QWidget *parent)
    : QWidget(parent)
{
    pp = (unsigned char *)malloc(640 * 480 * 3 * sizeof(char));
    frame = new QImage(pp, 640, 480, QImage::Format_RGB888); // 480,不是452
    label = new QLabel(this);
    // 设置背景颜色为白色
    QPalette palette = this->palette();
    palette.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(palette);

    vd = new VideoDevice(QStringLiteral("/dev/video4"));
    connect(vd, &VideoDevice::display_error, this, &ProcessImage::display_error);
    addBackButton();
    rs = vd->open_device();
    if(rs == -1) {
        QMessageBox::warning(this, tr("error"), tr("open /dev/dsp error"), QMessageBox::Yes);
        vd->close_device();
    }

    rs = vd->init_device();
    if(rs == -1) {
        QMessageBox::warning(this, tr("error"), tr("init failed"), QMessageBox::Yes);
        vd->close_device();
    }

    rs = vd->start_capturing();
    if(rs == -1) {
        QMessageBox::warning(this, tr("error"), tr("start capture failed"), QMessageBox::Yes);
        vd->close_device();
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&ProcessImage::update));
    timer->start(30);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(label);
    setLayout(hLayout);
    setWindowTitle(tr("iTOP-UVC-camera"));
}

ProcessImage::~ProcessImage()
{
    if(timer) timer->stop();
    if(vd) {
        vd->stop_capturing();
        vd->uninit_device();
        vd->close_device();
        delete vd;
    }
    if(frame) delete frame;
    if(pp) free(pp);
}

void ProcessImage::addBackButton()
{
    QPushButton *backBtn = new QPushButton("Back", this);
    backBtn->setGeometry(10, 200, 60, 32);

    // 设置按钮样式
    backBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: white;" // 白色底色
        "    color: black;"            // 黑色字体
        "    border: 2px solid black;" // 黑色边框
        "    border-radius: 8px;"      // 圆角半径
        "}"
        "QPushButton:hover {"
        "    background-color: #f0f0f0;" // 鼠标悬停时的背景颜色（浅灰）
        "}"
        "QPushButton:pressed {"
        "    background-color: #e0e0e0;" // 按下时的背景颜色（更深的灰色）
        "}"
    );

    connect(backBtn, &QPushButton::clicked, this, [this](){
        emit backToMenu();
    });
}

void ProcessImage::paintEvent(QPaintEvent *)
{
    rs = vd->get_frame((void **)&p, &len);
    convert_yuv_to_rgb_buffer(p, pp, 640, 480);

    // 直接使用数据，不用loadFromData
    // frame->loadFromData((uchar *)pp, 640 * 480 * 3 * sizeof(char)); // 不推荐
    *frame = QImage(pp, 640, 480, QImage::Format_RGB888); // 重新构造QImage指向数据

    label->setPixmap(QPixmap::fromImage(*frame, Qt::AutoColor));

    rs = vd->unget_frame();
}

void ProcessImage::display_error(QString err)
{
    QMessageBox::warning(this, tr("error"), err, QMessageBox::Yes);
}

int ProcessImage::convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
    unsigned int in, out = 0;
    unsigned int pixel_16;
    unsigned char pixel_24[3];
    unsigned int pixel32;
    int y0, u, y1, v;
    for(in = 0; in < width * height * 2; in += 4) {
        pixel_16 =
            yuv[in + 3] << 24 |
            yuv[in + 2] << 16 |
            yuv[in + 1] <<  8 |
            yuv[in + 0];
        y0 = (pixel_16 & 0x000000ff);
        u  = (pixel_16 & 0x0000ff00) >>  8;
        y1 = (pixel_16 & 0x00ff0000) >> 16;
        v  = (pixel_16 & 0xff000000) >> 24;
        pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
        pixel_24[0] = (pixel32 & 0x000000ff);
        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
        rgb[out++] = pixel_24[0];
        rgb[out++] = pixel_24[1];
        rgb[out++] = pixel_24[2];
        pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
        pixel_24[0] = (pixel32 & 0x000000ff);
        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
        rgb[out++] = pixel_24[0];
        rgb[out++] = pixel_24[1];
        rgb[out++] = pixel_24[2];
    }
    return 0;
}

int ProcessImage::convert_yuv_to_rgb_pixel(int y, int u, int v)
{
    unsigned int pixel32 = 0;
    unsigned char *pixel = (unsigned char *)&pixel32;
    int r, g, b;
    r = y + (1.370705 * (v-128));
    g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
    b = y + (1.732446 * (u-128));
    r = qBound(0, r, 255);
    g = qBound(0, g, 255);
    b = qBound(0, b, 255);
    pixel[0] = r * 220 / 256;
    pixel[1] = g * 220 / 256;
    pixel[2] = b * 220 / 256;
    return pixel32;
}
