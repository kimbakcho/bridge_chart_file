#ifndef BRIDGE_CHART_WIDGET_H
#define BRIDGE_CHART_WIDGET_H

#include <QWidget>
#include <bridge_chart.h>
#include <bridge_chartview.h>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QSqlError>
#include <QDir>
#include <smtp/SmtpMime>
#include <QThread>
#include <QEventLoop>
#include <QScatterSeries>
class send_email_data {
public:
    send_email_data();
    bool match_case[8];
    QStringList filenames;
    bool check_send_email();



};

namespace Ui {
class bridge_chart_widget;
}

class bridge_chart_widget : public QWidget
{
    Q_OBJECT

public:
    explicit bridge_chart_widget(QWidget *parent = 0);
    bridge_chart *main_chart;
    bridge_chartview *main_chartview;
    bridge_chart *KSQ_chart;
    bridge_chartview *KSQ_chartview;
    QTimer charttimer;
    QSqlDatabase my_db;
    QSqlDatabase ms_mes_db;
    QScatterSeries *value_series;
    QScatterSeries *ksqmatch_series_total;
    QLineSeries *ksqmatch_series_ksq;
    QLineSeries *CL_series;
    QLineSeries *USL_series[3];
    QLineSeries *LSL_series[3];
    QDateTimeAxis *axisX ;
    QValueAxis *axisY;
    QEventLoop nonblock;

    QDateTime dbcurrentdatetime;

    QLineSeries *KSQ_value_series;
    QLineSeries *KSQ_CL_series;
    QLineSeries *KSQ_USL_series[3];
    QLineSeries *KSQ_LSL_series[3];
    QDateTimeAxis *KSQ_axisX ;
    QValueAxis *KSQ_axisY;

    QString mail_subname;

    int run_count;

    int x;

    QString make_image_file(QDateTime currentdatetime, QWidget *pictuerwidget, QString pickture_name);

    void KSQ_chart_draw(int count, int errcount);

    void email_send(send_email_data *email_data);
    ~bridge_chart_widget();
public slots:
    void chart_timer_timeout();

    void nonblock_timer_timeout();

private slots:

    void on_chart_timer_stop_btn_clicked();

    void on_chart_timer_start_btn_clicked();

    void on_zoom_reset_btn_clicked();

private:
    Ui::bridge_chart_widget *ui;
};

#endif // BRIDGE_CHART_WIDGET_H
