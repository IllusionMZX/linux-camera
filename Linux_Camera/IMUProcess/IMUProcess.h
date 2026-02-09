#ifndef IMUPROCESS_H
#define IMUPROCESS_H

#include <QWidget>
#include <QLabel>
#include <QSocketNotifier>
#include <QGridLayout>
#include <QTimer>
#include <termios.h>  // For serial port configuration

class IMUProcess : public QWidget {
    Q_OBJECT
public:
    explicit IMUProcess(QWidget *parent = nullptr);
    ~IMUProcess();

private slots:
    void readSerialData();

signals:
    void backToMenu();

private:
    QSocketNotifier *notifier;
    int fd;  // File descriptor for the serial port
    int buzzerFd;  // File descriptor for the buzzer device

    // Buffer and processing variables
    static constexpr int buf_length = 11;
    static constexpr double accelerationThreshold = 12.0;  // Threshold for acceleration
    unsigned char RxBuff[buf_length];
    int start = 0;
    int data_length = 0;
    int CheckSum = 0;

    double acc[3] = {0.0};
    double gyro[3] = {0.0};
    double angle[3] = {0.0};

    QLabel *accLabels[3] = {nullptr};   // Labels for Accelerometer data
    QLabel *gyroLabels[3] = {nullptr}; // Labels for Gyroscope data
    QLabel *angleLabels[3] = {nullptr}; // Labels for Angle data
    QLabel *warningLabel = nullptr;    // Label for warning messages

    void resetBuffer();
    void processInputData(unsigned char inputdata);
    void handleData();
    void parseData(const unsigned char *data, double *output, double scale);
    void updateModule(QLabel *labels[3], const double *data);
    void toggleBuzzer(bool state);  // Toggle buzzer state (on/off)

    int openSerialPort(const QString &portName);  // Open and configure the serial port

    void flashWarningLabel();
    void addBackButton();  // Add back button to the UI
    void createParameterModule(QGridLayout *layout, int column, const QString &title, QLabel *labels[3]);  // Create parameter module for UI
};

#endif // IMUPROCESS_H
