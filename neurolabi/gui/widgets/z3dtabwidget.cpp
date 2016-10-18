#include "z3dtabwidget.h"

#include <QDockWidget>

#include "neutubeconfig.h"
#include "z3dwindow.h"
#include "z3dgraphfilter.h"
#include "zroiwidget.h"

Z3DTabWidget::Z3DTabWidget(QWidget *parent) : QTabWidget(parent)
{
  setTabsClosable(true);

  connect(this, SIGNAL(tabIndexChanged(int)), this, SLOT(updateWindow(int)));

  // default button status
  for(int i=0; i<4; i++)
  {
      buttonStatus[i][0] = true;
      buttonStatus[i][1] = false;
      buttonStatus[i][2] = false;
      buttonStatus[i][3] = false;

      windowStatus[i] = false;

      tabLUT[i] = -1;
  }

  preIndex = -1;

}

QTabBar* Z3DTabWidget::tabBar()
{
    return QTabWidget::tabBar();
}

void Z3DTabWidget::resetCamera()
{
    Z3DWindow *cur3Dwin = (Z3DWindow *)(this->currentWidget());

    if(cur3Dwin)
    {
        cur3Dwin->resetCamera();
    }

}

void Z3DTabWidget::setXZView()
{
    Z3DWindow *cur3Dwin = (Z3DWindow *)(this->currentWidget());

    if(cur3Dwin)
    {
        cur3Dwin->setXZView();
    }

}

void Z3DTabWidget::setYZView()
{
    Z3DWindow *cur3Dwin = (Z3DWindow *)(this->currentWidget());

    if(cur3Dwin)
    {
        cur3Dwin->setYZView();
    }

}

void Z3DTabWidget::resetCameraCenter()
{
    Z3DWindow *cur3Dwin = (Z3DWindow *)(this->currentWidget());

    if(cur3Dwin)
    {
        cur3Dwin->resetCameraCenter();
    }
}

void Z3DTabWidget::showGraph(bool v)
{
    int index = this->currentIndex();

    Z3DWindow *cur3Dwin = (Z3DWindow *)(widget(index));

    if(cur3Dwin)
    {
        cur3Dwin->setButtonStatus(0,v);
        buttonStatus[getRealIndex(index)][0] = v;
        cur3Dwin->getGraphFilter()->setVisible(v);
    }
}

void Z3DTabWidget::settingsPanel(bool v)
{
    int index = this->currentIndex();

    Z3DWindow *cur3Dwin = (Z3DWindow *)(widget(index));

    if(cur3Dwin)
    {
        //
        bool checked = cur3Dwin->getButtonStatus(1);

        if(checked != v)
        {
            cur3Dwin->setButtonStatus(1,v);
            buttonStatus[getRealIndex(index)][1] = v;
            cur3Dwin->getSettingsDockWidget()->toggleViewAction()->trigger();
        }
        else
        {
            cur3Dwin->getSettingsDockWidget()->setVisible(v);
        }
    }

}

void Z3DTabWidget::objectsPanel(bool v)
{
    int index = this->currentIndex();

    Z3DWindow *cur3Dwin = (Z3DWindow *)(widget(index));

    if(cur3Dwin)
    {
        //
        bool checked = cur3Dwin->getButtonStatus(2);

        if(checked != v)
        {
            cur3Dwin->setButtonStatus(2,v);
            buttonStatus[getRealIndex(index)][2] = v;
            cur3Dwin->getObjectsDockWidget()->toggleViewAction()->trigger();
        }
        else
        {
            cur3Dwin->getObjectsDockWidget()->setVisible(v);
        }
    }
}

void Z3DTabWidget::roiPanel(bool v)
{
    int index = this->currentIndex();

    Z3DWindow *cur3Dwin = (Z3DWindow *)(widget(index));

    if(cur3Dwin)
    {
        //
        bool checked = cur3Dwin->getButtonStatus(3);

        if(checked != v)
        {
            cur3Dwin->setButtonStatus(3,v);
            buttonStatus[getRealIndex(index)][3] = v;
            cur3Dwin->getROIsDockWidget()->toggleViewAction()->trigger();
        }
        else
        {
            cur3Dwin->getROIsDockWidget()->setVisible(v);
        }
    }

    if(v)
    {
        emit buttonROIsClicked();
    }

}

void Z3DTabWidget::resetSettingsButton()
{

}

void Z3DTabWidget::resetObjectsButton()
{

}

void Z3DTabWidget::resetROIButton()
{
    // widget is closed
    int index = this->currentIndex();

    Z3DWindow *cur3Dwin = (Z3DWindow *)(widget(index));

    if(cur3Dwin)
    {
        cur3Dwin->setButtonStatus(3,false);
        buttonStatus[getRealIndex(index)][3] = false;
        cur3Dwin->getROIsDockWidget()->toggleViewAction()->setChecked(false);

        emit buttonROIsToggled(cur3Dwin->getButtonStatus(3));
    }

}

void Z3DTabWidget::addWindow(int index, Z3DWindow *window, const QString &title)
{
  if (window != NULL) {

      tabLUT[index] = insertTab(index, window, title);

      setCurrentIndex(tabLUT[index]);

      for(int i=index+1; i<4; i++)
      {
          if(tabLUT[i]>-1)
          {
              tabLUT[i]++;
          }
      }

      connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeWindow(int)));

      windowStatus[index] = true;

      updateWindow(tabLUT[index]);
  }
}

