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
    explicit SceneWindow(QWidget *parent = Q_NULLPTR): QMainWindow(parent) {}
    ~SceneWindow();

    void setupUi(QMainWindow* menuWindow);

private slots:
    void onApplyOnePrismSetup();
    void onApplyTwoPrismsSetup();

    void onFirstPrismRotationChanged(int value);
    void onFirstPrismAngleChanged(int value);
    void onSecondPrismRotationChanged(int value);
    void onSecondPrismAngleChanged(int value);
    void onSecondPrismAnomDispEnable(bool value);

    void onAbsorptionSpectrumCenterChanged(int value);
    void onAbsorptionSpectrumWidthChanged(int value);
    void onAbsorptionSpectrumHeightChanged(int value);
private:
    void setSecondPrismEnabled(bool enable);
    Ui::SceneWindow *ui;
};

#endif // PRESENTATIONWINDOW_H
