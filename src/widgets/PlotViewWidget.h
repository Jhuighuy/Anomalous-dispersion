#ifndef PLOTVIEWWIDGET_H
#define PLOTVIEWWIDGET_H

#include <QChartView>
#include <QSplineSeries>

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
		QtCharts::QChart* chart = new QtCharts::QChart();

        mIndexFunction = indexFunction;
        QtCharts::QSplineSeries* lineSeries = new QtCharts::QSplineSeries();
		for (qreal x = 0.38; x < 0.78; x += 0.01)
        {
            lineSeries->append(x * 1000.0, indexFunction->real(x));
		}
        lineSeries->setName("Govno");
		chart->addSeries(lineSeries);

		if (indexFunction->imaginaryPart() != nullptr)
		{
			QtCharts::QSplineSeries* lineSeries2 = new QtCharts::QSplineSeries();
			for (qreal x = 0.38; x < 0.78; x += 0.01)
			{
				lineSeries2->append(x * 1000.0, indexFunction->imaginary(x));
			}
			lineSeries2->setName("Jopa");
			chart->addSeries(lineSeries2);
		}

		chart->createDefaultAxes();
		setChart(chart);
    }

	void rebind()
    {
		bindWithComplexFunction(mIndexFunction);
    }

    PhComplexIndexFunction_p mIndexFunction;
private:
};

#endif // PLOTVIEWWIDGET_H