int Z3DTabWidget::getTabIndex(int index)
{
    return (tabLUT[index]);
}

int Z3DTabWidget::getRealIndex(int index)
{
    int cur = -1;

    if(index>-1)
    {
        for(int i=0; i<4; i++)
        {
            if(tabLUT[i]==index)
            {
                cur = i;
            }
        }
    }

    return cur;
}

void Z3DTabWidget::updateTabs(int index)
{
    emit tabIndexChanged(index);
}

void Z3DTabWidget::updateWindow(int index)
{
    int cur = getRealIndex(index);

    qDebug()<<"###updateWindow"<<index<<preIndex<<currentIndex()<<cur;

    if(cur>-1 && windowStatus[cur]==true)
    {
        qDebug()<<"####updateWindow run";

        Z3DWindow *w = (Z3DWindow *)(widget(index));

        if (w != NULL)
        {
            bool buttonChecked;

            // show graph
            buttonChecked = w->getButtonStatus(0);
            w->getGraphFilter()->setVisible(buttonChecked);
            buttonStatus[cur][0] = buttonChecked;

            if(preIndex>-1)
            {
                if(windowStatus[preIndex]==true && preIndex!=cur)
                {

                    qDebug()<<"###if###"<<preIndex<<cur;

                    Z3DWindow *preWin = (Z3DWindow *)(widget(tabLUT[preIndex]));

                    if(preWin)
                    {
                        // settings
                        buttonChecked = w->getButtonStatus(1);
                        if(buttonChecked != preWin->getButtonStatus(1))
                        {
                            w->getSettingsDockWidget()->toggleViewAction()->trigger();
                            buttonStatus[cur][1] = buttonChecked;
                        }

                        // objects
                        buttonChecked = w->getButtonStatus(2);
                        if(buttonChecked != preWin->getButtonStatus(2))
                        {
                            w->getObjectsDockWidget()->toggleViewAction()->trigger();
                            buttonStatus[cur][2] = buttonChecked;
                        }

                        // ROIs
                        buttonChecked = w->getButtonStatus(3);
                        if(buttonChecked != preWin->getButtonStatus(3))
                        {
                            w->getROIsDockWidget()->toggleViewAction()->trigger();
                            buttonStatus[cur][3] = buttonChecked;
                        }
                    }

                }
                else
                {
                    qDebug()<<"###else###"<<preIndex<<index;

                    // settings
                    buttonChecked = w->getButtonStatus(1);
                    if(buttonChecked != buttonStatus[preIndex][1])
                    {
                        w->getSettingsDockWidget()->toggleViewAction()->trigger();
                        buttonStatus[cur][1] = buttonChecked;
                    }

                    // objects
                    buttonChecked = w->getButtonStatus(2);

                    qDebug()<<"####objects"<<buttonChecked<<buttonStatus[preIndex][2];

                    if(buttonChecked != buttonStatus[preIndex][2])
                    {
                        qDebug()<<"####objects toggle";

                        w->getObjectsDockWidget()->toggleViewAction()->trigger();
                        buttonStatus[cur][2] = buttonChecked;
                    }

                    // ROIs
                    buttonChecked = w->getButtonStatus(3);
                    if(buttonChecked != buttonStatus[preIndex][3])
                    {
                        w->getROIsDockWidget()->toggleViewAction()->trigger();
                        buttonStatus[cur][3] = buttonChecked;
                    }
                }
            }

            //
            emit buttonShowGraphToggled(w->getButtonStatus(0));
            emit buttonSettingsToggled(w->getButtonStatus(1));
            emit buttonObjectsToggled(w->getButtonStatus(2));
            emit buttonROIsToggled(w->getButtonStatus(3));
        }

        preIndex = cur;
    }

}

void Z3DTabWidget::closeAllWindows()
{

    for(int i=0; i<4; i++)
    {
        if(tabLUT[i]==currentIndex())
        {
            preIndex = i;
        }

        tabLUT[i] = -1;
    }

    for(int i=0; i<4; i++)
    {
        closeWindow(i);
    }
}

void Z3DTabWidget::closeWindow(int index)
{
  if (NeutubeConfig::GetVerboseLevel() >= 2) {
    qDebug()<<"####closeWindow"<<preIndex<<index<<getRealIndex(index);
  }

  Z3DWindow *w = (Z3DWindow *)(widget(index));
  if (w != NULL) {

    buttonStatus[index][0] = true;
    buttonStatus[index][1] = false;
    buttonStatus[index][2] = false;
    buttonStatus[index][3] = false;

    w->getGraphFilter()->setVisible(true);

    bool buttonChecked = w->getButtonStatus(1);
    if(buttonChecked != false)
    {
        w->getSettingsDockWidget()->toggleViewAction()->trigger();
    }

    buttonChecked = w->getButtonStatus(2);
    if(buttonChecked != false)
    {
        w->getObjectsDockWidget()->toggleViewAction()->trigger();
    }

    buttonChecked = w->getButtonStatus(3);
    if(buttonChecked != false)
    {
        w->getROIsDockWidget()->toggleViewAction()->trigger();
    }

    windowStatus[getRealIndex(index)] = false;

    if(preIndex==getRealIndex(index))
        preIndex = -1;

    w->close();
  }


}

Z3DTabWidget::~Z3DTabWidget()
{

}
