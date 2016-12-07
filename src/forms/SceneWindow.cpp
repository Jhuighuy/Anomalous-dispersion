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

    connect(ui->pushButtonToMenu, &QPushButton::clicked, this, &QMainWindow::hide);
    connect(ui->pushButtonToMenu, &QPushButton::clicked, menuWindow, &QMainWindow::showMaximized);
    connect(ui->pushButtonSetupOnePrism, &QPushButton::clicked, this, &SceneWindow::onApplyOnePrismSetup);
    connect(ui->pushButtonSetupTwoPrisms, &QPushButton::clicked, this, &SceneWindow::onApplyTwoPrismsSetup);

    connect(ui->spinBoxFirstPrismRotation, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &SceneWindow::onFirstPrismRotationChanged);
    connect(ui->spinBoxFirstPrismAngle, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &SceneWindow::onFirstPrismAngleChanged);
    connect(ui->spinBoxSecondPrismRotation, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &SceneWindow::onSecondPrismRotationChanged);
    connect(ui->spinBoxSecondPrismAngle, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &SceneWindow::onSecondPrismAngleChanged);
    connect(ui->checkBoxSecondPrismAnomalous, &QCheckBox::toggled,
            this, &SceneWindow::onSecondPrismAnomalousToggled);

    connect(ui->sliderAbspSpectrumCenter, &QSlider::valueChanged,
            this, &SceneWindow::onAbsorptionSpectrumCenterChanged);
    connect(ui->sliderAbspSpectrumWidth, &QSlider::valueChanged,
            this, &SceneWindow::onAbsorptionSpectrumWidthChanged);
    connect(ui->sliderAbspSpectrumHeight, &QSlider::valueChanged,
            this, &SceneWindow::onAbsorptionSpectrumHeightChanged);

	defaultFirstPrismRotation = ui->spinBoxFirstPrismRotation->value(), defaultFirstPrismAngle = ui->spinBoxFirstPrismAngle->value();
	defaultSecondPrismRotation = ui->spinBoxSecondPrismRotation->value(), defaultSecondPrismAngle  = ui->spinBoxSecondPrismAngle->value();
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

    PrPrismRenderer_p firstPrism = scene->firstPrism();
    QVector3D rotationDegrees = firstPrism->rotationDegrees();
    rotationDegrees.setX(static_cast<float>(-value));
    firstPrism->setRotationDegrees(rotationDegrees);
	scene->recalculateBeams();
}
void SceneWindow::onFirstPrismAngleChanged(int value)
{
	PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
	Q_ASSERT(scene != nullptr);

	PrPrismRenderer_p firstPrism = scene->firstPrism();
    firstPrism->setAngle(static_cast<float>(value));
	scene->recalculateBeams();
}
void SceneWindow::onSecondPrismRotationChanged(int value)
{
	PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
	Q_ASSERT(scene != nullptr);

	//! @todo
	PrPrismRenderer_p secondPrism = scene->secondPrism();
	QVector3D rotationDegrees = secondPrism->rotationDegrees();
	rotationDegrees.setX(static_cast<float>(-value));
	secondPrism->setRotationDegrees(rotationDegrees);
	scene->recalculateBeams();
}
void SceneWindow::onSecondPrismAngleChanged(int value)
{
	PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
    Q_ASSERT(scene != nullptr);

	PrPrismRenderer_p secondPrism = scene->firstPrism();
	secondPrism->setAngle(static_cast<float>(value));
	scene->recalculateBeams();
}
void SceneWindow::onSecondPrismAnomalousToggled(bool value)
{
	PrScene_p scene = ui->sceneWidget->scene().dynamicCast<PrScene>();
	Q_ASSERT(scene != nullptr);

	PrPrismRenderer_p secondPrism = scene->secondPrism();
	secondPrism->setAnomaluos(value);
	scene->recalculateBeams();
	if (value)
	{
		ui->chartRefractiveIndex->bindWithComplexFunction(scene->secondPrism()->refractiveIndex());
	}
	else
	{
		ui->chartRefractiveIndex->bindWithComplexFunction(scene->firstPrism()->refractiveIndex());
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
		scene->secondPrism()->setAbsorptionIndexWidth(static_cast<float>(value));
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
	for (QWidget* widget : widgetsToEnable)
	{
		widget->setEnabled(enable);
	}
}
void SceneWindow::setSecondPrismEnabled(bool enable)
{
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
	ui->spinBoxSecondPrismRotation->setValue(defaultSecondPrismRotation), ui->spinBoxSecondPrismAngle->setValue(defaultSecondPrismAngle);
	if (defaultSecondPrismAnomalous != ui->checkBoxSecondPrismAnomalous->isChecked())
		ui->checkBoxSecondPrismAnomalous->toggle();
	ui->sliderAbspSpectrumCenter->setValue(defaultAbsorptionSpectrumCenter);
	ui->sliderAbspSpectrumWidth->setValue(defaultAbsorptionSpectrumWidth);
	ui->sliderAbspSpectrumHeight->setValue(defaultAbsorptionSpectrumHeight);

    QWidget* widgetsToEnable[] = {
        ui->labelAbspSpectrumCenter, ui->labelAbspSpectrumCenterMin, ui->labelAbspSpectrumCenterMid, ui->labelAbspSpectrumCenterMax,
        ui->sliderAbspSpectrumCenter,
        ui->labelAbspSpectrumWidth, ui->labelAbspSpectrumWidthMin, ui->labelAbspSpectrumWidthMid, ui->labelAbspSpectrumWidthMax,
        ui->sliderAbspSpectrumWidth,
        ui->labelAbspSpectrumHeight, ui->labelAbspSpectrumHeightMin, ui->labelAbspSpectrumHeightMid, ui->labelAbspSpectrumHeightMax,
        ui->sliderAbspSpectrumHeight,
    };
    for (QWidget* widget : widgetsToEnable)
    {
        widget->setEnabled(enable);
    }

	setSecondPrismAnomalous(ui->checkBoxSecondPrismAnomalous->isChecked());
}

