#include "IMUProcess.h"
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QFont>
#include <QMessageBox>
#include <QPushButton>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <QTimer>

IMUProcess::IMUProcess(QWidget *parent) : QWidget(parent), notifier(nullptr), fd(-1), buzzerFd(-1) {
    // Set up layout
    auto *layout = new QGridLayout(this);
    setLayout(layout);

    // Modify window size to 480x272
    resize(480, 272);

    // Set background color to white
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    addBackButton();

    // Create modules for Accelerometer, Gyroscope, and Angle data
    createParameterModule(layout, 0, "Accelerometer", accLabels);
    createParameterModule(layout, 1, "Gyroscope", gyroLabels);
    createParameterModule(layout, 2, "Angle", angleLabels);

    // Warning label
    warningLabel = new QLabel(this);
    warningLabel->setText("");
    warningLabel->setFont(QFont("Consolas", 12, QFont::Bold));
    warningLabel->setStyleSheet("color: red;");
    layout->addWidget(warningLabel, 1, 0, 1, 3, Qt::AlignCenter);

    // Open serial port by specifying a port name
    QString portName = "ttyUSB0";  // Replace with desired serial port, e.g., "ttyS0", "ttyUSB0"
    fd = openSerialPort(portName);
    if (fd < 0) {
        QMessageBox::warning(this, tr("Error"), tr("Fail to open serial port!"));
        return;
    }

    // Open buzzer device
    buzzerFd = ::open("/dev/buzzer_ctl", O_RDWR);
    if (buzzerFd < 0) {
        QMessageBox::warning(this, tr("Error"), tr("Fail to open buzzer device!"));
    }

    // Create a QSocketNotifier to monitor incoming data
    notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &IMUProcess::readSerialData);

    // Initialize variables
    resetBuffer();
}

IMUProcess::~IMUProcess() {
    if (notifier) {
        delete notifier;
        notifier = nullptr;
    }

    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }

    if (buzzerFd >= 0) {
        ::close(buzzerFd);
        buzzerFd = -1;
    }
}

int IMUProcess::openSerialPort(const QString &portName) {
    QString devicePath = "/dev/" + portName;  // Construct device path
    int fd = ::open(devicePath.toStdString().c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        return -1;
    }

    termios serialAttr;
    memset(&serialAttr, 0, sizeof(serialAttr));
    serialAttr.c_iflag = IGNPAR;
    serialAttr.c_cflag = B9600 | HUPCL | CS8 | CREAD | CLOCAL;
    serialAttr.c_cc[VMIN] = 1;

    if (tcsetattr(fd, TCSANOW, &serialAttr) != 0) {
        ::close(fd);
        return -1;
    }

    return fd;
}

void IMUProcess::resetBuffer() {
    start = 0;
    data_length = 0;
    CheckSum = 0;
    memset(RxBuff, 0, buf_length);
}

void IMUProcess::addBackButton()
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

void IMUProcess::processInputData(unsigned char inputdata) {
    if (inputdata == 0x55 && start == 0) {
        start = 1;
        data_length = buf_length;
        CheckSum = 0;
    }

    if (start == 1) {
        CheckSum += inputdata;
        RxBuff[buf_length - data_length] = inputdata;
        data_length--;

        if (data_length == 0) {
            CheckSum = (CheckSum - inputdata) & 0xff;
            start = 0;
            handleData();
        }
    }
}

void IMUProcess::createParameterModule(QGridLayout *layout, int column, const QString &title, QLabel *labels[3]) {
    QFrame *moduleFrame = new QFrame(this);
    moduleFrame->setFrameStyle(QFrame::Box);
    moduleFrame->setLineWidth(2);
    moduleFrame->setStyleSheet("border: 2px solid black;");

    QVBoxLayout *moduleLayout = new QVBoxLayout(moduleFrame);

    QLabel *titleLabel = new QLabel(title, this);
    QFont font("Consolas", 12, QFont::Bold);
    titleLabel->setFont(font);
    moduleLayout->addWidget(titleLabel);

    for (int i = 0; i < 3; ++i) {
        QFrame *rowFrame = new QFrame(this);
        rowFrame->setFrameStyle(QFrame::Box);
        rowFrame->setLineWidth(1);
        rowFrame->setStyleSheet("border: 1px solid black;");
        QHBoxLayout *rowLayout = new QHBoxLayout(rowFrame);

        QLabel *nameLabel = new QLabel(this);
        nameLabel->setText(i == 0 ? "X:" : (i == 1 ? "Y:" : "Z:"));
        nameLabel->setFont(QFont("Consolas", 10));
        rowLayout->addWidget(nameLabel);

        labels[i] = new QLabel("0.00", this);
        labels[i]->setFont(QFont("Consolas", 10));
        rowLayout->addWidget(labels[i]);

        moduleLayout->addWidget(rowFrame);
    }

    layout->addWidget(moduleFrame, 0, column);
}

