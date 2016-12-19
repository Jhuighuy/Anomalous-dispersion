#include "SceneWindow.h"
#include "ui_SceneWindow.h"

#include "PresentationScene.h"

SceneWindow* gScene;

SceneWindow::~SceneWindow()
{
    delete ui;
}

void SceneWindow::setupUi(QMainWindow* menuWindow)
{
	gScene = this;

    ui = new Ui::SceneWindow();
    ui->setupUi(this);
	ui->sceneWidget->setScene(PrScene::create());

    connect(ui->pushButtonToMenu, &QPushButton::clicked, qApp, &QCoreApplication::quit);
    connect(ui->pushButtonSetupOnePrism, &QPushButton::clicked, this, &SceneWindow::onApplyOnePrismSetup);
    connect(ui->pushButtonSetupTwoPrisms, &QPushButton::clicked, this, &SceneWindow::onApplyTwoPrismsSetup);

    connect(ui->spinBoxFirstPrismRotation, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &SceneWindow::onFirstPrismRotationChanged);
    connect(ui->spinBoxFirstPrismAngle, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &SceneWindow::onFirstPrismAngleChanged);
    connect(ui->checkBoxSecondPrismAnomalous, &QCheckBox::toggled,
            this, &SceneWindow::onSecondPrismAnomalousToggled);

    connect(ui->sliderAbspSpectrumCenter, &QSlider::valueChanged,
            this, &SceneWindow::onAbsorptionSpectrumCenterChanged);
    connect(ui->sliderAbspSpectrumWidth, &QSlider::valueChanged,
            this, &SceneWindow::onAbsorptionSpectrumWidthChanged);
    connect(ui->sliderAbspSpectrumHeight, &QSlider::valueChanged,
            this, &SceneWindow::onAbsorptionSpectrumHeightChanged);

	defaultFirstPrismRotation = ui->spinBoxFirstPrismRotation->value(), defaultFirstPrismAngle = ui->spinBoxFirstPrismAngle->value();
	defaultSecondPrismAnomalous = ui->checkBoxSecondPrismAnomalous->isChecked();
	defaultAbsorptionSpectrumCenter = ui->sliderAbspSpectrumCenter->value();
	defaultAbsorptionSpectrumWidth = ui->sliderAbspSpectrumWidth->value();
	defaultAbsorptionSpectrumHeight = ui->sliderAbspSpectrumHeight->value();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void SceneWindow::onApplyOnePrismSetup()
{
    setSecondPrismEnabled(false);
}
void SceneWindow::onApplyTwoPrismsSetup()
{
    setSecondPrismEnabled(true);
}

void SceneWindow::onFirstPrismRotationChanged(int value)
{
    PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
    Q_ASSERT(scene != nullptr);

    PrPrismRenderer_p firstPrism = second ? scene->secondPrism() : scene->firstPrism();
    QVector3D rotationDegrees = firstPrism->rotationDegrees();
	if (second)
		rotationDegrees.setY(static_cast<float>(-value));
	else
		rotationDegrees.setX(static_cast<float>(-value));
    firstPrism->setRotationDegrees(rotationDegrees);
	scene->recalculateBeams();
}
void SceneWindow::onFirstPrismAngleChanged(int value)
{
	PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
	Q_ASSERT(scene != nullptr);

	PrPrismRenderer_p firstPrism = second ? scene->secondPrism() : scene->firstPrism();
	firstPrism->setAngle(static_cast<float>(value));
	scene->recalculateBeams();
}
void SceneWindow::onSecondPrismAnomalousToggled(bool value)
{
	PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
	Q_ASSERT(scene != nullptr);

    PrPrismRenderer_p firstPrism = scene->firstPrism();
    PrPrismRenderer_p secondPrism = scene->secondPrism();

    //! @todo Олег, исправь хуйню, которую я тут нахуярил, все работает, но хуй знает, че тебе там по вкусу
    auto firstRotationDegrees = firstPrism->rotationDegrees();
    auto secondRotationDegrees = secondPrism->rotationDegrees();
    firstRotationDegrees.setX(value? 20.0f : 0.0f);
    secondRotationDegrees.setY(value? 20.0f : 0.0f);
    firstPrism->setRotationDegrees(firstRotationDegrees);
    secondPrism->setRotationDegrees(secondRotationDegrees);

	secondPrism->setAnomaluos(value);
	scene->recalculateBeams();
	if (value)
	{
		ui->chartRefractiveIndex->bindWithComplexFunction(scene->secondPrism()->refractiveIndex());
	}
	else
	{
		ui->chartRefractiveIndex->bindWithComplexFunction(scene->firstPrism()->refractiveIndex(), 0.9);
	}

	skip = true;
    ui->sliderAbspSpectrumCenter->setValue(defaultAbsorptionSpectrumCenter);
    ui->sliderAbspSpectrumWidth->setValue(defaultAbsorptionSpectrumWidth);
	ui->sliderAbspSpectrumHeight->setValue(defaultAbsorptionSpectrumHeight);
	skip = false;

	setSecondPrismAnomalous(value);
}

void SceneWindow::onAbsorptionSpectrumCenterChanged(int value)
{
	if (skip)
	{
		return;
	}

	qreal valueMcm = value / 1000.0;

	PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
	if (scene != nullptr)
	{
		scene->secondPrism()->setAbsorptionIndexCenter(valueMcm);
		scene->recalculateBeams();
		ui->chartRefractiveIndex->bindWithComplexFunction(scene->secondPrism()->refractiveIndex());
	}
}
void SceneWindow::onAbsorptionSpectrumWidthChanged(int value)
{
	if (skip)
	{
		return;
	}

	PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
	if (scene != nullptr)
	{
        qreal valueMCM = value / 100000.0;
        qreal v = 4.0 / (valueMCM * valueMCM);
		qDebug() << v;

		scene->secondPrism()->setAbsorptionIndexWidth(v);
		scene->recalculateBeams();
		ui->chartRefractiveIndex->bindWithComplexFunction(scene->secondPrism()->refractiveIndex());
	}
}
void SceneWindow::onAbsorptionSpectrumHeightChanged(int value)
{
	if (skip)
	{
		return;
	}

	PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
	if (scene != nullptr)
	{
		scene->secondPrism()->setAbsorptionIndexHeight(value / 100.0f);
		scene->recalculateBeams();
		ui->chartRefractiveIndex->bindWithComplexFunction(scene->secondPrism()->refractiveIndex());
	}
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void SceneWindow::setSecondPrismAnomalous(bool enable)
{
	QWidget* widgetsToEnable[] = {
		ui->labelAbspSpectrumCenter, ui->labelAbspSpectrumCenterMin, ui->labelAbspSpectrumCenterMid, ui->labelAbspSpectrumCenterMax,
		ui->sliderAbspSpectrumCenter,
		ui->labelAbspSpectrumWidth, ui->labelAbspSpectrumWidthMin, ui->labelAbspSpectrumWidthMid, ui->labelAbspSpectrumWidthMax,
		ui->sliderAbspSpectrumWidth,
		ui->labelAbspSpectrumHeight, ui->labelAbspSpectrumHeightMin, ui->labelAbspSpectrumHeightMid, ui->labelAbspSpectrumHeightMax,
		ui->sliderAbspSpectrumHeight,


    };
    QWidget* widgetsToDisable[] = {
        ui->labelFirstPrism, ui->labelFirstPrismAngle, ui->labelFirstPrismRotation, ui->spinBoxFirstPrismAngle,
        ui->spinBoxFirstPrismRotation
    };
	for (QWidget* widget : widgetsToEnable)
	{
		widget->setEnabled(enable);
    }

    for(QWidget* widget: widgetsToDisable)
    {
        widget->setEnabled(!enable);
    }

	if (enable)
	{
		ui->imageLabelPlotAbsp->show();
		ui->labelPlotAbsp->show();
	}
	else
	{
		ui->imageLabelPlotAbsp->hide();
		ui->labelPlotAbsp->hide();
    }


}
void SceneWindow::setSecondPrismEnabled(bool enable)
{
	QFont font1 = ui->pushButtonSetupOnePrism->font();
	QFont font2 = ui->pushButtonSetupTwoPrisms->font();
	font1.setBold(!enable);
	font2.setBold(enable);
	ui->pushButtonSetupOnePrism->setFont(font1);
	ui->pushButtonSetupTwoPrisms->setFont(font2);

	second = enable;
	char static const one[] = "\320\237\320\265\321\200\320\262\320\260\321\217 \320\277\321\200\320\270\320\267\320\274\320\260";
	char static const two[] = "\320\222\321\202\320\276\321\200\320\260\321\217 \320\277\321\200\320\270\320\267\320\274\320\260";
	ui->labelFirstPrism->setText(enable ? two : one);

	PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
	if (scene != nullptr)
    {
		if (enable)
		{
			scene->setTwoPrismsScene();
			ui->chartRefractiveIndex->bindWithComplexFunction(scene->secondPrism()->refractiveIndex());
		}
		else
		{
			scene->setOnePrismScene();
			ui->chartRefractiveIndex->bindWithComplexFunction(scene->firstPrism()->refractiveIndex());
		}
		scene->recalculateBeams();
	}

	ui->spinBoxFirstPrismRotation->setValue(defaultFirstPrismRotation), ui->spinBoxFirstPrismAngle->setValue(defaultFirstPrismAngle);
	if (defaultSecondPrismAnomalous != ui->checkBoxSecondPrismAnomalous->isChecked())
		ui->checkBoxSecondPrismAnomalous->toggle();
	ui->sliderAbspSpectrumCenter->setValue(defaultAbsorptionSpectrumCenter);
	ui->sliderAbspSpectrumWidth->setValue(defaultAbsorptionSpectrumWidth);
	ui->sliderAbspSpectrumHeight->setValue(defaultAbsorptionSpectrumHeight);

    QWidget* widgetsToEnable[] = {
		ui->checkBoxSecondPrismAnomalous,
    };
    for (QWidget* widget : widgetsToEnable)
    {
        widget->setEnabled(enable);
    }

	setSecondPrismAnomalous(ui->checkBoxSecondPrismAnomalous->isChecked());
}

