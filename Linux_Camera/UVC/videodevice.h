#ifndef VIDEODEVICE_H
#define VIDEODEVICE_H

#include <QObject>
#include <QString>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <string.h>
#include <stdlib.h>

// 清零宏定义
#define CLEAR(x) memset(&(x), 0, sizeof(x))

class VideoDevice : public QObject
{
    Q_OBJECT
public:
    explicit VideoDevice(const QString &dev_name);
    int open_device();
    int close_device();
    int init_device();
    int start_capturing();
    int stop_capturing();
    int uninit_device();
    int get_frame(void **, size_t *);
    int unget_frame();

signals:
    void display_error(const QString &);

private:
    struct buffer
    {
        void *start;
        size_t length;
    };

    int init_mmap();

    QString dev_name;
    int fd;
    buffer *buffers;
    unsigned int n_buffers;
    int index;
};

#endif // VIDEODEVICE_H
