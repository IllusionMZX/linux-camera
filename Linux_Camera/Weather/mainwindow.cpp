#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QContextMenuEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QString>
#include <QTimer>
#include <QDesktopServices>

#include "weathertool.h"
#include "sysTrayIcon.h"

#define INCREMENT 3 //温度升降 像素点增量
#define POINT_RADIUS 3 //曲线描点的大小
#define TEXT_OFFSET_X 12
#define TEXT_OFFSET_Y 12

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //setWindowFlag(Qt::FramelessWindowHint); //设置无边框窗口
    //setAttribute(Qt::WA_TranslucentBackground);  //设置背景透明
    //setFixedSize(width(), height()); //设置固定窗口大小
    this->resize(480, 272);
    addBackButton();
    //构建右键菜单
    mExitMenu = new QMenu(this);
    mExitAct = new QAction();

    mExitAct->setText("退出");
    mExitAct->setIcon(QIcon(":/res/close.png"));

    mExitMenu->addAction(mExitAct);
    connect(mExitAct, &QAction::triggered, this, [=]{
        qApp->exit(0);
    });

    //UI初始化，控件添加到控件数组
    //星期 日期
    mWeekList << ui->lblWeek0 << ui->lblWeek1 << ui->lblWeek2 << ui->lblWeek3 << ui->lblWeek4 << ui->lblWeek5;
    mDateList << ui->lblDate0 << ui->lblDate1 << ui->lblDate2 << ui->lblDate3 << ui->lblDate4 << ui->lblDate5;

    // 天气 图标
    mTypeList << ui->lblType0 << ui->lblType1 << ui->lblType2 << ui->lblType3 << ui->lblType4 << ui->lblType5;
    mTypeIconList << ui->lblTypeIcon0 << ui->lblTypeIcon1 << ui->lblTypeIcon2 << ui->lblTypeIcon3 << ui->lblTypeIcon4 << ui->lblTypeIcon5;

    //天气指数
    mAqiList << ui->lblQuality0 << ui->lblQuality1 << ui->lblQuality2 << ui->lblQuality3 << ui->lblQuality4 << ui->lblQuality5;

    //风力 风向
    mFxList << ui->lblFx0 << ui->lblFx1 << ui->lblFx2 << ui->lblFx3 << ui->lblFx4 << ui->lblFx5;
    mFlList << ui->lblFl0 << ui->lblFl1 << ui->lblFl2 << ui->lblFl3 << ui->lblFl4 << ui->lblFl5;
