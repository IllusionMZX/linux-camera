#include "videodevice.h"
#include <unistd.h> // for close/munmap

VideoDevice::VideoDevice(const QString &dev_name)
    : dev_name(dev_name),
      fd(-1),
      buffers(nullptr),
      n_buffers(0),
      index(-1)
{
}

int VideoDevice::open_device()
{
    fd = open(dev_name.toStdString().c_str(), O_RDWR, 0);
    if (fd == -1)
    {
        emit display_error(tr("open: %1").arg(QString(strerror(errno))));
        return -1;
    }
    return 0;
}

int VideoDevice::close_device()
{
    if (fd != -1 && close(fd) == -1)
    {
        emit display_error(tr("close: %1").arg(QString(strerror(errno))));
        return -1;
    }
    fd = -1;
    return 0;
}

int VideoDevice::init_device()
{
    v4l2_capability cap;
    v4l2_cropcap cropcap;
    v4l2_crop crop;
    v4l2_format fmt;

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        if (errno == EINVAL)
            emit display_error(tr("%1 is no V4l2 device").arg(dev_name));
        else
            emit display_error(tr("VIDIOC_QUERYCAP: %1").arg(QString(strerror(errno))));
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        emit display_error(tr("%1 is no video capture device").arg(dev_name));
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        emit display_error(tr("%1 does not support streaming i/o").arg(dev_name));
        return -1;
    }

    CLEAR(cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_CROPCAP, &cropcap) == 0)
    {
        CLEAR(crop);
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;
        if (ioctl(fd, VIDIOC_S_CROP, &crop) == -1)
        {
            if (errno != EINVAL)
            {
                emit display_error(tr("VIDIOC_S_CROP: %1").arg(QString(strerror(errno))));
                return -1;
            }
            // else ignore EINVAL
        }
    }
    else
    {
        emit display_error(tr("VIDIOC_CROPCAP: %1").arg(QString(strerror(errno))));
        return -1;
    }

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 950;
    fmt.fmt.pix.height = 670;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
    {
        emit display_error(tr("VIDIOC_S_FMT: %1").arg(QString(strerror(errno))));
        return -1;
    }

    if (init_mmap() == -1)
        return -1;

    return 0;
}

int VideoDevice::init_mmap()
{
    v4l2_requestbuffers req;
    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
    {
        if (errno == EINVAL)
            emit display_error(tr("%1 does not support memory mapping").arg(dev_name));
        else
            emit display_error(tr("VIDIOC_REQBUFS: %1").arg(QString(strerror(errno))));
        return -1;
    }

    if (req.count < 2)
    {
        emit display_error(tr("Insufficient buffer memory on %1").arg(dev_name));
        return -1;
    }

    buffers = (buffer *)calloc(req.count, sizeof(*buffers));
    if (!buffers)
    {
        emit display_error(tr("out of memory"));
        return -1;
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        v4l2_buffer buf;
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            emit display_error(tr("VIDIOC_QUERYBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = mmap(
            NULL,
            buf.length,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd, buf.m.offset);

        if (buffers[n_buffers].start == MAP_FAILED)
        {
            emit display_error(tr("mmap: %1").arg(QString(strerror(errno))));
            return -1;
        }
    }
    return 0;
}

int VideoDevice::start_capturing()
{
    for (unsigned int i = 0; i < n_buffers; ++i)
    {
        v4l2_buffer buf;
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)
        {
            emit display_error(tr("VIDIOC_QBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1)
    {
        emit display_error(tr("VIDIOC_STREAMON: %1").arg(QString(strerror(errno))));
        return -1;
    }
    return 0;
}

int VideoDevice::stop_capturing()
{
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1)
    {
        emit display_error(tr("VIDIOC_STREAMOFF: %1").arg(QString(strerror(errno))));
        return -1;
    }
    return 0;
}

int VideoDevice::uninit_device()
{
    for (unsigned int i = 0; i < n_buffers; ++i)
    {
        if (munmap(buffers[i].start, buffers[i].length) == -1)
        {
            emit display_error(tr("munmap: %1").arg(QString(strerror(errno))));
            return -1;
        }
    }
    free(buffers);
    buffers = nullptr;
    n_buffers = 0;
    return 0;
}

int VideoDevice::get_frame(void **frame_buf, size_t *len)
{
    v4l2_buffer queue_buf;
    CLEAR(queue_buf);

    queue_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queue_buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_DQBUF, &queue_buf) == -1)
    {
        switch (errno)
        {
        case EAGAIN:
            return -1;
        case EIO:
            return -1;
        default:
            emit display_error(tr("VIDIOC_DQBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }
    }

    *frame_buf = buffers[queue_buf.index].start;
    *len = buffers[queue_buf.index].length;
    index = queue_buf.index;
    return 0;
}

int VideoDevice::unget_frame()
{
    if (index != -1)
    {
        v4l2_buffer queue_buf;
        CLEAR(queue_buf);

        queue_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        queue_buf.memory = V4L2_MEMORY_MMAP;
        queue_buf.index = index;

        if (ioctl(fd, VIDIOC_QBUF, &queue_buf) == -1)
        {
            emit display_error(tr("VIDIOC_QBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }
        index = -1;
        return 0;
    }
    return -1;
}
