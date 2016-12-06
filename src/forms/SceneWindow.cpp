#include "SceneWindow.h"
#include "ui_SceneWindow.h"

#include "PresentationScene.h"

SceneWindow::~SceneWindow()
{
    delete ui;
}

void SceneWindow::setupUi(QMainWindow* menuWindow)
{
    ui = new Ui::SceneWindow();
    ui->setupUi(this);

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
    connect(ui->checkBoxSceneParamsSecondPrismAnomalous, &QCheckBox::toggled,
            this, &SceneWindow::onSecondPrismAngleChanged);

    connect(ui->sliderAbspSpectrumCenter, &QSlider::valueChanged,
            this, &SceneWindow::onAbsorptionSpectrumCenterChanged);
    connect(ui->sliderAbspSpectrumWidth, &QSlider::valueChanged,
            this, &SceneWindow::onAbsorptionSpectrumWidthChanged);
    connect(ui->sliderAbspSpectrumHeight, &QSlider::valueChanged,
            this, &SceneWindow::onAbsorptionSpectrumHeightChanged);

    setSecondPrismEnabled(false);
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
    PrScene* scene = dynamic_cast<PrScene*>(ui->sceneWidget->mScene);
    Q_ASSERT(scene != nullptr);

    PrPrismRenderer& firstPrism = scene->mPrismRenderers.first();
    QVector3D rotationDegrees = firstPrism.rotationDegrees();
    rotationDegrees.setX((float)value);
    firstPrism.setRotationDegrees(-rotationDegrees);
}
void SceneWindow::onFirstPrismAngleChanged(int value)
{
    PrScene* scene = dynamic_cast<PrScene*>(ui->sceneWidget->mScene);
    Q_ASSERT(scene != nullptr);

    PrPrismRenderer& firstPrism = scene->mPrismRenderers.first();
    firstPrism.setAngle((float)value);
}
void SceneWindow::onSecondPrismRotationChanged(int value)
{
    PrScene* scene = dynamic_cast<PrScene*>(ui->sceneWidget->mScene);
    Q_ASSERT(scene != nullptr);

    PrPrismRenderer& secondPrism = scene->mPrismRenderers.last();
    QVector3D rotationDegrees = secondPrism.rotationDegrees();
    rotationDegrees.setX((float)value);
    secondPrism.setRotationDegrees(-rotationDegrees);
}
void SceneWindow::onSecondPrismAngleChanged(int value)
{
    PrScene* scene = dynamic_cast<PrScene*>(ui->sceneWidget->mScene);
    Q_ASSERT(scene != nullptr);

    PrPrismRenderer& secondPrism = scene->mPrismRenderers.last();
    secondPrism.setAngle((float)value);
}
void SceneWindow::onSecondPrismAnomDispEnable(bool value)
{
}

void SceneWindow::onAbsorptionSpectrumCenterChanged(int value)
{
}
void SceneWindow::onAbsorptionSpectrumWidthChanged(int value)
{
}
void SceneWindow::onAbsorptionSpectrumHeightChanged(int value)
{
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void SceneWindow::setSecondPrismEnabled(bool enable)
{
    PrScene* scene = dynamic_cast<PrScene*>(ui->sceneWidget->mScene);
    if (scene)
    {
        ui->chartRefractiveIndex->bindWithComplexFunction(scene->mPrismRenderers[enable].mFirstPlane.refractiveIndex());
    }

    QWidget* widgetsToEnable[] = {
        ui->labelSecondPrism, ui->labelSecondPrismRotation, ui->labelSecondPrismAngle,
        ui->spinBoxSecondPrismAngle, ui->spinBoxSecondPrismRotation, ui->checkBoxSceneParamsSecondPrismAnomalous,

        ui->comboBoxMaterial, ui->pushButtonMaterialSelect,

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
