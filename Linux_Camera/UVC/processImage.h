#ifndef PROCESSIMAGE_H
#define PROCESSIMAGE_H

#include <QWidget>
#include <QLabel>
#include <QImage>
#include <QTimer>
#include <QHBoxLayout>
#include <QMessageBox>
#include "videodevice.h"

class ProcessImage : public QWidget
{
    Q_OBJECT
public:
    explicit ProcessImage(QWidget *parent = nullptr);
    ~ProcessImage();

signals:
    void backToMenu();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *label;
    QImage *frame;
    QTimer *timer;
    int rs;
    uchar *pp;
    uchar *p;
    unsigned int len;
    int convert_yuv_to_rgb_pixel(int y, int u, int v);
    int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
    VideoDevice *vd;
    void addBackButton();

private slots:
    void display_error(QString err);

};

#endif // PROCESSIMAGE_H
