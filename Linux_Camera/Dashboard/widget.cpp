#include "widget.h"
#include "ui_widget.h"

#include <QPainter>
#include <QTimer>
#include <QtMath>
#include <QPushButton>
#include <QTimer>
#include <QCheckBox>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>

static int fb;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setFixedSize(480,272); // 适配屏幕分辨率
    startAngle = 150;
    mark = 0;
    currentValue = 0;
    fuelValue = 50;
    tempValue = 50;

    // Open the LED device
    fb = open("/dev/leds", O_RDWR);
    if (fb < 0) {
        perror("Failed to open device /dev/leds");
        exit(1);
    }
    ioctl(fb, 0, 0); // Turn off LED1
    ioctl(fb, 0, 1); // Turn off LED2
    addBackButton();
    addCheckBoxes();
    setupADCTimer(); // Initialize ADC reading timer
    // startSpeed();
    arrowTimer = new QTimer();
    connect(arrowTimer, &QTimer::timeout, this, [this]() {
        if (led1Active) {
            showRightArrow = !showRightArrow;
            ioctl(fb, showRightArrow ? 1 : 0, 0); // Control LED1
        }
        if (led2Active) {
            showLeftArrow = !showLeftArrow;
            ioctl(fb, showLeftArrow ? 1 : 0, 1); // Control LED2
        }
        if (DoubleFlashActive) {
            showDoubleFlash = !showDoubleFlash;
            ioctl(fb, showDoubleFlash ? 1 : 0, 0); // Control LED1
            ioctl(fb, showDoubleFlash ? 1 : 0, 1); // Control LED2
        }
        update();
    });
    arrowTimer->start(500); // 每500ms切换一次
}

Widget::~Widget()
{
    delete ui;
}

void Widget::addCheckBoxes()
{
    // LED1 Checkbox
    led1CheckBox = new QCheckBox("LED1", this);
    led1CheckBox->setGeometry(width() - 90, 30, 60, 20);
    connect(led1CheckBox, &QCheckBox::stateChanged, this, [this](int state) {
        led1Active = (state == Qt::Checked);
        if (!led1Active) {
            showRightArrow = false; // Turn off arrow
            ioctl(fb, 0, 0); // Turn off LED1
        }
        update();
    });

    // LED2 Checkbox
    led2CheckBox = new QCheckBox("LED2", this);
    led2CheckBox->setGeometry(85, 30, 60, 20);
    connect(led2CheckBox, &QCheckBox::stateChanged, this, [this](int state) {
        led2Active = (state == Qt::Checked);
        if (!led2Active) {
            showLeftArrow = false; // Turn off arrow
            ioctl(fb, 0, 1); // Turn off LED2
        }
        update();
    });

    // DoubleFalsh Checkbox
    DoubleFlashCheckBox = new QCheckBox("Falsh", this);
    DoubleFlashCheckBox->setGeometry(260, 20, 60, 20);
    connect(DoubleFlashCheckBox, &QCheckBox::stateChanged, this, [this](int state) {
        DoubleFlashActive = (state == Qt::Checked);
        if (!DoubleFlashActive) {
            showDoubleFlash = false; // Turn off arrow
            ioctl(fb, 0, 0); // Turn off LED1
            ioctl(fb, 0, 1); // Turn off LED2
        }
        update();
    });
}

// ------- 新增：绘制左箭头 --------
void Widget::drawLeftArrow(QPainter &painter, int cx, int cy)
{
    if (!showLeftArrow) return; // 如果当前状态是不显示，则直接返回

    painter.save();
    painter.translate(cx, cy);
    QPixmap pixmap(":/left_arrow.png");
    int w = 24, h = 24;
    painter.drawPixmap(-w/2, -h, w, h, pixmap);
    painter.restore();
}

