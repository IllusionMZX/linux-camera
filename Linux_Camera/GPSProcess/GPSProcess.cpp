#include "GPSProcess.h"
#include <QMessageBox>
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <QVBoxLayout>
#include <QPushButton>
#include <termios.h>

GPSProcess::GPSProcess(QWidget *parent) : QWidget(parent), textEdit(new QTextEdit(this)), notifier(nullptr), fd(-1), gpsValid(false) {
    // Set up UI
    auto *layout = new QVBoxLayout(this);
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);
    setLayout(layout);

    // Modify window size to 480x272
    resize(480, 272);

    // Set background color to white
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    // Add back button
    addBackButton();

    // Open serial port
    QString portName = "ttyUSB1";  // Replace with the desired serial port
    fd = openSerialPort(portName);
    if (fd < 0) {
        QMessageBox::warning(this, "Error", "Failed to open serial port!");
        return;
    }

    // Create a QSocketNotifier to monitor incoming data
    notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &GPSProcess::readSerialData);
}

GPSProcess::~GPSProcess() {
    if (notifier) {
        delete notifier;
        notifier = nullptr;
    }

    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

void GPSProcess::addBackButton() {
    QPushButton *backBtn = new QPushButton("Back", this);
    backBtn->setGeometry(10, 10, 60, 32);

    // 设置按钮样式
    backBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: white;"
        "    color: black;"
        "    border: 2px solid black;"
        "    border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f0f0f0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #e0e0e0;"
        "}"
    );

    connect(backBtn, &QPushButton::clicked, this, [this]() {
        emit backToMenu();
    });
}

int GPSProcess::openSerialPort(const QString &portName) {
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

void GPSProcess::readSerialData() {
    char c;
    static QByteArray buffer;

    while (::read(fd, &c, sizeof(c)) > 0) {
        if (c == '\n') {  // End of line
            QString line = QString::fromUtf8(buffer).trimmed();
            buffer.clear();

            if (line.startsWith("$GNGGA")) {
                QStringList ggaData = line.split(",");
                if (ggaData.size() >= 15) {
                    parseGGA(ggaData);
                    gpsValid = true;
                }
            } else if (line.startsWith("$GNVTG") && gpsValid) {
                QStringList vtgData = line.split(",");
                if (vtgData.size() >= 9) {
                    parseVTG(vtgData);
                }
            }

            // Update display
            textEdit->append("*********************");
            textEdit->append("UTC Time: " + utcTime);
            textEdit->append("Latitude: " + latitude);
            textEdit->append("Longitude: " + longitude);
            textEdit->append("Number of satellites: " + numSatellites);
            textEdit->append("Altitude: " + altitude);
            textEdit->append("True north heading: " + cogTrue + "°");
            textEdit->append("Magnetic north heading: " + cogMagnetic + "°");
            textEdit->append("Ground speed: " + speedKnots + " Kn");
            textEdit->append("Ground speed: " + speedKph + " Km/h");
            textEdit->append("*********************");
        } else {
            buffer.append(c);
        }
    }
}

void GPSProcess::parseGGA(const QStringList &ggaData) {
    utcTime = ggaData[1];

    // 处理纬度数据
    QString rawLatitude = ggaData[2]; // 原始纬度数据，例如 "4139.9999"
    double latitudeDegrees = 0.0;
    if (!rawLatitude.isEmpty()) {
        int dotIndex = rawLatitude.indexOf('.'); // 找到小数点的位置
        if (dotIndex > 2) { // 确保数据格式正确
            QString degreesPart = rawLatitude.left(dotIndex - 2); // 提取度数部分
            QString minutesPart = rawLatitude.mid(dotIndex - 2);  // 提取分钟部分
            latitudeDegrees = degreesPart.toDouble() + (minutesPart.toDouble() / 60.0); // 转换为十进制度数
        }
    }
    latitude = QString::number(latitudeDegrees, 'f', 8) + "°N";

    // 处理经度数据
    QString rawLongitude = ggaData[4]; // 原始经度数据，例如 "12325.29102"
    double longitudeDegrees = 0.0;
    if (!rawLongitude.isEmpty()) {
        int dotIndex = rawLongitude.indexOf('.'); // 找到小数点的位置
        if (dotIndex > 3) { // 确保数据格式正确
            QString degreesPart = rawLongitude.left(dotIndex - 2); // 提取度数部分
            QString minutesPart = rawLongitude.mid(dotIndex - 2);  // 提取分钟部分
            longitudeDegrees = degreesPart.toDouble() + (minutesPart.toDouble() / 60.0); // 转换为十进制度数
        }
    }
    longitude = QString::number(longitudeDegrees, 'f', 8) + "°E";

    numSatellites = ggaData[7];
    altitude = ggaData[9];
}

void GPSProcess::parseVTG(const QStringList &vtgData) {
    cogTrue = vtgData[1] + "°";
    cogMagnetic = vtgData[3] + "°";
    speedKnots = vtgData[5]; // 地面速度
    speedKph = vtgData[7]; // 地面速度转换为公里每小时
}

double GPSProcess::convertToDegrees(const QString &data1, const QString &data2) {
    int len = data1.length();
    QString strData2 = QString("%1").arg(data2.toInt(), 5, 10, QChar('0'));
    int tempData = data1.toInt();
    int symbol = (tempData < 0) ? -1 : 1;
    int degree = tempData / 100;
    QString strDecimal = data1.mid(len - 2) + strData2;
    double fDegree = strDecimal.toDouble() / 60.0 / 100000.0;
    return symbol > 0 ? degree + fDegree : degree - fDegree;
}
