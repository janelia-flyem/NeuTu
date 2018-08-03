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
  connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeWindow(int)));

  // default button status
  for(int i=0; i<WINDOW3D_COUNT; i++) {
    resetButtonStatus(i);

    m_windowAdded[i] = false;

    m_rankToIndex[i] = -1;
  }

  m_preRank = -1;

}

QTabBar* Z3DTabWidget::tabBar()
{
  return QTabWidget::tabBar();
}

void Z3DTabWidget::resetCamera()
{
  Z3DWindow *cur3Dwin = getCurrentWindow();

  if(cur3Dwin) {
    cur3Dwin->resetCamera();
  }

}

void Z3DTabWidget::setXZView()
{
  Z3DWindow *cur3Dwin = getCurrentWindow();

  if(cur3Dwin) {
    cur3Dwin->setXZView();
  }

}

void Z3DTabWidget::setYZView()
{
  Z3DWindow *cur3Dwin = getCurrentWindow();

  if(cur3Dwin) {
    cur3Dwin->setYZView();
  }
}

void Z3DTabWidget::resetCameraCenter()
{
  Z3DWindow *cur3Dwin = getCurrentWindow();

  if(cur3Dwin) {
    cur3Dwin->resetCameraCenter();
  }
}

void Z3DTabWidget::showGraph(bool v)
{
  int index = this->currentIndex();

  Z3DWindow *cur3Dwin = getCurrentWindow();

  if(cur3Dwin) {
    cur3Dwin->setButtonStatus(0,v);
    m_buttonStatus[getWindowRank(index)][0] = v;
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
      m_buttonStatus[getWindowRank(index)][1] = v;
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
      m_buttonStatus[getWindowRank(index)][2] = v;
      cur3Dwin->getObjectsDockWidget()->toggleViewAction()->trigger();
    }
    else
    {
      cur3Dwin->getObjectsDockWidget()->setVisible(v);
    }
  }
}

Z3DWindow* Z3DTabWidget::getWindowFromIndex(int index) const
{
  return qobject_cast<Z3DWindow*>(widget(index));
}

Z3DWindow* Z3DTabWidget::getCurrentWindow() const
{
  int index = this->currentIndex();

  Z3DWindow *cur3Dwin = getWindowFromIndex(index);

  return cur3Dwin;
}

ZROIWidget* Z3DTabWidget::roiPanel(bool v)
{
  ZROIWidget* roiWidget = NULL;


  int index = currentIndex();

  Z3DWindow *cur3Dwin = getWindowFromIndex(index);

  if(cur3Dwin) {
    //
    bool checked = cur3Dwin->getButtonStatus(3);

    if(checked != v) {
      cur3Dwin->setButtonStatus(3,v);
      m_buttonStatus[getWindowRank(index)][3] = v;
      roiWidget = cur3Dwin->getROIsDockWidget();
      roiWidget->toggleViewAction()->trigger();
    } else {
      roiWidget = cur3Dwin->getROIsDockWidget();
      roiWidget->setVisible(v);
    }
  }

  if(v) {
    emit buttonROIsClicked();
  }

  return roiWidget;
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
    m_buttonStatus[getWindowRank(index)][3] = false;
    cur3Dwin->getROIsDockWidget()->toggleViewAction()->setChecked(false);

    emit buttonROIsToggled(cur3Dwin->getButtonStatus(3));
  }

}

bool Z3DTabWidget::windowExists(int rank)
{
  if (rank <= 0 || rank >= WINDOW3D_COUNT) {
    return false;
  }

  return m_windowAdded[rank];
}

void Z3DTabWidget::addWindow(int rank, Z3DWindow *window, const QString &title)
{
  if (window != NULL) {

    bool added = false;
    for (int i = 0; i < WINDOW3D_COUNT; ++i) {
      if (m_windowAdded[i]) {
        if (rank < i) { //Smaller rank should take the place
          m_rankToIndex[rank] = insertTab(getTabIndex(i), window, title);
          added = true;
          break;
        }
      }
    }

    if (!added) { //Add to the last when there is no existing window has a bigger rank
      m_rankToIndex[rank] = insertTab(count(), window, title);
    }

#ifdef _DEBUG_
    std::cout << "Add tab: @" << rank << " ->" << getTabIndex(rank) << std::endl;
#endif

    setCurrentIndex(m_rankToIndex[rank]);

    for(int i=rank+1; i<WINDOW3D_COUNT; i++) { //Update the indices behind the new one
      if(m_rankToIndex[i] >= 0) {
        m_rankToIndex[i]++;
      }
    }

    m_windowAdded[rank] = true;

    updateWindow(m_rankToIndex[rank]);
  }
}

int Z3DTabWidget::getTabIndex(int rank)
{
  return (m_rankToIndex[rank]);
}