void IMUProcess::readSerialData() {
    char c;
    if (::read(fd, &c, sizeof(c)) != 1) {
        QMessageBox::warning(this, tr("Error"), tr("Receive error!"));
        return;
    }
    processInputData(static_cast<unsigned char>(c));
}

void IMUProcess::handleData() {
    const double threshold = 3.0;  // Example threshold for acceleration

    if (RxBuff[buf_length - 1] != CheckSum) {
        QMessageBox::warning(this, tr("Error"), tr("Checksum mismatch!"));
        return;
    }

    if (RxBuff[1] == 0x51) {  // Accelerometer data
        parseData(RxBuff + 2, acc, 16.0);
        updateModule(accLabels, acc);

        bool exceedsThreshold = false;
        for (int i = 0; i < 3; ++i) {
            if (std::abs(acc[i]) > threshold) {
                exceedsThreshold = true;
                break;
            }
        }

        if (exceedsThreshold) {
            // Flash warningLabel for 5 seconds
            warningLabel->setText("Acceleration exceeded threshold!");
            flashWarningLabel();

            // Activate buzzer for two beeps, each lasting 200ms
            toggleBuzzer(true);
            QTimer::singleShot(200, this, [this]() { toggleBuzzer(false); });  // First beep off
            QTimer::singleShot(400, this, [this]() { toggleBuzzer(true); });   // Second beep on
            QTimer::singleShot(600, this, [this]() { toggleBuzzer(false); });  // Second beep off
        }
    } else if (RxBuff[1] == 0x52) {  // Gyroscope data
        parseData(RxBuff + 2, gyro, 2000.0);
        updateModule(gyroLabels, gyro);
    } else if (RxBuff[1] == 0x53) {  // Angle data
        parseData(RxBuff + 2, angle, 180.0);
        updateModule(angleLabels, angle);
    }
}

void IMUProcess::flashWarningLabel() {
    QTimer *flashTimer = new QTimer(this);
    int flashDuration = 5000;    // Total duration for flashing (5 seconds)
    int flashInterval = 500;     // Interval for each flash (500ms)
    bool showText = true;        // Track whether to show or clear the text

    connect(flashTimer, &QTimer::timeout, this, [this, &showText]() {
        if (showText) {
            warningLabel->setText("Acceleration exceeded threshold!"); // Set warning text
        } else {
            warningLabel->setText(""); // Clear warning text
        }
        showText = !showText; // Toggle the state
    });

    flashTimer->start(flashInterval);

    // Stop the flashing after the duration
    QTimer::singleShot(flashDuration, this, [flashTimer, this]() {
        flashTimer->stop();
        warningLabel->setText("");  // Clear the warning text at the end
        flashTimer->deleteLater();
    });
}

void IMUProcess::parseData(const unsigned char *data, double *output, double scale) {
    for (int i = 0; i < 3; ++i) {
        int16_t value = (data[i * 2 + 1] << 8) | data[i * 2];
        output[i] = value / 32768.0 * scale;
        if (output[i] >= scale) {
            output[i] -= 2 * scale;
        }
    }
}

void IMUProcess::updateModule(QLabel *labels[3], const double *data) {
    for (int i = 0; i < 3; ++i) {
        labels[i]->setText(QString::number(data[i], 'f', 2));
    }
}

void IMUProcess::toggleBuzzer(bool state) {
    if (buzzerFd >= 0) {
        ioctl(buzzerFd, state ? 1 : 0, 0);
    }
}
