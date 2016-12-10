#ifndef PRESENTATIONWINDOW_H
#define PRESENTATIONWINDOW_H

#include <QMainWindow>

namespace Ui {
class SceneWindow;
}

class SceneWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SceneWindow(QWidget *parent = nullptr): QMainWindow(parent) {}
    ~SceneWindow();

    void setupUi(QMainWindow* menuWindow);

private slots:
    void onApplyOnePrismSetup();
    void onApplyTwoPrismsSetup();

    void onFirstPrismRotationChanged(int value);
    void onFirstPrismAngleChanged(int value);
    void onSecondPrismAnomalousToggled(bool value);

    void onAbsorptionSpectrumCenterChanged(int value);
    void onAbsorptionSpectrumWidthChanged(int value);
    void onAbsorptionSpectrumHeightChanged(int value);
public:
	void setSecondPrismAnomalous(bool enable);
	void setSecondPrismEnabled(bool enable);

private:
	int defaultFirstPrismRotation, defaultFirstPrismAngle;
	int defaultSecondPrismRotation, defaultSecondPrismAngle;
	bool defaultSecondPrismAnomalous;
	int defaultAbsorptionSpectrumCenter;
	int defaultAbsorptionSpectrumWidth;
	int defaultAbsorptionSpectrumHeight;
	bool skip = false;
	bool second = false;
    Ui::SceneWindow *ui = nullptr;
};

extern SceneWindow* gScene;

#endif // PRESENTATIONWINDOW_H
