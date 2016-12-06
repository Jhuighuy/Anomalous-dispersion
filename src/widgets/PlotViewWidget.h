#ifndef PLOTVIEWWIDGET_H
#define PLOTVIEWWIDGET_H

#include <QChartView>
#include "PresentationPhysics.h"

class ScPlotViewWidget : public QtCharts::QChartView
{
public:
    ScPlotViewWidget(QWidget* parent = nullptr):
        QtCharts::QChartView(parent)
    {
    }

    void bindWithComplexFunction(PhComplexIndexFunction_p indexFunction)
    {
        mIndexFunction = indexFunction;
        QtCharts::QLineSeries* lineSeries = new QtCharts::QLineSeries();
        for (qreal x = 0.38; x < 0.78; x += 0.01)
        {
            lineSeries->append(x, indexFunction->real(x));
        }
        lineSeries->setName("Govno");

        QtCharts::QLineSeries* lineSeries2 = new QtCharts::QLineSeries();
        for (qreal x = 0.38; x < 0.78; x += 0.01)
        {
            lineSeries2->append(x, indexFunction->imaginary(x));
        }
        lineSeries2->setName("Jopa");

        QtCharts::QChart* chart = new QtCharts::QChart();
        chart->addSeries(lineSeries);
        chart->addSeries(lineSeries2);
        setChart(chart);
    }

    PhComplexIndexFunction_p mIndexFunction;
private:
};

#endif // PLOTVIEWWIDGET_H