/*
    //根据keys,设置icon的路径
    mTypeMap.insert("暴雪",":/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到大暴雨",":/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨到特大暴雨",":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("大到暴雪",":/res/type/BaoXue.png");
    mTypeMap.insert("大雪",":/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/res/type/DongYu.png");
    mTypeMap.insert("多云",":/res/type/DuoYun.png");
    mTypeMap.insert("浮沉",":/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYu.png");
    mTypeMap.insert("霾",":/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");
    mTypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("undefined",":/res/type/undefined.png");
    mTypeMap.insert("雾",":/res/type/Wu.png");
    mTypeMap.insert("小到中雪",":/res/type/ZhongXue.png");
    mTypeMap.insert("小到中雨",":/res/type/ZhongYu.png");
    mTypeMap.insert("小雪",":/res/type/XiaoXue.png");
    mTypeMap.insert("小雨",":/res/type/XiaoYu.png");
    mTypeMap.insert("雪",":/res/type/XiaoXue.png");
    mTypeMap.insert("扬沙",":/res/type/YangSha.png");
    mTypeMap.insert("阴",":/res/type/Yin.png");
    mTypeMap.insert("雨",":/res/type/ZhongYu.png");
    mTypeMap.insert("雨夹雪",":/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    mTypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    mTypeMap.insert("中到大雪",":/res/type/DaXue.png");
    mTypeMap.insert("中到大雨",":/res/type/DaYu.png");
    mTypeMap.insert("中雪",":/res/type/ZhongXue.png");
    mTypeMap.insert("中雨",":/res/type/ZhongYu.png");
*/
    // Set the icon paths based on keys
    mTypeMap.insert("Blizzard", ":/res/type/BaoXue.png");
    mTypeMap.insert("Hv Rain", ":/res/type/BaoYu.png");
    mTypeMap.insert("Hv Rain to Tr Rain", ":/res/type/DaBaoYu.png");
    mTypeMap.insert("Tr Rain", ":/res/type/DaBaoYu.png");
    mTypeMap.insert("Tr Rain to Ex Tr Rain", ":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("Hv Snow to Blizzard", ":/res/type/BaoXue.png");
    mTypeMap.insert("Hv Snow", ":/res/type/DaXue.png");
    mTypeMap.insert("Hv Rain", ":/res/type/DaYu.png");
    mTypeMap.insert("Freezing Rain", ":/res/type/DongYu.png");
    mTypeMap.insert("Cloudy", ":/res/type/DuoYun.png");
    mTypeMap.insert("Floating Dust", ":/res/type/FuChen.png");
    mTypeMap.insert("Thunderstorms", ":/res/type/LeiZhenYu.png");
    mTypeMap.insert("Thunderstorms with Hail", ":/res/type/LeiZhenYu.png");
    mTypeMap.insert("Haze", ":/res/type/Mai.png");
    mTypeMap.insert("Severe Sandstorm", ":/res/type/QiangShaChenBao.png");
    mTypeMap.insert("Sunny", ":/res/type/Qing.png");
    mTypeMap.insert("Sandstorm", ":/res/type/ShaChenBao.png");
    mTypeMap.insert("Ex Tr Rain", ":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("undefined", ":/res/type/undefined.png");
    mTypeMap.insert("Fog", ":/res/type/Wu.png");
    mTypeMap.insert("Lt Snow to Md Snow", ":/res/type/ZhongXue.png");
    mTypeMap.insert("Lt Rain to Md Rain", ":/res/type/ZhongYu.png");
    mTypeMap.insert("Lt Snow", ":/res/type/XiaoXue.png");
    mTypeMap.insert("Lt Rain", ":/res/type/XiaoYu.png");
    mTypeMap.insert("Snow", ":/res/type/XiaoXue.png");
    mTypeMap.insert("Blowing Sand", ":/res/type/YangSha.png");
    mTypeMap.insert("Overcast", ":/res/type/Yin.png");
    mTypeMap.insert("Rain", ":/res/type/ZhongYu.png");
    mTypeMap.insert("Sleet", ":/res/type/YuJiaXue.png");
    mTypeMap.insert("Snow Showers", ":/res/type/ZhenXue.png");
    mTypeMap.insert("Rain Showers", ":/res/type/ZhenYu.png");
    mTypeMap.insert("Md to Hv Snow", ":/res/type/DaXue.png");
    mTypeMap.insert("Md to Hv Rain", ":/res/type/DaYu.png");
    mTypeMap.insert("Md Snow", ":/res/type/ZhongXue.png");
    mTypeMap.insert("Md Rain", ":/res/type/ZhongYu.png");


    //托盘菜单
    msysTray = new sysTrayIcon(this);

    //请求网络
    mNetAccessManager = new QNetworkAccessManager(this);
    connect(mNetAccessManager, &QNetworkAccessManager::finished, this, &MainWindow::onReplied);
    //直接在构造中，请求天气数据 30min自动刷新一次
    QTimer *timer = new QTimer();
    timer->setInterval(1800000);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(getWeatherInfo("苏州")));
    getWeatherInfo("苏州");

    //回车触发按钮
    connect(ui->leCity, SIGNAL(returnPressed()),ui->btnSearch, SIGNAL(clicked()));

    //给标签添加事件过滤器
    ui->lblHighCurve->installEventFilter(this);
    ui->lblLowCurve->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addBackButton()
{
    QPushButton *backBtn = new QPushButton("Back", this);
    backBtn->setGeometry(10, 240, 60, 32);

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

void MainWindow::on_min_Button_clicked()
{
    //最小化按钮
    this->hide();
}

void MainWindow::on_close_Button_clicked()
{
    //关闭按钮
    this->close();
}

void MainWindow::onReplied(QNetworkReply *reply)
{
    // qDebug() << "onReplied success";

    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    // qDebug()<< status_code;
/*
    qDebug() << "operation:" << reply->operation();
    qDebug() << "status_code:" << status_code;
    qDebug() << "url:" << reply->url();
    qDebug() << "rawHeader:" << reply->rawHeaderList();
*/
    if(reply->error() != QNetworkReply::NoError || status_code != 200 )
    {
        // qDebug() << reply->errorString().toLatin1().data();
        QMessageBox::warning(this, "天气", "请求数据失败", QMessageBox::Ok);
    }
    else
    {
        QByteArray byteArr = reply->readAll();
        //qDebug() << "read all:" << byteArr.data();
        //解析数据
        parseJson(byteArr);
    }
    reply->deleteLater();
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    //弹出右键菜单
    //mExitMenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    mOffset = event->globalPos() - this->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() - mOffset);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    //重写hide事件，隐藏窗口 最小化到托盘
    if(msysTray->isVisible())
    {
        QDateTime currentTime = QDateTime::currentDateTime();
        if (mLastCloseTime.isNull() || mLastCloseTime.msecsTo(currentTime) > 100) {
            msysTray->showTrayMessage();
        }
        hide();
        event->ignore();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
        button=QMessageBox::warning(this,tr("退出程序"),QString(tr("确认退出程序")),QMessageBox::Yes|QMessageBox::No);
        if(button==QMessageBox::No)
        {
            event->ignore(); // 忽略退出信号，程序继续进行
        }
        else if(button==QMessageBox::Yes)
        {
            mLastCloseTime = QDateTime::currentDateTime();
            event->accept();
        }
}

void MainWindow::getWeatherInfo(QString cityName)
{
    QString cityCode = WeatherTool::getCityCode(cityName);
    //qDebug()<<"testcitycode"<<cityCode;
    QUrl url("http://t.weather.itboy.net/api/weather/city/101070101" + cityCode);
    mNetAccessManager->get(QNetworkRequest(url));
}

void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(byteArray, &err);
    if(err.error != QJsonParseError::NoError)
    {
        return ;
    }
    QJsonObject rootObj = doc.object();
    //qDebug() << rootObj.value("message").toString();

    // 解析日期、城市
    mToday.date = rootObj.value("time").toString();
    mToday.city = rootObj.value("cityInfo").toObject().value("city").toString();


    // 解析yesterday
    QJsonObject objData = rootObj.value("data").toObject();
    QJsonObject objYesterday = objData.value("yesterday").toObject();
    mDay[0].week = objYesterday.value("week").toString();
    mDay[0].date = objYesterday.value("ymd").toString();
    mDay[0].type = objYesterday.value("type").toString();


    QString s;
    s = objYesterday.value("high").toString().split(" ").at(1);
    s = s.left(s.length()-1);
    mDay[0].high = s.toInt();

    s = objYesterday.value("low").toString().split(" ").at(1);
    s = s.left(s.length()-1);
    mDay[0].low = s.toInt();

    //风向、风力、污染指数
    mDay[0].fx = objYesterday.value("fx").toString();
    mDay[0].fl = objYesterday.value("fl").toString();
    mDay[0].aqi = objYesterday.value("aqi").toDouble();

    //获取forcast中数据
    QJsonArray forecastArr = objData.value("forecast").toArray();
    for(int i=0; i < 5; i++)
    {
        QJsonObject objForecast = forecastArr[i].toObject();
        mDay[i+1].week = objForecast.value("week").toString();
        mDay[i+1].date = objForecast.value("ymd").toString();
        mDay[i+1].type = objForecast.value("type").toString();

        s = objForecast.value("high").toString().split(" ").at(1);
        s = s.left(s.length()-1);
        mDay[i+1].high = s.toInt();

        s = objForecast.value("low").toString().split(" ").at(1);
        s = s.left(s.length()-1);
        mDay[i+1].low = s.toInt();

        //风向、风力、污染指数
        mDay[i+1].fx = objForecast.value("fx").toString();
        mDay[i+1].fl = objForecast.value("fl").toString();
        mDay[i+1].aqi = objForecast.value("aqi").toDouble();
    }

    for(int i = 0; i < 5; i++)
    {
        if(mDay[i].fx == "北风")
        {
            mDay[i].fx = "N";
        }
        else if(mDay[i].fx == "东风")
        {
            mDay[i].fx = "E";
        }
        else if(mDay[i].fx == "南风")
        {
            mDay[i].fx = "S";
        }
        else if(mDay[i].fx == "西风")
        {
            mDay[i].fx = "W";
        }
        else if(mDay[i].fx == "东北风")
        {
            mDay[i].fx = "NE";
        }
        else if(mDay[i].fx == "东南风")
        {
            mDay[i].fx = "SE";
        }
        else if(mDay[i].fx == "西南风")
        {
            mDay[i].fx = "SW";
        }
        else if(mDay[i].fx == "西北风")
        {
            mDay[i].fx = "NW";
        }
    }

    for (int i = 0; i < 5; i++)
    {
        if (mDay[i].type == "暴雪")
        {
            mDay[i].type = "Blizzard";
        }
        else if (mDay[i].type == "暴雨")
        {
            mDay[i].type = "Hv Rain";
        }
        else if (mDay[i].type == "暴雨到大暴雨")
        {
            mDay[i].type = "Hv Rain to Tr Rain";
        }
        else if (mDay[i].type == "大暴雨")
        {
            mDay[i].type = "Tr Rain";
        }
        else if (mDay[i].type == "大暴雨到特大暴雨")
        {
            mDay[i].type = "Tr Rain to Ex Tr Rain";
        }
        else if (mDay[i].type == "大到暴雪")
        {
            mDay[i].type = "Hv Snow to Blizzard";
        }
        else if (mDay[i].type == "大雪")
        {
            mDay[i].type = "Hv Snow";
        }
        else if (mDay[i].type == "大雨")
        {
            mDay[i].type = "Hv Rain";
        }
        else if (mDay[i].type == "冻雨")
        {
            mDay[i].type = "Freezing Rain";
        }
        else if (mDay[i].type == "多云")
        {
            mDay[i].type = "Cloudy";
        }
        else if (mDay[i].type == "浮沉")
        {
            mDay[i].type = "Floating Dust";
        }
        else if (mDay[i].type == "雷阵雨")
        {
            mDay[i].type = "Thunderstorms";
        }
        else if (mDay[i].type == "雷阵雨伴有冰雹")
        {
            mDay[i].type = "Thunderstorms with Hail";
        }
        else if (mDay[i].type == "霾")
        {
            mDay[i].type = "Haze";
        }
        else if (mDay[i].type == "强沙尘暴")
        {
            mDay[i].type = "Severe Sandstorm";
        }
        else if (mDay[i].type == "晴")
        {
            mDay[i].type = "Sunny";
        }
        else if (mDay[i].type == "沙尘暴")
        {
            mDay[i].type = "Sandstorm";
        }
        else if (mDay[i].type == "特大暴雨")
        {
            mDay[i].type = "Ex Tr Rain";
        }
        else if (mDay[i].type == "undefined")
        {
            mDay[i].type = "undefined";
        }
        else if (mDay[i].type == "雾")
        {
            mDay[i].type = "Fog";
        }
        else if (mDay[i].type == "小到中雪")
        {
            mDay[i].type = "Lt Snow to Md Snow";
        }
        else if (mDay[i].type == "小到中雨")
        {
            mDay[i].type = "Lt Rain to Md Rain";
        }
        else if (mDay[i].type == "小雪")
        {
            mDay[i].type = "Lt Snow";
        }
        else if (mDay[i].type == "小雨")
        {
            mDay[i].type = "Lt Rain";
        }
        else if (mDay[i].type == "雪")
        {
            mDay[i].type = "Snow";
        }
        else if (mDay[i].type == "扬沙")
        {
            mDay[i].type = "Blowing Sand";
        }
        else if (mDay[i].type == "阴")
        {
            mDay[i].type = "Overcast";
        }
        else if (mDay[i].type == "雨")
        {
            mDay[i].type = "Rain";
        }
        else if (mDay[i].type == "雨夹雪")
        {
            mDay[i].type = "Sleet";
        }
        else if (mDay[i].type == "阵雪")
        {
            mDay[i].type = "Snow Showers";
        }
        else if (mDay[i].type == "阵雨")
        {
            mDay[i].type = "Rain Showers";
        }
        else if (mDay[i].type == "中到大雪")
        {
            mDay[i].type = "Md to Hv Snow";
        }
        else if (mDay[i].type == "中到大雨")
        {
            mDay[i].type = "Md to Hv Rain";
        }
        else if (mDay[i].type == "中雪")
        {
            mDay[i].type = "Md Snow";
        }
        else if (mDay[i].type == "中雨")
        {
            mDay[i].type = "Md Rain";
        }
    }

    //今日数据
    mToday.ganmao = objData.value("ganmao").toString();
    //mToday.wendu = objData.value("wendu").toString().toInt();
    mToday.wendu = objData.value("wendu").toString().toInt();
    mToday.pm25 = objData.value("pm25").toInt();
    mToday.quality = objData.value("quality").toString();

    // 首个数据为今日数据
    mToday.type = mDay[1].type;
    mToday.fx = mDay[1].fx;
    mToday.fl = mDay[1].fl;
    mToday.high = mDay[1].high;
    mToday.low = mDay[1].low;

    if(mToday.city == "上海市")
    {
        mToday.city = "Shanghai";
    }else if(mToday.city == "沈阳市")
    {
        mToday.city = "Shenyang";   
    }
    

    for(int i = 1; i < 5; i++)
    {
        if(mDay[i].week == "星期一")
        {
            mDay[i].week = "Mon.";
        }
        else if(mDay[i].week == "星期二")
        {
            mDay[i].week = "Tue.";
        }
        else if(mDay[i].week == "星期三")
        {
            mDay[i].week = "Wed.";
        }
        else if(mDay[i].week == "星期四")
        {
            mDay[i].week = "Thu.";
        }
        else if(mDay[i].week == "星期五")
        {
            mDay[i].week = "Fri.";
        }
        else if(mDay[i].week == "星期六")
        {
            mDay[i].week = "Sat.";
        }
        else if(mDay[i].week == "星期日")
        {
            mDay[i].week = "Sun.";
        }
    }


    // 更新UI界面
    updateUI();

    //绘制温度曲线
    ui->lblHighCurve->update();
    ui->lblLowCurve->update();
}

void MainWindow::updateUI()
{
    // 更新日期和城市
    QString lblcity = mToday.city;
    QString lbltype = mToday.type;
    QString lbltemp = QString::number((mToday.low+mToday.high)/2);
    //QString lbltemp = mToday.wendu;
    ui->lblDate->setText(QDateTime::fromString(mToday.date, "yyyy-MM-dd HH:mm:ss").toString("MM/dd HH:mm") + " " + mDay[1].week);


    ui->lblCity->setText(mToday.city);

    // 更新今天
//    ui->lblTypeIcon->setPixmap(mTypeMap[mToday.type]);
    ui->lblTypeIcon->setPixmap(mTypeMap[mToday.type].split(".").at(0)+"128.png");
    //ui->lblTemp->setText(QString::number(mToday.wendu) + "°");
    //mToday.wendu += 100;
    ui->lblTemp->setText(QString::number(mToday.low) + "℃");
    ui->lblType->setText(mToday.type);
    ui->lblLowHigh->setText(QString::number(mToday.low) + "~" +QString::number(mToday.high) + "℃");
    ui->lblGanMao->setText("感冒指数：" + mToday.ganmao);
    ui->lblWindFl->setText(mToday.fl);
    ui->lblWindFx->setText(mToday.fx);
    ui->lblPM25->setText(QString::number(mToday.pm25));
    ui->lblShiDu->setText(mToday.shidu);
    ui->lblQuality->setText(mToday.quality);

    //托盘悬停鼠标时显示信息
    QDateTime curTime = QDateTime::currentDateTime();
    QString tip = lblcity + "\n" + lbltype + "\n" + lbltemp + "℃" + "\n" + mToday.fx + " " + mToday.fl + "\n"+ curTime.toString("hh:mm:ss");
    msysTray->setToolTip(tip);
    //msysTray->setIcon(QIcon(mTypeMap[mToday.type].split(".").at(0)+"128.png"));

    // 更新六天
    ui->lblWeek0->setText("Ytd.");
    ui->lblWeek1->setText("Td.");
    ui->lblWeek2->setText("Tmr.");
    for(int i=0; i < 6; i++)
    {
        if(i > 2) {
            mWeekList[i]->setText("Week" + mDay[i].week.right(1));
        }

        QStringList ymdList = mDay[i].date.split("-");
        mDateList[i]->setText(ymdList[1] + "/" + ymdList[2]);

        //更新天气类型
        mTypeList[i]->setText(mDay[i].type);
        mTypeIconList[i]->setPixmap(mTypeMap[mDay[i].type]);
        //空气质量
        if(mDay[i].aqi > 0 && mDay[i].aqi <= 50) {
            mAqiList[i]->setText("Fine");
            mAqiList[i]->setStyleSheet("background-color: rgb(121, 184, 0);");
        } else if(mDay[i].aqi > 50 && mDay[i].aqi <= 100){
            mAqiList[i]->setText("Soso");
            mAqiList[i]->setStyleSheet("background-color: rgb(255, 183, 23);");
        } else if(mDay[i].aqi > 100 && mDay[i].aqi <= 150){
            mAqiList[i]->setText("Lt");
            mAqiList[i]->setStyleSheet("background-color: rgb(255, 87, 97);");
        } else if(mDay[i].aqi > 150 && mDay[i].aqi <= 200){
            mAqiList[i]->setText("Bad");
            mAqiList[i]->setStyleSheet("background-color: rgb(235, 17, 27);");
        } else if(mDay[i].aqi > 200 && mDay[i].aqi <= 250){
            mAqiList[i]->setText("Hv");
            mAqiList[i]->setStyleSheet("background-color: rgb(170, 0, 0);");
        } else {
            mAqiList[i]->setText("Ex");
            mAqiList[i]->setStyleSheet("background-color: rgb(110, 0, 0);");
        }

        //更新风力
        mFxList[i]->setText(mDay[i].fx);
        mFlList[i]->setText(mDay[i].fl);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->lblHighCurve && event->type() == QEvent::Paint){
        paintHighCurve();
    }
    if(watched == ui->lblLowCurve && event->type() == QEvent::Paint){
        paintLowCurve();
    }
}

void MainWindow::paintHighCurve()
{
    QPainter painter(ui->lblHighCurve);

    //平滑曲线 抗锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);

    //获取x坐标
    int point_x[6]={0};
    for(int i=0; i<6; i++)
    {
        point_x[i] = mWeekList[i]->pos().x() + mWeekList[i]->width()/2;
    }

    //获取y坐标
    int tempSum = 0;
    int tempAvg = 0;
    int yCenter = ui->lblHighCurve->height()/2;
    for(int i=0; i<6; i++)
    {
        tempSum += mDay[i].high;
    }
    tempAvg = tempSum / 6;

    int point_y[6] = {0};

    for(int i=0; i<6; i++)
    {
        point_y[i] = yCenter - ((mDay[i].high - tempAvg) * INCREMENT);
    }

    //开始绘制background-color: #FAEBD7;
    QPen pen = painter.pen();
    pen.setWidth(1);  //设置画笔宽度
    pen.setColor(QColor(249,101,84)); //设置画笔颜色
    painter.setPen(pen);
    painter.setBrush(QColor(249,101,84)); //设置画刷内部填充颜色
    for(int i=0; i<6; i++)
    {
        //显示点
        painter.drawEllipse(QPoint(point_x[i], point_y[i]), POINT_RADIUS, POINT_RADIUS);

        //显示文本
        painter.drawText(point_x[i] - TEXT_OFFSET_X, point_y[i] - TEXT_OFFSET_Y, QString::number(mDay[i].high) + "°");
    }

    //绘制曲线
    for(int i=0; i<5; i++)
    {
        if(i==0)
        {
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        }
        else
        {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        painter.drawLine(point_x[i], point_y[i], point_x[i+1], point_y[i+1]);
    }
}

void MainWindow::paintLowCurve()
{
    QPainter painter(ui->lblLowCurve);

    //平滑曲线 抗锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);

    //获取x坐标
    int point_x[6]={0};
    for(int i=0; i<6; i++)
    {
        point_x[i] = mWeekList[i]->pos().x() + mWeekList[i]->width()/2;
    }

    //获取y坐标
    int tempSum = 0;
    int tempAvg = 0;
    int yCenter = ui->lblLowCurve->height()/2;
    for(int i=0; i<6; i++)
    {
        tempSum += mDay[i].low;
    }
    tempAvg = tempSum / 6;

    int point_y[6] = {0};

    for(int i=0; i<6; i++)
    {
        point_y[i] = yCenter - ((mDay[i].low - tempAvg) * INCREMENT);
    }

    //开始绘制
    QPen pen = painter.pen();
    pen.setWidth(1);  //设置画笔宽度
    pen.setColor(QColor(56,120,218)); //设置画笔颜色
    painter.setPen(pen);
    painter.setBrush(QColor(56,120,218)); //设置画刷内部填充颜色
    for(int i=0; i<6; i++)
    {
        //显示点
        painter.drawEllipse(QPoint(point_x[i], point_y[i]), POINT_RADIUS, POINT_RADIUS);

        //显示文本
        painter.drawText(point_x[i] - TEXT_OFFSET_X, point_y[i] - TEXT_OFFSET_Y, QString::number(mDay[i].low) + "°");
    }

    //绘制曲线
    for(int i=0; i<5; i++)
    {
        if(i==0)
        {
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        }
        else
        {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        painter.drawLine(point_x[i], point_y[i], point_x[i+1], point_y[i+1]);
    }
}

void MainWindow::on_btnSearch_clicked()
{
    QString cityName = ui->leCity->text();
    getWeatherInfo(cityName);
}

