#ifndef PLOTVIEWWIDGET_H
#define PLOTVIEWWIDGET_H

#include <QChartView>

class ScPlotViewWidget : public QtCharts::QChartView
{
public:
    ScPlotViewWidget(QWidget* parent = Q_NULLPTR):
        QtCharts::QChartView(parent)
    {
    }
};

#endif // PLOTVIEWWIDGET_H