int Z3DTabWidget::getWindowRank(int index)
{
  int cur = -1;

  if(index>-1) {
    for(int i=0; i<WINDOW3D_COUNT; i++) {
      if(m_rankToIndex[i]==index) {
        cur = i;
        break;
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
  int rank = getWindowRank(index);

  qDebug()<<"###updateWindow"<<index<<m_preRank<<currentIndex()<<rank;

  if(rank >= 0 && windowExists(rank)) {
    qDebug()<<"####updateWindow run";

    Z3DWindow *w = (Z3DWindow *)(widget(index));

    if (w != NULL)
    {
      bool buttonChecked;

      // show graph
      buttonChecked = w->getButtonStatus(0);
      if (w->getGraphFilter() != NULL) {
        w->getGraphFilter()->setVisible(buttonChecked);
      }
      m_buttonStatus[rank][0] = buttonChecked;

      if(m_preRank>-1)
      {
        if(m_windowAdded[m_preRank]==true && m_preRank!=rank)
        {

          qDebug()<<"###if###"<<m_preRank<<rank;

          Z3DWindow *preWin = (Z3DWindow *)(widget(m_rankToIndex[m_preRank]));

          if(preWin)
          {
            // settings
            buttonChecked = w->getButtonStatus(1);
            if(buttonChecked != preWin->getButtonStatus(1))
            {
              w->getSettingsDockWidget()->toggleViewAction()->trigger();
              m_buttonStatus[rank][1] = buttonChecked;
            }

            // objects
            buttonChecked = w->getButtonStatus(2);
            if(buttonChecked != preWin->getButtonStatus(2))
            {
              w->getObjectsDockWidget()->toggleViewAction()->trigger();
              m_buttonStatus[rank][2] = buttonChecked;
            }

            // ROIs
            buttonChecked = w->getButtonStatus(3);
            if(buttonChecked != preWin->getButtonStatus(3))
            {
              w->getROIsDockWidget()->toggleViewAction()->trigger();
              m_buttonStatus[rank][3] = buttonChecked;
            }
          }

        }
        else
        {
          qDebug()<<"###else###"<<m_preRank<<index;

          // settings
          buttonChecked = w->getButtonStatus(1);
          if(buttonChecked != m_buttonStatus[m_preRank][1])
          {
            w->getSettingsDockWidget()->toggleViewAction()->trigger();
            m_buttonStatus[rank][1] = buttonChecked;
          }

          // objects
          buttonChecked = w->getButtonStatus(2);

          qDebug()<<"####objects"<<buttonChecked<<m_buttonStatus[m_preRank][2];

          if(buttonChecked != m_buttonStatus[m_preRank][2])
          {
            qDebug()<<"####objects toggle";

            w->getObjectsDockWidget()->toggleViewAction()->trigger();
            m_buttonStatus[rank][2] = buttonChecked;
          }

          // ROIs
          buttonChecked = w->getButtonStatus(3);
          if(buttonChecked != m_buttonStatus[m_preRank][3])
          {
            w->getROIsDockWidget()->toggleViewAction()->trigger();
            m_buttonStatus[rank][3] = buttonChecked;
          }
        }
      }

      //
      emit buttonShowGraphToggled(w->getButtonStatus(0));
      emit buttonSettingsToggled(w->getButtonStatus(1));
      emit buttonObjectsToggled(w->getButtonStatus(2));
      emit buttonROIsToggled(w->getButtonStatus(3));
    }

    m_preRank = rank;
  }

}

void Z3DTabWidget::closeAllWindows()
{

  for(int i=0; i<WINDOW3D_COUNT; i++)
  {
    if(m_rankToIndex[i]==currentIndex())
    {
      m_preRank = i;
    }

//    m_rankToIndex[i] = -1;
  }

  for(int i = WINDOW3D_COUNT - 1; i >= 0; i--)
  { //Need to close from back to front to avoid crash
    closeWindow(i);
  }
}

void Z3DTabWidget::resetButtonStatus(int rank)
{
  m_buttonStatus[rank][0] = true;
  m_buttonStatus[rank][1] = false;
  m_buttonStatus[rank][2] = false;
  m_buttonStatus[rank][3] = false;
}

Z3DWindow *Z3DTabWidget::removeWindow(int index)
{
  Z3DWindow *w = getWindowFromIndex(index);
  if (w != NULL) {
    int rank = getWindowRank(index);

    resetButtonStatus(rank);

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

    m_windowAdded[rank] = false;

    if(m_preRank == rank) {
      m_preRank = -1;
    }

    m_rankToIndex[rank] = -1;
    //Update indices for bigger ranks
    for (int i = index + 1; i < WINDOW3D_COUNT; ++i) {
      if (m_rankToIndex[i] > 0) {
        --m_rankToIndex[i];
      }
    }

#ifdef _DEBUG_
    std::cout << "Current index: ";
    for (int i = 0; i < WINDOW3D_COUNT; ++i) {
      if (m_rankToIndex[i] >= 0) {
        std::cout << "[" << i << "]" << " " << m_rankToIndex[i] << "; ";
      }
    }
    std::cout << std::endl;
#endif
  }

  return w;
}

void Z3DTabWidget::closeWindow(int index)
{
  if (NeutubeConfig::GetVerboseLevel() >= 2) {
    qDebug()<<"####closeWindow"<<m_preRank<<index<<getWindowRank(index);
  }

  Z3DWindow *w = removeWindow(index);
  if (w != NULL) {
    w->close();
  }
}

Z3DTabWidget::~Z3DTabWidget()
{

}
