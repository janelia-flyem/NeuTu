#ifndef Z3DTABWIDGET_H
#define Z3DTABWIDGET_H

#include <QTabWidget>

class Z3DWindow;

class Z3DTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit Z3DTabWidget(QWidget* parent = 0);
    ~Z3DTabWidget();
    QTabBar* tabBar();

    void addWindow(int index, Z3DWindow *window, const QString &title);
    int getTabIndex(int index);
    int getRealIndex(int index);

public slots:
    void closeWindow(int index);
    void updateTabs(int index);
    void updateWindow(int index);
    void closeAllWindows();

public slots:
    void resetCamera();
    void setXZView();
    void setYZView();

    void settingsPanel(bool v);
    void objectsPanel(bool v);
    void roiPanel(bool v);
    void showGraph(bool v);

    void resetSettingsButton();
    void resetObjectsButton();
    void resetROIButton();

    void resetCameraCenter();

signals:
    void buttonShowGraphToggled(bool);
    void buttonSettingsToggled(bool);
    void buttonObjectsToggled(bool);
    void buttonROIsToggled(bool);
    void buttonROIsClicked();

    void tabIndexChanged(int);

private:
    bool buttonStatus[4][4]; // 0-coarsebody 1-body 2-skeleton 3-synapse 0-showgraph 1-settings 2-objects 3-rois
    bool windowStatus[4]; // 0-coarsebody 1-body 2-skeleton 3-synapse false-closed true-opened
    int tabLUT[4]; // tab index look up table
    int preIndex;

};


#endif // Z3DTABWIDGET_H
