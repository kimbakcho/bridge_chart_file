#ifndef BRIDGE_CHART_H
#define BRIDGE_CHART_H

#include <QObject>
#include <QWidget>

#include <QtCharts/QChart>
#include <QLineSeries>
#include <QValueAxis>
QT_CHARTS_USE_NAMESPACE

class bridge_chart : public QChart
{
    Q_OBJECT
public:
    bridge_chart(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
};

#endif // BRIDGE_CHART_H
