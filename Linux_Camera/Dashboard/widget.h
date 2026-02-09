#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QCheckBox>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void backToMenu();

private:
    Ui::Widget *ui;
    QTimer *timer;
    double angle;
    int startAngle;
    int mark;
    int currentValue;
    QCheckBox *led1CheckBox;
    QCheckBox *led2CheckBox;
    QCheckBox *DoubleFlashCheckBox;
    int fuelValue;
    int tempValue;
    // ------- 新增：箭头定时器与状态变量 --------
    bool showLeftArrow = true; // 左箭头是否显示
    bool showRightArrow = true; // 右箭头是否显示
    bool showDoubleFlash = true; // 双闪是否显示

    bool led1Active = false;
    bool led2Active = false;
    bool DoubleFlashActive = false;

    QTimer* arrowTimer; // 定时器
    QTimer *adcTimer; // Timer for ADC polling

    void startSpeed();
    void initCanvas(QPainter& painter);
    void drawMiddleCircle(QPainter &painter, int radius);
    void drawCurrentSpeed(QPainter &painter);
    void drawScale(QPainter &painter,int radius);
    void drawScaleText(QPainter &painter, int radius);
    void drawPointLine(QPainter &painter,int lenth);
    void drawSpeedPie(QPainter &painter, int radius);
    void drawEllipseInnerBlack(QPainter &painter, int radius);
    void drawEllipseInnerShine(QPainter &painter, int radius);
    void drawEllipseOutShine(QPainter &painter, int radius);
    void drawLogo(QPainter &painter, int radius);

    void drawFuelGauge(QPainter &painter, int cx, int cy, int radius, int fuelPercent);
    void drawTempGauge(QPainter &painter, int cx, int cy, int radius, int tempPercent);
    void drawFuelIcon(QPainter &painter, int cx, int cy);
    void drawTempIcon(QPainter &painter, int cx, int cy);

    void addBackButton();
    void addCheckBoxes();

    void setupADCTimer();
    void updateGaugesFromADC();

    void drawRightArrow(QPainter &painter, int cx, int cy);
    void drawLeftArrow(QPainter &painter, int cx, int cy);
    void drawDoubleFlash(QPainter &painter, int cx, int cy);
};

#endif // WIDGET_H