// ------- 新增：绘制右箭头 --------
void Widget::drawRightArrow(QPainter &painter, int cx, int cy)
{
    if (!showRightArrow) return; // 如果当前状态是不显示，则直接返回

    painter.save();
    painter.translate(cx, cy);
    QPixmap pixmap(":/right_arrow.png");
    int w = 24, h = 24;
    painter.drawPixmap(-w/2, -h, w, h, pixmap);
    painter.restore();
}

// ------- 新增：绘制双闪 --------
void Widget::drawDoubleFlash(QPainter &painter, int cx, int cy)
{
    if (!showDoubleFlash) return; // 如果当前状态是不显示，则直接返回

    painter.save();
    painter.translate(cx, cy);
    QPixmap pixmap(":/double_flash.png");
    int w = 24, h = 24;
    painter.drawPixmap(-w/2, -h, w, h, pixmap);
    painter.restore();
}

void Widget::setupADCTimer()
{
    adcTimer = new QTimer(this);
    connect(adcTimer, &QTimer::timeout, this, &Widget::updateGaugesFromADC);
    adcTimer->start(500); // Poll ADC every 500ms
}

void Widget::updateGaugesFromADC()
{
    int fd = ::open("/dev/adc", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open ADC device");
        return;
    }

    char buffer[30] = "";
    int len = ::read(fd, buffer, sizeof(buffer) - 1);
    if (len > 0) {
        buffer[len] = '\0';
        int value = -1;
        sscanf(buffer, "%d", &value);

        if (value >= 0 && value <= 4096) {
            // Map ADC value to gauge ranges
            currentValue = value * 61 / 4096; // Speed gauge (0 to 61)
            fuelValue = 100 - (value * 100 / 4096); // Fuel gauge (0 to 100)
            tempValue = 30 + (value * 70 / 4096);  // Temperature gauge (30 to 100)
        }
    }
    ::close(fd);
    update();
}

void Widget::startSpeed()
{
    timer = new QTimer();
    connect(timer,&QTimer::timeout,this,[=](){
        if(mark == 0){
            currentValue++;
            if(currentValue >= 61)
                mark = 1;
        }
        if(mark == 1)
            currentValue--;
        if(currentValue == 0){
            mark = 0;
        }
        fuelValue = 100 - currentValue * 100 / 61;
        tempValue = 30 + currentValue * 70 / 61;
        update();
    });
    timer->start(50);
}

void Widget::initCanvas(QPainter& painter)
{
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setBrush(Qt::black);
    painter.drawRect(rect());
}

void Widget::addBackButton()
{
    QPushButton *backBtn = new QPushButton("Back", this);
    backBtn->setGeometry(10, 10, 60, 32);

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

void Widget::drawMiddleCircle(QPainter &painter, int radius)
{
    painter.setPen(QPen(Qt::white,3));
    painter.drawEllipse(QPoint(0,0),radius,radius);
}

void Widget::drawCurrentSpeed(QPainter &painter)
{
    painter.setPen(Qt::white);
    QFont font("Arial",30);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(-60,-60,120,70),Qt::AlignCenter,QString::number(currentValue*4));
    QFont font2("Arial",13);
    font2.setBold(true);
    painter.setFont(font2);
    painter.drawText(QRect(-60,-60,120,160),Qt::AlignCenter,"Km/h");
}

void Widget::drawScale(QPainter &painter,int radius)
{
    angle = 240*1.0 / 60;
    painter.save();
    painter.setPen(QPen(Qt::white,5));
    painter.rotate(startAngle);
    for(int i=0;i<=60;i++){
        if(i >= 40){
            painter.setPen(QPen(Qt::red,5));
        }
        if(i % 5 == 0){
            painter.drawLine(radius - 20,0,radius - 3,0);
        }else{
            painter.drawLine(radius - 8,0,radius - 3,0);
        }
        painter.rotate(angle);
    }
    painter.restore();
    painter.setPen(QPen(Qt::white,5));
}

