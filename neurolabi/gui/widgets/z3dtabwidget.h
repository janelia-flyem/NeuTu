#ifndef Z3DTABWIDGET_H
#define Z3DTABWIDGET_H

#include <QTabWidget>

class Z3DWindow;
class ZROIWidget;

#define WINDOW3D_COUNT 10 //Support up to 10 tabs

/*!
 * \brief The class of tab widget of managing 3D windows.
 *
 * Each window is associated with a rank when it's added to the tab widget.
 * There is a map from index to rank and vice versa. The ranks and indices have
 * the same order for exisiting windows.
 */
class Z3DTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit Z3DTabWidget(QWidget* parent = 0);
    ~Z3DTabWidget();
    QTabBar* tabBar();

    void addWindow(int rank, Z3DWindow *window, const QString &title);

    int getTabIndex(int rank);

    //Get the window rank
    int getWindowRank(int index);

    Z3DWindow* getCurrentWindow() const;

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
    ZROIWidget *roiPanel(bool v);
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
    bool windowExists(int rank);
    Z3DWindow* getWindowFromIndex(int index) const;
    void resetButtonStatus(int rank);
    Z3DWindow* removeWindow(int index);

private:
    bool m_buttonStatus[WINDOW3D_COUNT][4]; // 0-coarsebody 1-body 2-skeleton 3-synapse 0-showgraph 1-settings 2-objects 3-rois
    bool m_windowAdded[WINDOW3D_COUNT]; // 0-coarsebody 1-body 2-skeleton 3-synapse false-closed true-opened
    int m_rankToIndex[WINDOW3D_COUNT]; // tab index look up table (from window rank to tab index)
    int m_preRank;

};


#endif // Z3DTABWIDGET_H
