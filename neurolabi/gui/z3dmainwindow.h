#ifndef Z3DMAINWINDOW_H
#define Z3DMAINWINDOW_H

#include <QMainWindow>

class Z3DTabWidget;
class Z3DWindow;
class QAction;

class Z3DMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    Z3DMainWindow(QWidget* parent = 0);
    ~Z3DMainWindow();

    void closeEvent(QCloseEvent *event);

    void setCurrentWidow(Z3DWindow *window);

public:
    QToolBar *toolBar;

public:
    QAction *resetCameraAction;
    QAction *xzViewAction;
    QAction *yzViewAction;
    QAction *recenterAction;
    QAction *showGraphAction;
    QAction *settingsAction;
    QAction *objectsAction;
    QAction *roiAction;

    QAction *m_stayOnTopAction;

public slots:
    void updateButtonShowGraph(bool v);
    void updateButtonSettings(bool v);
    void updateButtonObjects(bool v);
    void updateButtonROIs(bool v);

signals:
    void closed();

private:
    Z3DTabWidget* getCentralTab() const;

private slots:
    void stayOnTop(bool on);
};

#endif // Z3DMAINWINDOW_H
