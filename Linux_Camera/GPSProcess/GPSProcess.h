#ifndef GPSPROCESS_H
#define GPSPROCESS_H

#include <QWidget>
#include <QTextEdit>
#include <QSocketNotifier>

class GPSProcess : public QWidget {
    Q_OBJECT
public:
    explicit GPSProcess(QWidget *parent = nullptr);
    ~GPSProcess();

signals:
    void backToMenu();

private:
    QTextEdit *textEdit;
    QSocketNotifier *notifier;
    int fd;
    bool gpsValid;

    QString utcTime;
    QString latitude;
    QString uLatitude;
    QString longitude;
    QString uLongitude;
    QString numSatellites;
    QString altitude;
    QString cogTrue;
    QString cogMagnetic;
    QString speedKnots;
    QString speedKph;

    void addBackButton();
    int openSerialPort(const QString &portName);
    void readSerialData();
    void parseGGA(const QStringList &ggaData);
    void parseVTG(const QStringList &vtgData);
    double convertToDegrees(const QString &data1, const QString &data2);
};

#endif // GPSPROCESS_H
