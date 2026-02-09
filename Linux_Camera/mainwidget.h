#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

class QPushButton;

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);

signals:

private slots:
    void openDashboard();
    void openCamera();
    void openMap();
    void openIMU();
    void openGPS();
    void openWeather();

private:
    QPushButton *btns[6];
};

#endif // MAINWIDGET_H
