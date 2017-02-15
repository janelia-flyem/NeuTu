#include "z3dmainwindow.h"

#include <QAction>
#include "widgets/z3dtabwidget.h"
#include "z3dwindow.h"

Z3DMainWindow::Z3DMainWindow(QWidget *parent) : QMainWindow(parent)
{
    setParent(parent);

    toolBar = addToolBar("3DBodyViewTools");

    //    QPixmap quitpix("quit.png");
    //    QAction *quit = toolBar->addAction(QIcon(quitpix), "Quit 3D Body View");
    //    QAction *quit = toolBar->addAction("Quit 3D Body View");
    //    connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));


    //toolBar->hide();
}

Z3DMainWindow::~Z3DMainWindow()
{
}

void Z3DMainWindow::closeEvent(QCloseEvent *event)
{
  QMainWindow::closeEvent(event);

  emit closed();
}

void Z3DMainWindow::stayOnTop(bool on)
{
  Qt::WindowFlags flags = this->windowFlags();
  if (on) {
    setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    show();
  } else {
    setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
    show();
  }
}

void Z3DMainWindow::updateButtonShowGraph(bool v)
{
    if(showGraphAction)
    {
        showGraphAction->setChecked(v);
    }
}

void Z3DMainWindow::updateButtonSettings(bool v)
{
    if(settingsAction)
    {
        settingsAction->setChecked(v);
    }
}

void Z3DMainWindow::updateButtonObjects(bool v)
{
    if(objectsAction)
    {
        objectsAction->setChecked(v);
    }
}

void Z3DMainWindow::updateButtonROIs(bool v)
{
    if(roiAction)
    {
       roiAction->setChecked(v);
    }
}

Z3DTabWidget* Z3DMainWindow::getCentralTab() const
{
  return qobject_cast<Z3DTabWidget*>(centralWidget());
}

void Z3DMainWindow::setCurrentWidow(Z3DWindow *window)
{
  if (getCentralTab() != NULL) {
    getCentralTab()->setCurrentWidget(window);
  }
}
