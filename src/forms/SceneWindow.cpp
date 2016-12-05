#include "SceneWindow.h"
#include "ui_SceneWindow.h"

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
}
void SceneWindow::onFirstPrismAngleChanged(int value)
{
}
void SceneWindow::onSecondPrismRotationChanged(int value)
{
}
void SceneWindow::onSecondPrismAngleChanged(int value)
{
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