void Widget::drawScaleText(QPainter &painter, int radius)
{
    QFont font(QFont("Arial",5));
    font.setBold(true);
    painter.setFont(font);
    int r = radius - 49;
    for(int i=0;i<=60;i++){
        if(i % 5 == 0){
            painter.save();
            int delX = qCos(qDegreesToRadians(210-angle*i)) * r;
            int delY = qSin(qDegreesToRadians(210-angle*i)) * r;
            painter.translate(QPoint(delX,-delY));
            painter.rotate(-120+angle*i);
            painter.drawText(-25,-25,50,30,Qt::AlignCenter,QString::number(i*4));
            painter.restore();
        }
    }
}

void Widget::drawPointLine(QPainter &painter,int lenth)
{
    painter.save();
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    static const QPointF points[4] = {
        QPointF(0,0),
        QPointF(100.0,-1.1),
        QPointF(100.0,1.1),
        QPointF(0,15.0),
    };
    painter.rotate(startAngle + angle * currentValue);
    painter.drawPolygon(points, 4);
    painter.restore();
}

void Widget::drawSpeedPie(QPainter &painter, int radius)
{
    QRect rentangle(-radius,-radius,radius*2,radius*2);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255,0,0,80));
    painter.drawPie(rentangle,(360-startAngle)*16,-angle*currentValue*16);
}

void Widget::drawEllipseInnerBlack(QPainter &painter, int radius)
{
    painter.setBrush(Qt::black);
    painter.drawEllipse(QPoint(0,0),radius,radius);
}

void Widget::drawEllipseInnerShine(QPainter &painter, int radius)
{
    QRadialGradient radaGradient(0,0,radius);
    radaGradient.setColorAt(0.0,QColor(255,0,0,200));
    radaGradient.setColorAt(1.0,QColor(0,0,0,100));
    painter.setBrush(radaGradient);
    painter.drawEllipse(QPoint(0,0),radius,radius);
}

void Widget::drawEllipseOutShine(QPainter &painter, int radius)
{
    QRect rentangle(-radius,-radius,radius*2,radius*2);
    painter.setPen(Qt::NoPen);

    QRadialGradient radiaGradient(0,0,radius);
    radiaGradient.setColorAt(1.0,QColor(255,0,0,200));
    radiaGradient.setColorAt(0.97,QColor(255,0,0,70));
    radiaGradient.setColorAt(0.9,QColor(0,0,0,0));
    radiaGradient.setColorAt(0,QColor(0,0,0,0));
    painter.setBrush(radiaGradient);

    painter.drawPie(rentangle,(360-150)*16,-angle*61*16);
}

void Widget::drawLogo(QPainter &painter, int radius)
{
    QRect rectangle(-65,radius*0.38,130,50);
    painter.drawPixmap(rectangle,QPixmap(":/icon.png"));
}

// ------- 新增：油量仪表板 --------
void Widget::drawFuelGauge(QPainter &painter, int cx, int cy, int radius, int fuelPercent)
{
    painter.save();
    painter.translate(cx, cy);

    QRectF rect(-radius, -radius, 2*radius, 2*radius);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(30, 30, 30));
    painter.drawPie(rect, 0*16, 180*16);

    painter.setPen(QPen(Qt::white, 2));
    for(int i=0; i<=10; ++i) {
        painter.save();
        painter.rotate(-90 + i*18);
        int len = (i%5==0) ? 12 : 7;
        painter.drawLine(0, -radius+4, 0, -radius+4+len);
        painter.restore();
    }
    QFont font("Arial",10); font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(-radius+4, 6, 20, 20, Qt::AlignLeft, "E");
    painter.drawText(radius-24, 6, 20, 20, Qt::AlignRight, "F");
    //painter.drawText(-11, -6, 22, 14, Qt::AlignCenter, QChar(0x26FD));

    painter.save();
    double angle = -90 + fuelPercent * 1.8;
    painter.rotate(angle);
    painter.setPen(QPen(Qt::red, 3));
    painter.drawLine(0,0,0,-radius+13);
    painter.restore();

    painter.restore();
}

