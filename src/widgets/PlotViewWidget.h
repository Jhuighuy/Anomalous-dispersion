#ifndef PLOTVIEWWIDGET_H
#define PLOTVIEWWIDGET_H

#include <QChartView>
#include <QValueAxis>
#include <QSplineSeries>

#include "PresentationPhysics.h"

class ScPlotViewWidget : public QtCharts::QChartView
{
public:
    ScPlotViewWidget(QWidget* parent = nullptr):
        QtCharts::QChartView(parent)
    {
    }

    void bindWithComplexFunction(PhComplexIndexFunction_p indexFunction, qreal c = 1)
    {
		static const QString spec("\320\224\320\273\320\270\320\275\320\260 \320\262\320\276\320\273\320\275\321\213 (\320\275\320\274)");
		static const QString refr("\320\237\320\276\320\272\320\260\320\267\320\260\321\202\320\265\320\273\321\214 \320\277\321\200\320\265\320\273\320\276\320\274\320\273\320\265\320\275\320\270\321\217");
        //!@todo change this title to коэффициент преломления кек
        static const QString absp("\320\232\320\276\321\215\321\204\321\204\320\270\321\206\320\270\320\265\320\275\321\202 \320\277\320\276\320\263\320\273\320\276\321\211\320\265\320\275\320\270\321\217");

		QtCharts::QChart* chart = new QtCharts::QChart();
        chart->legend()->hide();

		QtCharts::QValueAxis* axisX = new QtCharts::QValueAxis();
		QtCharts::QValueAxis* axisY = new QtCharts::QValueAxis();
		QtCharts::QValueAxis* axisY2 = new QtCharts::QValueAxis();

		axisX->setRange(380, 780);
		axisX->setTickCount(5);
		axisX->setTitleText(spec);
		axisY->setTitleText(refr);
		axisY2->setTitleText(absp);
		axisX->setGridLineVisible(false);
		axisY->setGridLineVisible(false);
		axisY2->setGridLineVisible(false);



		QPen pen;
		pen.setColor(Qt::black);
		pen.setWidth(5);

		// ----------------------
		{
			QLinearGradient chartAreaGradient;
			chartAreaGradient.setStart(QPointF(0, 0.1));
			chartAreaGradient.setFinalStop(QPointF(1, 0.1));

			for (qreal x = 0.38; x < 0.78; x += 0.01)
			{
				chartAreaGradient.setColorAt((x - 0.38) / (0.78 - 0.38),
					PhSpectrum::convertWavelengthToColorRGBA(x));
			}

			chartAreaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
			chart->setPlotAreaBackgroundBrush(chartAreaGradient);
			
			chart->setPlotAreaBackgroundVisible(true);
		}

		// ----------------------
        qreal minReal = 1000.0, maxReal = -1000.0;
        qreal minImaginary = 1000.0, maxImaginary = -1000.0;

        mIndexFunction = indexFunction;
		QtCharts::QSplineSeries* refrIndexSeries = new QtCharts::QSplineSeries();
		QtCharts::QSplineSeries* refrIndexSeries2 = new QtCharts::QSplineSeries();
		refrIndexSeries->setColor(Qt::white);
		refrIndexSeries2->setPen(pen);
		for (qreal x = 0.38; x < 0.78; x += 0.01)
        {
			qreal y = indexFunction->real(x) / c;
            minReal = qMin(minReal, y);
            maxReal = qMax(maxReal, y);
			refrIndexSeries->append(x * 1000.0, y);
			refrIndexSeries2->append(x * 1000.0, y);
		}
		//chart->addSeries(refrIndexSeries2);
		chart->addSeries(refrIndexSeries);

		QtCharts::QSplineSeries* abspIndexSeries = nullptr;
		QtCharts::QSplineSeries* abspIndexSeries2 = nullptr;
		if (indexFunction->imaginaryPart() != nullptr)
		{
			abspIndexSeries = new QtCharts::QSplineSeries();
			abspIndexSeries2 = new QtCharts::QSplineSeries();
			
			QPen p = abspIndexSeries->pen();
			p.setColor(Qt::black);
			abspIndexSeries->setPen(p);
			abspIndexSeries2->setPen(pen);

			for (qreal x = 0.38; x < 0.78; x += 0.01)
			{
				qreal y = indexFunction->imaginary(x);
                minImaginary = qMin(minImaginary, y);
                maxImaginary = qMax(maxImaginary, y);
				abspIndexSeries->append(x * 1000.0, y);
				abspIndexSeries2->append(x * 1000.0, y);
			}
			//chart->addSeries(abspIndexSeries2);
			chart->addSeries(abspIndexSeries);
		}

		axisX->setLabelFormat("%i");
		axisY->setTickCount(5);

		chart->addAxis(axisX, Qt::AlignBottom);
		chart->addAxis(axisY, Qt::AlignLeft);
		if (abspIndexSeries)
		{
			chart->addAxis(axisY2, Qt::AlignRight);
		}

        /*axisY->setRange(minReal, maxReal + 0.02);
        if (abspIndexSeries)
		{
            axisY2->setRange(minImaginary, maxImaginary + 0.02);
        } */

        if (abspIndexSeries)
        {
            axisY->setRange(1.0f, 1.6f);
            axisY2->setRange(0.0f, 0.45f);
        }
        else
        {
            axisY->setRange(minReal, maxReal + 0.02f);
        }

		refrIndexSeries->attachAxis(axisX);
		refrIndexSeries->attachAxis(axisY);
		refrIndexSeries2->attachAxis(axisX);
		refrIndexSeries2->attachAxis(axisY);

		if (abspIndexSeries)
		{
			abspIndexSeries->attachAxis(axisX);
			abspIndexSeries->attachAxis(axisY2);

			abspIndexSeries2->attachAxis(axisX);
			abspIndexSeries2->attachAxis(axisY2);
		}

		setChart(chart);
		setRenderHint(QPainter::Antialiasing);
    }

	void rebind()
    {
		bindWithComplexFunction(mIndexFunction);
    }

    PhComplexIndexFunction_p mIndexFunction;
private:
};

#endif // PLOTVIEWWIDGET_H
