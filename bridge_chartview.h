#ifndef BRIDGE_CHARTVIEW_H
#define BRIDGE_CHARTVIEW_H

#include <QObject>
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtWidgets/QRubberBand>
#include <QDebug>
#include <QPointF>
#include <QTimer>
QT_CHARTS_USE_NAMESPACE
class bridge_chartview : public QChartView
{
    Q_OBJECT
public:
    bridge_chartview(QChart *chart, QWidget *parent = 0);

private:
    void keyPressEvent(QKeyEvent *event);

};

#endif // BRIDGE_CHARTVIEW_H