void Widget::drawTempGauge(QPainter &painter, int cx, int cy, int radius, int tempPercent)
{
    painter.save();
    painter.translate(cx, cy);

    QRectF rect(-radius, -radius, 2*radius, 2*radius);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(30, 30, 30));
    painter.drawPie(rect, 0*16, 180*16);

    painter.setPen(QPen(Qt::white, 2));
    for(int i=0; i<=10; ++i) {
        painter.save();
        painter.rotate(-90 + i*18);
        int len = (i%5==0) ? 12 : 7;
        painter.drawLine(0, -radius+4, 0, -radius+4+len);
        painter.restore();
    }
    QFont font("Arial",10); font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(-radius+4, 6, 20, 20, Qt::AlignLeft, "C");
    painter.drawText(radius-24, 6, 20, 20, Qt::AlignRight, "H");
    //painter.drawText(-11, -6, 22, 14, Qt::AlignCenter, QChar(0x1F321));

    painter.save();
    double angle = -90 + tempPercent * 1.8;
    painter.rotate(angle);
    painter.setPen(QPen(Qt::red, 3));
    painter.drawLine(0,0,0,-radius+13);
    painter.restore();

    painter.restore();
}

// 仪表下方图标
// 使用图片绘制油量图标
void Widget::drawFuelIcon(QPainter &painter, int cx, int cy)
{
    painter.save();
    painter.translate(cx, cy);
    QPixmap pixmap(":/fuel-icon.png");
    // 图片适当缩放
    int w = 24, h = 24;
    painter.drawPixmap(-w/2, 0, w, h, pixmap);
    painter.restore();
}

// 使用图片绘制温度图标
void Widget::drawTempIcon(QPainter &painter, int cx, int cy)
{
    painter.save();
    painter.translate(cx, cy);
    QPixmap pixmap(":/temperature-icon.png");
    int w = 24, h = 24;
    painter.drawPixmap(-w/2, 0, w, h, pixmap);
    painter.restore();
}

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    initCanvas(painter);

    // 仪表半径设置
    int mainRadius = 90;
    int subRadius = 45;

    // 主仪表(速度)居中
    int mainCx = width()/2;
    int mainCy = height()/2 + 10;

    // 左侧油量仪表
    int fuelCx = mainCx - mainRadius - subRadius - 20; // 距离主表一定间距
    int fuelCy = mainCy;

    // 右侧温度仪表
    int tempCx = mainCx + mainRadius + subRadius + 20;
    int tempCy = mainCy;

    // 主速度表
    painter.save();
    painter.translate(mainCx, mainCy);
    drawMiddleCircle(painter, 36);
    drawScale(painter, mainRadius);
    drawScaleText(painter, mainRadius);
    drawPointLine(painter, mainRadius-28);
    drawSpeedPie(painter, mainRadius+12);
    drawEllipseInnerShine(painter, 55);
    drawEllipseInnerBlack(painter, 38);
    drawCurrentSpeed(painter);
    drawEllipseOutShine(painter, mainRadius+12);
    drawLogo(painter, mainRadius);
    painter.restore();

    // 左侧油量表
    drawFuelGauge(painter, fuelCx, fuelCy, subRadius, fuelValue);

    // 右侧温度表
    drawTempGauge(painter, tempCx, tempCy, subRadius, tempValue);

    // 油量图标位于油表下方
    drawFuelIcon(painter, fuelCx, fuelCy + subRadius + 6);

    // 温度图标位于温度表下方
    drawTempIcon(painter, tempCx, tempCy + subRadius + 6);

    // ------- 新增：绘制左箭头、右箭头、双闪 --------
    drawLeftArrow(painter, fuelCx, tempCy - subRadius - 10); // 左箭头位于温度表上方
    drawRightArrow(painter, tempCx, fuelCy - subRadius - 10); // 右箭头位于油量表上方
    drawDoubleFlash(painter,mainCx,mainCy - subRadius - 65); // 双闪位于速度表上方
}
