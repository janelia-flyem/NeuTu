#include "z3dwindow.h"

#include <iostream>
#include <sstream>
#include <z3dgl.h>
#include <QtGui>
#ifdef _QT5_
#include <QtWidgets>
#endif
#include <limits>
#include <QToolBar>

#include "zstack.hxx"
#include "zstackdoc.h"
#include "zstackframe.h"

#include "neutubeconfig.h"
#include "z3dinteractionhandler.h"
#include "z3dapplication.h"
#include "z3dnetworkevaluator.h"
#include "z3dcanvasrenderer.h"
#include "z3daxis.h"
#include "z3dpunctafilter.h"
#include "z3dswcfilter.h"
#include "z3dcompositor.h"
#include "z3dvolumesource.h"
#include "z3dvolumeraycaster.h"
#include "zpunctum.h"
#include "zlocsegchain.h"
#include "z3dtakescreenshotwidget.h"
#include "z3dcanvas.h"
#include <QThread>
#include "z3dgraphfilter.h"
#include "zswcnetwork.h"
#include "zcloudnetwork.h"
#include "znormcolormap.h"
#include "swctreenode.h"
#include "dialogs/swctypedialog.h"
#include "dialogs/swcsizedialog.h"
#include "dialogs/swcskeletontransformdialog.h"
#include "zswcbranch.h"
#include "zswcdisttrunkanalyzer.h"
#include "zswcbranchingtrunkanalyzer.h"
#include "zfiletype.h"
#include "zswcsizetrunkanalyzer.h"
#include "zswcweighttrunkanalyzer.h"
#include "tubemodel.h"
#include "dialogs/informationdialog.h"
#include "zmoviescene.h"
#include "zmovieactor.h"
#include "zswcmovieactor.h"
#include "zmoviemaker.h"
#include "zmoviescript.h"
#include "zobjsmanagerwidget.h"
#include "zswcobjsmodel.h"
//#include "zpunctaobjsmodel.h"
#include "zdialogfactory.h"
#include "qcolordialog.h"
#include "dialogs/zalphadialog.h"
#include "zstring.h"
#include "zpunctumio.h"
#include "zswcglobalfeatureanalyzer.h"
#include "misc/miscutility.h"
#include "zstackdocmenufactory.h"
#include "swc/zswcsubtreeanalyzer.h"
#include "biocytin/zbiocytinfilenameparser.h"
#include <QApplication>
#include "zvoxelgraphics.h"
#include "zstroke2darray.h"
#include "zswcgenerator.h"
#include "zstroke2d.h"
#include "zsparsestack.h"
#include "dialogs/zmarkswcsomadialog.h"
#include "zinteractivecontext.h"
#include "zwindowfactory.h"
#include "zstackviewparam.h"
#include "z3drendererbase.h"
#include "z3dsurfacefilter.h"
#include "zroiwidget.h"
#include "flyem/zflyemtodolistfilter.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyemtodoitem.h"
#include "zactionlibrary.h"
#include "zmenufactory.h"

class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};

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

Z3DWindow::Z3DWindow(ZSharedPointer<ZStackDoc> doc, Z3DWindow::EInitMode initMode,
                     bool stereoView, QWidget *parent)
  : QMainWindow(parent)
  , m_doc(doc)
  , m_networkEvaluator(NULL)
  , m_canvas(NULL)
  , m_canvasRenderer(NULL)
  , m_axis(NULL)
  , m_volumeRaycaster(NULL)
  , m_punctaFilter(NULL)
  , m_compositor(NULL)
  , m_volumeBoundBox(6)
  , m_swcBoundBox(6)
  , m_punctaBoundBox(6)
  , m_graphBoundBox(6)
  , m_boundBox(6)
  , m_isClean(false)
  , m_blockingTraceMenu(false)
  , m_screenShotWidget(NULL)
  , m_widgetsGroup(NULL)
  , m_settingsDockWidget(NULL)
  , m_objectsDockWidget(NULL)
  , m_advancedSettingDockWidget(NULL)
  , m_isStereoView(stereoView)
  , m_toolBar(NULL)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setFocusPolicy(Qt::StrongFocus);
  createActions();
  createMenus();
  createStatusBar();
  m_viewMenu->addAction("Reset Camera", this, SLOT(resetCamera()));
  init(initMode);
  createDockWindows();
  setAcceptDrops(true);
  m_mergedContextMenu = new QMenu(this);
  m_contextMenu = NULL;

  if (m_doc->getStack() != NULL) {
    setWindowTitle(m_doc->stackSourcePath().c_str());
  }

  m_doc->registerUser(this);
  //createToolBar();

  m_buttonStatus[0] = true;  // showgraph
  m_buttonStatus[1] = false; // settings
  m_buttonStatus[2] = false; // objects
  m_buttonStatus[3] = false; // ROIs

  setWindowType(NeuTube3D::TYPE_GENERAL);
}

Z3DWindow::~Z3DWindow()
{
  cleanup();

  delete m_actionLibrary;
  delete m_menuFactory;
}

void Z3DWindow::createStatusBar()
{
  statusBar()->showMessage("3D window ready.");
}


void Z3DWindow::createToolBar()
{
  m_toolBar = addToolBar("Interaction");
}

void Z3DWindow::gotoPosition(double x, double y, double z, double radius)
{
  double xsize = radius;
  double ysize = radius;
  double zsize = radius;
  std::vector<double> bound(6);
  bound[0] = x - xsize;
  bound[1] = x + xsize;
  bound[2] = y - ysize;
  bound[3] = y + ysize;
  bound[4] = z - zsize;
  bound[5] = z + zsize;
  setupCamera(bound, Z3DCamera::ResetAll);
}

void Z3DWindow::gotoPosition(std::vector<double> bound, double minRadius,
                             double range)
{

  if (bound[1] - bound[0] < minRadius * 2) {
    double expand = range - bound[1] + bound[0];
    bound[1] += expand/2;
    bound[0] -= expand/2;
  }
  if (bound[3] - bound[2] < minRadius * 2) {
    double expand = range - bound[3] + bound[2];
    bound[3] += expand/2;
    bound[2] -= expand/2;
  }

  if (bound[5] - bound[4] < minRadius * 2) {
    double expand = range - bound[5] + bound[4];
    bound[5] += expand/2;
    bound[4] -= expand/2;
  }

  setupCamera(bound, Z3DCamera::PreserveViewVector);
}

void Z3DWindow::zoomToSelectedSwcNodes()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
  if (!nodeSet.empty()) {
    ZCuboid cuboid = SwcTreeNode::boundBox(nodeSet);
    std::vector<double> bound(6);
    bound[0] = cuboid.firstCorner().x();
    bound[2] = cuboid.firstCorner().y();
    bound[4] = cuboid.firstCorner().z();
    bound[1] = cuboid.lastCorner().x();
    bound[3] = cuboid.lastCorner().y();
    bound[5] = cuboid.lastCorner().z();
    gotoPosition(bound);
  }
}

void Z3DWindow::init(EInitMode mode)
{
    // init canvas and opengl context
  #ifdef _QT5_
    m_canvas = new Z3DCanvas("", 512, 512);
  #else
    QGLFormat format = QGLFormat();
    format.setAlpha(true);
    format.setDepth(true);
    format.setDoubleBuffer(true);
    format.setRgba(true);
    format.setSampleBuffers(true);
    if (m_isStereoView)
      format.setStereo(true);
    m_canvas = new Z3DCanvas("", 512, 512, format);
  #endif

  // processors
  m_axis = new Z3DAxis();

  // more processors: init volumes
  if (mode == INIT_EXCLUDE_VOLUME) {
    m_volumeSource = new Z3DVolumeSource(NULL);
  } else {
    if (mode == INIT_NORMAL) {
      m_volumeSource = new Z3DVolumeSource(m_doc.get());
    } else if (mode == INIT_FULL_RES_VOLUME) {
      m_volumeSource = new Z3DVolumeSource(m_doc.get(), MAX_INT32 / 2);
    }
    connect(m_volumeSource, SIGNAL(xScaleChanged()), this, SLOT(volumeScaleChanged()));
    connect(m_volumeSource, SIGNAL(yScaleChanged()), this, SLOT(volumeScaleChanged()));
    connect(m_volumeSource, SIGNAL(zScaleChanged()), this, SLOT(volumeScaleChanged()));
    connect(getDocument(), SIGNAL(volumeModified()), this, SLOT(volumeChanged()));
  }

  // more processors: init geometry filters
  m_compositor = new Z3DCompositor();
  m_punctaFilter = new Z3DPunctaFilter();
  m_layerList.append(LAYER_PUNCTA);
  m_punctaFilter->setData(m_doc->getPunctumList());
  m_swcFilter = new Z3DSwcFilter();
  m_layerList.append(LAYER_SWC);
  m_swcFilter->setData(m_doc->getSwcList());
  m_graphFilter = new Z3DGraphFilter();
  m_layerList.append(LAYER_GRAPH);
#if defined _FLYEM_
  m_todoFilter = new ZFlyEmTodoListFilter;
  m_layerList.append(LAYER_TODO);
  updateTodoList();
#else
  m_todoFilter = NULL;
#endif

  if (m_doc->swcNetwork() != NULL) {
    ZPointNetwork *network = m_doc->swcNetwork()->toPointNetwork();
    //ZNormColorMap colorMap;
    //m_graphFilter->setData(*network, &colorMap);
    m_graphFilter->setData(*network, NULL);

    delete network;
  } else if (ZFileType::fileType(m_doc->additionalSource()) ==
             ZFileType::JSON_FILE) {
    Z3DGraph graph;
    graph.importJsonFile(m_doc->additionalSource());
    m_graphFilter->setData(graph);
  }

//  m_decorationFilter = new Z3DGraphFilter();
//  m_decorationFilter->setStayOnTop(true);


  m_graphFilter->setData(m_doc->get3DGraphDecoration());
  ZOUT(LTRACE(), 5) << "Getting 3d graph";
  TStackObjectList objList = m_doc->getObjectList(ZStackObject::TYPE_3D_GRAPH);
  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    m_graphFilter->addData(*dynamic_cast<Z3DGraph*>(*iter));
  }

  // hard code
  m_surfaceFilter = new Z3DSurfaceFilter;
  m_layerList.append(LAYER_SURFACE);

  //  qDebug()<<"hard coded ...";

  //  //
  //  setROIs(1);
  //  ZCubeArray *cube = new ZCubeArray;
  //  cube->setSource("test cubearray");

  //  //
  //  std::vector<Z3DCube> cubeArray;

  //  //
  //  Z3DCube *cube1 = new Z3DCube;

  //  //
  //  cube1->b_visible.clear();
  //  for(int i=0; i<6; i++)
  //      cube1->b_visible.push_back(true);

  //  float o = -1;
  //  float l = 1;
  //  cube1->length = 4;
  //  cube1->nodes.push_back(glm::vec3(o,o,o)); // 0
  //  cube1->nodes.push_back(glm::vec3(o+l,o,o)); // 1
  //  cube1->nodes.push_back(glm::vec3(o,o+l,o)); // 2
  //  cube1->nodes.push_back(glm::vec3(o+l,o+l,o)); // 3
  //  cube1->nodes.push_back(glm::vec3(o,o,o+l)); // 4
  //  cube1->nodes.push_back(glm::vec3(o+l,o,o+l)); // 5
  //  cube1->nodes.push_back(glm::vec3(o,o+l,o+l)); // 6
  //  cube1->nodes.push_back(glm::vec3(o+l,o+l,o+l)); // 7

  //  //
  //  //cube1->color = glm::vec4(1.0, 0, 0, 0.5);;
  //  cube1->initByNodes = true;
  //  cubeArray.push_back(*cube1);

  //  //
  //  Z3DCube *cube2 = new Z3DCube;

  //  //
  //  cube2->b_visible.clear();
  //  for(int i=0; i<6; i++)
  //      cube2->b_visible.push_back(true);

  //  o = 1;
  //  l = 1;
  //  cube2->length = 4;
  //  cube2->nodes.push_back(glm::vec3(o,o,o)); // 0
  //  cube2->nodes.push_back(glm::vec3(o+l,o,o)); // 1
  //  cube2->nodes.push_back(glm::vec3(o,o+l,o)); // 2
  //  cube2->nodes.push_back(glm::vec3(o+l,o+l,o)); // 3
  //  cube2->nodes.push_back(glm::vec3(o,o,o+l)); // 4
  //  cube2->nodes.push_back(glm::vec3(o+l,o,o+l)); // 5
  //  cube2->nodes.push_back(glm::vec3(o,o+l,o+l)); // 6
  //  cube2->nodes.push_back(glm::vec3(o+l,o+l,o+l)); // 7

  //  //
  //  cube2->initByNodes = true;

  //  //
  //  cubeArray.push_back(*cube2);
  //  cube->setColor(QColor(255,255,0,255));
  //  cube->setCubeArray(cubeArray);

  //  //
  //  m_surfaceFilter->addData(cube);
  //  updateSurfaceBoundBox(); // end hard code


  connect(getDocument(), SIGNAL(punctaModified()), this, SLOT(punctaChanged()));
  connect(getDocument(), SIGNAL(swcModified()), this, SLOT(swcChanged()));
  connect(getDocument(), SIGNAL(swcNetworkModified()),
          this, SLOT(updateNetworkDisplay()));
  connect(getDocument(), SIGNAL(graph3dModified()),
          this, SLOT(update3DGraphDisplay()));
  connect(getDocument(), SIGNAL(cube3dModified()),
          this, SLOT(update3DCubeDisplay()));
  connect(getDocument(), SIGNAL(todoModified()),
          this, SLOT(updateTodoDisplay()));
  connect(getDocument(),
          SIGNAL(objectSelectionChanged(QList<ZStackObject*>,QList<ZStackObject*>)),
          this, SLOT(updateObjectSelection(QList<ZStackObject*>,QList<ZStackObject*>)));
  connect(getDocument(),
          SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),
          this, SLOT(punctaSelectionChanged()));
  connect(getDocument(),
          SIGNAL(swcSelectionChanged(QList<ZSwcTree*>,QList<ZSwcTree*>)),
          this, SLOT(swcSelectionChanged()));
  connect(getDocument(),
          SIGNAL(swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),
          this, SLOT(swcTreeNodeSelectionChanged()));
  connect(getDocument(),
          SIGNAL(punctumVisibleStateChanged()),
          m_punctaFilter, SLOT(updatePunctumVisibleState()));
  connect(getDocument(),
          SIGNAL(graphVisibleStateChanged()),
          this, SLOT(update3DGraphDisplay()));
  connect(getDocument(),
          SIGNAL(surfaceVisibleStateChanged()),
          this, SLOT(update3DCubeDisplay()));
  connect(getDocument(),
          SIGNAL(swcVisibleStateChanged(ZSwcTree*, bool)),
          m_swcFilter, SLOT(updateSwcVisibleState()));
  connect(m_punctaFilter->getRendererBase(), SIGNAL(coordScalesChanged()),
          this, SLOT(punctaCoordScaleChanged()));
  connect(m_swcFilter->getRendererBase(), SIGNAL(coordScalesChanged()),
          this, SLOT(swcCoordScaleChanged()));
  connect(m_punctaFilter->getRendererBase(), SIGNAL(sizeScaleChanged()),
          this, SLOT(punctaSizeScaleChanged()));
  connect(m_swcFilter->getRendererBase(), SIGNAL(sizeScaleChanged()),
          this, SLOT(swcSizeScaleChanged()));
  connect(m_punctaFilter, SIGNAL(punctumSelected(ZPunctum*, bool)),
          this, SLOT(selectedPunctumChangedFrom3D(ZPunctum*, bool)));
  if (m_todoFilter != NULL) {
    connect(m_todoFilter, SIGNAL(objectSelected(ZStackObject*,bool)),
            this, SLOT(selectdObjectChangedFrom3D(ZStackObject*,bool)));
  }
  connect(m_swcFilter, SIGNAL(treeSelected(ZSwcTree*,bool)),
          this, SLOT(selectedSwcChangedFrom3D(ZSwcTree*,bool)));
  connect(m_swcFilter, SIGNAL(treeNodeSelected(Swc_Tree_Node*,bool)),
          this, SLOT(selectedSwcTreeNodeChangedFrom3D(Swc_Tree_Node*,bool)));

  connect(m_swcFilter, SIGNAL(treeNodeSelectConnection(Swc_Tree_Node*)),
          m_doc.get(), SLOT(selectSwcNodeConnection(Swc_Tree_Node*)));
  connect(m_swcFilter, SIGNAL(treeNodeSelectFloodFilling(Swc_Tree_Node*)),
          m_doc.get(), SLOT(selectSwcNodeFloodFilling(Swc_Tree_Node*)));
  connect(m_swcFilter, SIGNAL(addNewSwcTreeNode(double, double, double, double)),
          this, SLOT(addNewSwcTreeNode(double, double, double, double)));
  connect(m_swcFilter, SIGNAL(extendSwcTreeNode(double, double, double, double)),
          this, SLOT(extendSwcTreeNode(double, double, double, double)));
  connect(m_swcFilter, SIGNAL(connectingSwcTreeNode(Swc_Tree_Node*)), this,
          SLOT(connectSwcTreeNode(Swc_Tree_Node*)));

  connect(m_doc.get(), SIGNAL(statusMessageUpdated(QString)),
          this, SLOT(notifyUser(QString)));

  m_swcFilter->setSelectedSwcs(
        m_doc->getObjectGroup().getSelectedSet(ZStackObject::TYPE_SWC));
  //m_swcFilter->setSelectedSwcTreeNodes(m_doc->getSelectedSwcTreeSet());
  m_punctaFilter->setSelectedPuncta(
        m_doc->getObjectGroup().getSelectedSet(ZStackObject::TYPE_PUNCTUM));

  // init windows size based on data
  setWindowSize();

  //
  setCentralWidget(m_canvas);
  m_canvas->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_canvas, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(show3DViewContextMenu(QPoint)));
  m_canvas->getGLFocus();

  m_canvasRenderer = new Z3DCanvasRenderer();
  m_canvasRenderer->setCanvas(m_canvas);

  // more processors: init raycaster
  m_volumeRaycaster = new Z3DVolumeRaycaster();
  m_layerList.append(LAYER_VOLUME);
  connect(m_volumeRaycaster,
          SIGNAL(pointInVolumeLeftClicked(QPoint, glm::ivec3, Qt::KeyboardModifiers)),
          this, SLOT(pointInVolumeLeftClicked(QPoint, glm::ivec3, Qt::KeyboardModifiers)));

  // register processors to receive QGLWidget event
  m_canvas->addEventListenerToBack(m_swcFilter);
  m_canvas->addEventListenerToBack(m_punctaFilter);
  m_canvas->addEventListenerToBack(m_volumeRaycaster);      // for trace
  m_canvas->addEventListenerToBack(m_compositor);  // for interaction
  m_canvas->addEventListenerToBack(m_graphFilter);
  m_canvas->addEventListenerToBack(m_surfaceFilter);
  m_canvas->addEventListenerToBack(m_todoFilter);
//  m_canvas->addEventListenerToBack(m_decorationFilter);

  // build network
  for (int i=0; i<5; i++) {  // max supported channel is 5
    m_volumeSource->getOutputPort(QString("Volume%1").arg(i+1))->connect(m_volumeRaycaster->getInputPort("Volumes"));
  }
  m_volumeSource->getOutputPort("Stack")->connect(m_volumeRaycaster->getInputPort("Stack"));
  m_volumeRaycaster->getOutputPort("Image")->connect(m_compositor->getInputPort("Image"));
  m_volumeRaycaster->getOutputPort("LeftEyeImage")->connect(m_compositor->getInputPort("LeftEyeImage"));
  m_volumeRaycaster->getOutputPort("RightEyeImage")->connect(m_compositor->getInputPort("RightEyeImage"));
  m_punctaFilter->getOutputPort("GeometryFilter")->connect(m_compositor->getInputPort("GeometryFilters"));
  m_swcFilter->getOutputPort("GeometryFilter")->connect(m_compositor->getInputPort("GeometryFilters"));
  m_graphFilter->getOutputPort("GeometryFilter")->connect(
        m_compositor->getInputPort("GeometryFilters"));
  m_surfaceFilter->getOutputPort("GeometryFilter")->connect(
        m_compositor->getInputPort("GeometryFilters"));
  if (m_todoFilter != NULL) {
    m_todoFilter->getOutputPort("GeometryFilter")->connect(
          m_compositor->getInputPort("GeometryFilters"));
  }
//  m_decorationFilter->getOutputPort("GeometryFilter")->connect(m_compositor->getInputPort("GeometryFilters"));

  m_axis->getOutputPort("GeometryFilter")->connect(m_compositor->getInputPort("GeometryFilters"));
  m_compositor->getOutputPort("Image")->connect(m_canvasRenderer->getInputPort("Image"));
  m_compositor->getOutputPort("LeftEyeImage")->connect(m_canvasRenderer->getInputPort("LeftEyeImage"));
  m_compositor->getOutputPort("RightEyeImage")->connect(m_canvasRenderer->getInputPort("RightEyeImage"));

  // connection: canvas <-----> networkevaluator <-----> canvasrender
  m_networkEvaluator = new Z3DNetworkEvaluator();
  m_canvas->setNetworkEvaluator(m_networkEvaluator);

  // pass the canvasrender to the network evaluator
  m_networkEvaluator->setNetworkSink(m_canvasRenderer);

  // initializes all connected processors
  m_networkEvaluator->initializeNetwork();

  //get objects size
  updateVolumeBoundBox();
  updateSwcBoundBox();
  updatePunctaBoundBox();
  updateGraphBoundBox();
  updateSurfaceBoundBox();
//  updateDecorationBoundBox();
  updateOverallBoundBox();

  // adjust camera
  resetCamera();
  m_volumeRaycaster->getCamera()->dependsOn(m_compositor->getCamera());

  if (!NeutubeConfig::getInstance().getZ3DWindowConfig().isAxisOn()) {
    m_axis->setVisible(false);
  }

//  if (!NeutubeConfig::getInstance().getZ3DWindowConfig().isGraphOn()) {
//    m_graphFilter->get
//  }

  connect(getInteractionHandler(), SIGNAL(cameraMoved()),
          this, SLOT(resetCameraClippingRange()));
  connect(getInteractionHandler(), SIGNAL(objectsMoved(double,double,double)),
          this, SLOT(moveSelectedObjects(double,double,double)));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(selectingSwcNodeInRoi(bool)),
          this, SLOT(selectSwcTreeNodeInRoi(bool)));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(croppingSwc()),
          this, SLOT(cropSwcInRoi()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(selectingDownstreamSwcNode()),
          m_doc.get(), SLOT(selectDownstreamNode()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(selectingUpstreamSwcNode()),
          m_doc.get(), SLOT(selectUpstreamNode()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(selectingConnectedSwcNode()),
          m_doc.get(), SLOT(selectConnectedNode()));

  /*
  connect(m_canvas, SIGNAL(strokePainted(ZStroke2d*)),
          this, SLOT(addStrokeFrom3dPaint(ZStroke2d*)));
          */

  connect(m_canvas, SIGNAL(strokePainted(ZStroke2d*)),
          this, SLOT(addPolyplaneFrom3dPaint(ZStroke2d*)));

  m_canvas->set3DInteractionHandler(m_compositor->getInteractionHandler());

#if defined(REMOTE_WORKSTATION)
  getCompositor()->setShowBackground(false);
#endif


  //  // if have image, try black background
  //  if (channelNumber() > 0) {
  //    m_background->setFirstColor(glm::vec3(0.f));
  //    m_background->setSecondColor(glm::vec3(0.f));
  //  }
}

void Z3DWindow::setROIs(size_t n)
{
    m_surfaceFilter->initRenderers(n);
}

void Z3DWindow::setWindowSize()
{
  int width = 512;
  int height = 512;

  float objectWidth = m_boundBox[1] - m_boundBox[0];
  float objectHeight = m_boundBox[3] - m_boundBox[2];

  //get screen size
  QDesktopWidget *desktop = QApplication::desktop();
  QRect screenSize = desktop->availableGeometry();
  float screenWidth = screenSize.width() * 0.6;
  float screenHeight = screenSize.height() * 0.6;

  if (objectWidth > screenWidth || objectHeight > screenHeight) {
    float scale = std::max(objectWidth/screenWidth, objectHeight/screenHeight);
    width = std::max(width, (int)(objectWidth/scale));
    height = std::max(height, (int)(objectHeight/scale));
  } else {
    width = std::max(width, (int)(objectWidth));
    height = std::max(height, (int)(objectHeight));
  }
  resize(width+500, height);   //500 for dock widgets
}

QAction* Z3DWindow::getAction(ZActionFactory::EAction item)
{
  QAction *action = NULL;
  switch (item) {
  case ZActionFactory::ACTION_DELETE_SELECTED:
    if (NeutubeConfig::getInstance().getApplication() != "Biocytin") {
      action = m_actionLibrary->getAction(
            item, this, SLOT(removeSelectedObject()));
    } else {
      action = m_actionLibrary->getAction(
            item, this, SLOT(deleteSelectedSwcNode()));
    }
    break;
  case ZActionFactory::ACTION_ADD_TODO_ITEM:
    action = m_actionLibrary->getAction(item, this, SLOT(addTodoMarker()));
    break;
  case ZActionFactory::ACTION_ADD_TODO_ITEM_CHECKED:
    action = m_actionLibrary->getAction(item, this, SLOT(addDoneMarker()));
    break;
  case ZActionFactory::ACTION_FLYEM_UPDATE_BODY:
    action = m_actionLibrary->getAction(item, this, SLOT(updateBody()));
    break;
  default:
    break;
  }

  return action;
}

void Z3DWindow::createActions()
{
  ZOUT(LTRACE(), 5) << "Create actions";
  /*
  m_undoAction = m_doc->undoStack()->createUndoAction(this, tr("&Undo"));
  m_undoAction->setIcon(QIcon(":/images/undo.png"));
  m_undoAction->setShortcuts(QKeySequence::Undo);

  m_redoAction = m_doc->undoStack()->createRedoAction(this, tr("&Redo"));
  m_redoAction->setIcon(QIcon(":/images/redo.png"));
  m_redoAction->setShortcuts(QKeySequence::Redo);
  */

  m_actionLibrary = new ZActionLibrary(this);
  m_menuFactory = new ZMenuFactory;

  m_undoAction = m_doc->getAction(ZActionFactory::ACTION_UNDO);
  m_redoAction = m_doc->getAction(ZActionFactory::ACTION_REDO);

  m_markSwcSomaAction = new QAction("Mark SWC Soma...", this);
  connect(m_markSwcSomaAction, SIGNAL(triggered()), this, SLOT(markSwcSoma()));

  m_removeSelectedObjectsAction = new QAction("Delete", this);
  if (NeutubeConfig::getInstance().getApplication() != "Biocytin") {
    connect(m_removeSelectedObjectsAction, SIGNAL(triggered()), this,
            SLOT(removeSelectedObject()));
  } else {
    connect(m_removeSelectedObjectsAction, SIGNAL(triggered()), this,
        SLOT(deleteSelectedSwcNode()));
  }

  m_locateSwcNodeIn2DAction = new QAction("Locate node(s) in 2D", this);
  connect(m_locateSwcNodeIn2DAction, SIGNAL(triggered()), this,
          SLOT(locateSwcNodeIn2DView()));

  m_toogleAddSwcNodeModeAction = new QAction("Add neuron node", this);
  m_toogleAddSwcNodeModeAction->setCheckable(true);
  connect(m_toogleAddSwcNodeModeAction, SIGNAL(toggled(bool)), this,
          SLOT(toogleAddSwcNodeMode(bool)));

  m_toggleMoveSelectedObjectsAction =
      new QAction("Move Selected (Shift+Mouse)", this);
  m_toggleMoveSelectedObjectsAction->setShortcut(Qt::Key_V);
  m_toggleMoveSelectedObjectsAction->setIcon(QIcon(":/images/move.png"));
  m_toggleMoveSelectedObjectsAction->setCheckable(true);
  connect(m_toggleMoveSelectedObjectsAction, SIGNAL(toggled(bool)), this,
          SLOT(toogleMoveSelectedObjectsMode(bool)));

  //  m_toogleExtendSelectedSwcNodeAction = new QAction("Extend selected node", this);
  //  m_toogleExtendSelectedSwcNodeAction->setCheckable(true);
  //  connect(m_toogleExtendSelectedSwcNodeAction, SIGNAL(toggled(bool)), this,
  //          SLOT(toogleExtendSelectedSwcNodeMode(bool)));
  //  m_singleSwcNodeActionActivator.registerAction(m_toogleExtendSelectedSwcNodeAction, true);

  m_toggleSmartExtendSelectedSwcNodeAction = new QAction("Extend", this);
  m_toggleSmartExtendSelectedSwcNodeAction->setCheckable(true);
  m_toggleSmartExtendSelectedSwcNodeAction->setShortcut(Qt::Key_Space);
  m_toggleSmartExtendSelectedSwcNodeAction->setStatusTip(
        "Extend the currently selected node with mouse click.");
  m_toggleSmartExtendSelectedSwcNodeAction->setIcon(QIcon(":/images/extend.png"));
  connect(m_toggleSmartExtendSelectedSwcNodeAction, SIGNAL(toggled(bool)), this,
          SLOT(toogleSmartExtendSelectedSwcNodeMode(bool)));
  m_singleSwcNodeActionActivator.registerAction(
        m_toggleSmartExtendSelectedSwcNodeAction, true);

  m_changeSwcNodeTypeAction = new QAction("Change type", this);
  connect(m_changeSwcNodeTypeAction, SIGNAL(triggered()),
          this, SLOT(changeSelectedSwcNodeType()));

  m_setSwcRootAction = new QAction("Set as a root", this);
  connect(m_setSwcRootAction, SIGNAL(triggered()),
          this, SLOT(setRootAsSelectedSwcNode()));
  m_singleSwcNodeActionActivator.registerAction(m_setSwcRootAction, true);

  m_breakSwcConnectionAction = new QAction("Break", this);
  connect(m_breakSwcConnectionAction, SIGNAL(triggered()), this,
          SLOT(breakSelectedSwcNode()));
  m_singleSwcNodeActionActivator.registerAction(m_breakSwcConnectionAction, false);

  m_connectSwcNodeAction = new QAction("Connect", this);
  connect(m_connectSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(connectSelectedSwcNode()));
  m_singleSwcNodeActionActivator.registerAction(m_connectSwcNodeAction, false);

  m_connectToSwcNodeAction = new QAction("Connect to", this);
  m_connectToSwcNodeAction->setShortcut(Qt::Key_C);
  m_connectToSwcNodeAction->setStatusTip(
        "Connect the currently selected node to another");
  connect(m_connectToSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(startConnectingSwcNode()));
  m_connectToSwcNodeAction->setIcon(QIcon(":/images/connect_to.png"));
  m_singleSwcNodeActionActivator.registerAction(m_connectToSwcNodeAction, true);

  m_mergeSwcNodeAction = new QAction("Merge", this);
  connect(m_mergeSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(mergeSelectedSwcNode()));
  m_singleSwcNodeActionActivator.registerAction(m_mergeSwcNodeAction, false);

  m_selectSwcConnectionAction = new QAction("Select Connection", this);
  connect(m_selectSwcConnectionAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectSwcNodeConnection()));

/*
  m_selectSwcNodeUpstreamAction = new QAction("Upstream", this);
  connect(m_selectSwcNodeUpstreamAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectUpstreamNode()));
          */
  /*
  m_selectSwcNodeDownstreamAction = new QAction("Downstream", this);
  connect(m_selectSwcNodeDownstreamAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectDownstreamNode()));
          */

  m_selectSwcNodeDownstreamAction =
      m_doc->getAction(ZActionFactory::ACTION_SELECT_DOWNSTREAM);

  m_selectSwcNodeUpstreamAction =
      m_doc->getAction(ZActionFactory::ACTION_SELECT_UPSTREAM);

  /*
  m_selectSwcNodeBranchAction = new QAction("Branch", this);
  connect(m_selectSwcNodeBranchAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectBranchNode()));
          */
  m_selectSwcNodeBranchAction =
      m_doc->getAction(ZActionFactory::ACTION_SELECT_SWC_BRANCH);


  m_selectSwcNodeTreeAction = new QAction("Tree", this);
  connect(m_selectSwcNodeTreeAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectTreeNode()));

  m_selectAllConnectedSwcNodeAction = new QAction("All Connected Nodes", this);
  connect(m_selectAllConnectedSwcNodeAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectConnectedNode()));

  m_selectAllSwcNodeAction = new QAction("All Nodes", this);
  connect(m_selectAllSwcNodeAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectAllSwcTreeNode()));

  m_translateSwcNodeAction = new QAction("Translate", this);
  connect(m_translateSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(tranlateSelectedSwcNode()));

  m_changeSwcNodeSizeAction = new QAction("Change size", this);
  connect(m_changeSwcNodeSizeAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcNodeSize()));

  m_saveSwcAction = new QAction("Save as", this);
  connect(m_saveSwcAction, SIGNAL(triggered()), this,
          SLOT(saveSelectedSwc()));

  m_changeSwcTypeAction = new QAction("Change type", this);
  connect(m_changeSwcTypeAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcType()));

  m_changeSwcSizeAction = new QAction("Change size", this);
  connect(m_changeSwcSizeAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcSize()));

  m_transformSwcAction = new QAction("Transform skeleton", this);
  connect(m_transformSwcAction, SIGNAL(triggered()), this,
          SLOT(transformSelectedSwc()));

  m_groupSwcAction = new QAction("Group", this);
  connect(m_groupSwcAction, SIGNAL(triggered()), this,
          SLOT(groupSelectedSwc()));

  m_breakForestAction = new QAction("Break forest", this);
  connect(m_breakForestAction, SIGNAL(triggered()), this,
          SLOT(breakSelectedSwc()));

  m_changeSwcColorAction = new QAction("Change color", this);
  connect(m_changeSwcColorAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcColor()));

  m_changeSwcAlphaAction = new QAction("Change transparency", this);
  connect(m_changeSwcAlphaAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcAlpha()));

  m_swcInfoAction = new QAction("Get Info", this);
  connect(m_swcInfoAction, SIGNAL(triggered()), this,
          SLOT(showSelectedSwcInfo()));

  m_swcNodeLengthAction = new QAction("Calculate length", this);
  connect(m_swcNodeLengthAction, SIGNAL(triggered()), this,
          SLOT(showSeletedSwcNodeLength()));

  m_refreshTraceMaskAction = new QAction("Refresh tracing mask", this);
  connect(m_refreshTraceMaskAction, SIGNAL(triggered()), this,
          SLOT(refreshTraceMask()));

    /*
  m_removeSwcTurnAction = new QAction("Remove turn", this);
  connect(m_removeSwcTurnAction, SIGNAL(triggered()),
          this, SLOT(removeSwcTurn()));


  m_resolveCrossoverAction = new QAction("Resolve crossover", this);
  connect(m_resolveCrossoverAction, SIGNAL(triggered()),
          m_doc.get(), SLOT(executeResolveCrossoverCommand()));
          */

  m_removeSwcTurnAction =
      m_doc->getAction(ZActionFactory::ACTION_REMOVE_TURN);

  m_resolveCrossoverAction =
      m_doc->getAction(ZActionFactory::ACTION_RESOLVE_CROSSOVER);

}

void Z3DWindow::createMenus()
{
  m_viewMenu = menuBar()->addMenu(tr("&View"));
  m_editMenu = menuBar()->addMenu(tr("&Edit"));
  m_editMenu->addAction(m_undoAction);
  m_editMenu->addAction(m_redoAction);
  m_editMenu->addSeparator();
  m_editMenu->addAction(m_markSwcSomaAction);

  createContextMenu();
  customizeContextMenu();
}

void Z3DWindow::createContextMenu()
{
  QMenu *contextMenu = new QMenu(this);

  m_saveSelectedPunctaAsAction = new QAction("save selected puncta as ...", this);
  connect(m_saveSelectedPunctaAsAction, SIGNAL(triggered()), this,
          SLOT(saveSelectedPunctaAs()));

  m_changePunctaNameAction = new QAction("Change name", this);
  connect(m_changePunctaNameAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedPunctaName()));

  m_saveAllPunctaAsAction = new QAction("save all puncta as ...", this);
  connect(m_saveAllPunctaAsAction, SIGNAL(triggered()), this,
          SLOT(saveAllPunctaAs()));
  m_locatePunctumIn2DAction = new QAction("Locate punctum in 2D View", this);
   connect(m_locatePunctumIn2DAction, SIGNAL(triggered()), this,
           SLOT(locatePunctumIn2DView()));
  contextMenu->addAction(m_saveSelectedPunctaAsAction);
  contextMenu->addAction(m_saveAllPunctaAsAction);
  contextMenu->addAction(m_changePunctaNameAction);
  contextMenu->addAction(m_locatePunctumIn2DAction);
  contextMenu->addAction("Transform selected puncta",
                         this, SLOT(transformSelectedPuncta()));
  contextMenu->addAction("Change color", this, SLOT(changeSelectedPunctaColor()));
  contextMenu->addAction("Transform all puncta",
                         this, SLOT(transformAllPuncta()));
  contextMenu->addAction("Convert to swc",
                         this, SLOT(convertPunctaToSwc()));
  contextMenu->addAction(m_removeSelectedObjectsAction);
  contextMenu->addAction(m_toggleMoveSelectedObjectsAction);
  m_contextMenuGroup["puncta"] = contextMenu;

  ZOUT(LTRACE(), 5) << "Create swc node menu";

  //Swc node
  contextMenu = new QMenu(this);


  contextMenu->addAction(m_toggleSmartExtendSelectedSwcNodeAction);
  contextMenu->addAction(m_connectToSwcNodeAction);
  contextMenu->addAction(m_selectSwcNodeTreeAction);
  contextMenu->addAction(m_toggleMoveSelectedObjectsAction);

  ZStackDocMenuFactory menuFactory;
  menuFactory.setSingleSwcNodeActionActivator(&m_singleSwcNodeActionActivator);
  menuFactory.makeSwcNodeContextMenu(getDocument(), this, contextMenu);
  contextMenu->addSeparator();
  contextMenu->addAction(m_locateSwcNodeIn2DAction);
  contextMenu->addAction(m_changeSwcNodeTypeAction);
  contextMenu->addAction(m_toogleAddSwcNodeModeAction);

  m_contextMenuGroup["swcnode"] = contextMenu;

  contextMenu = new QMenu(this);
  contextMenu->addAction(m_saveSwcAction);
  contextMenu->addAction(m_changeSwcTypeAction);
  contextMenu->addAction(m_changeSwcSizeAction);
  contextMenu->addAction(m_transformSwcAction);
  contextMenu->addAction(m_groupSwcAction);
  contextMenu->addAction(m_breakForestAction);
  contextMenu->addAction(m_changeSwcColorAction);
  contextMenu->addAction(m_changeSwcAlphaAction);
  contextMenu->addAction(m_swcInfoAction);

#ifdef _DEBUG_2
  contextMenu->addAction("Test", this, SLOT(test()));
#endif
  contextMenu->addAction(m_removeSelectedObjectsAction);
  contextMenu->addAction(m_toggleMoveSelectedObjectsAction);
  m_contextMenuGroup["swc"] = contextMenu;

  contextMenu = new QMenu(this);
  contextMenu->addAction("Accept", this, SLOT(convertSelectedChainToSwc()));
  contextMenu->addAction(m_removeSelectedObjectsAction);
  m_contextMenuGroup["chain"] = contextMenu;

  contextMenu = new QMenu(this);
  contextMenu->addAction("Trace", this, SLOT(traceTube()));
  //contextMenu->addAction("Trace Exp", this, SLOT(traceTube_Exp()));
  m_contextMenuGroup["trace"] = contextMenu;

  contextMenu = new QMenu(this);
  m_openVolumeZoomInViewAction = new QAction("Open Zoom In View", this);
  connect(m_openVolumeZoomInViewAction, SIGNAL(triggered()), this,
          SLOT(openZoomInView()));
  m_exitVolumeZoomInViewAction = new QAction("Exit Zoom In View", this);
  connect(m_exitVolumeZoomInViewAction, SIGNAL(triggered()), this,
          SLOT(exitZoomInView()));
  m_markPunctumAction = new QAction("Mark Punctum", this);
  connect(m_markPunctumAction, SIGNAL(triggered()), this,
          SLOT(markPunctum()));
  m_contextMenuGroup["volume"] = contextMenu;

  contextMenu = new QMenu(this);
  m_changeBackgroundAction = new QAction("Change Background", this);
  connect(m_changeBackgroundAction, SIGNAL(triggered()), this,
          SLOT(changeBackground()));

  m_contextMenuGroup["empty"] = contextMenu;
}

void Z3DWindow::customizeContextMenu()
{
  //Need modification
  m_selectSwcNodeTreeAction->setVisible(false);

  if (GET_APPLICATION_NAME == "Biocytin") {
    m_toogleAddSwcNodeModeAction->setVisible(false);
    m_toggleMoveSelectedObjectsAction->setVisible(false);
    //m_toogleExtendSelectedSwcNodeAction->setVisible(false);
    m_toggleSmartExtendSelectedSwcNodeAction->setVisible(false);
    m_translateSwcNodeAction->setVisible(false);
    //m_selectSwcNodeDownstreamAction->setVisible(false);
    //m_changeSwcNodeTypeAction->setVisible(false);
//    m_mergeSwcNodeAction->setVisible(false);
    m_changeSwcNodeSizeAction->setVisible(false);
    //m_setSwcRootAction->setVisible(false);
    //m_changeSwcTypeAction->setVisible(false);
    m_changeSwcSizeAction->setVisible(false);
    m_transformSwcAction->setVisible(false);
    m_changeSwcColorAction->setVisible(false);
    m_changeSwcAlphaAction->setVisible(false);
    m_refreshTraceMaskAction->setVisible(false);
  } else if (GET_APPLICATION_NAME == "FlyEM") {
    //m_toogleExtendSelectedSwcNodeAction->setVisible(false);
    m_toogleAddSwcNodeModeAction->setVisible(false);
    //m_toogleMoveSelectedObjectsAction->setVisible(false);
    m_toggleSmartExtendSelectedSwcNodeAction->setVisible(false);
    m_refreshTraceMaskAction->setVisible(false);
  }
}

Z3DVolumeRaycasterRenderer* Z3DWindow::getVolumeRaycasterRenderer() {
  return m_volumeRaycaster->getRenderer();
}

void Z3DWindow::hideControlPanel()
{
  if (m_settingsDockWidget != NULL) {
    m_settingsDockWidget->hide();
  }
}

void Z3DWindow::hideObjectView()
{
  if (m_objectsDockWidget != NULL) {
    m_objectsDockWidget->hide();
  }
}

void Z3DWindow::createDockWindows()
{
  m_settingsDockWidget = new QDockWidget(tr("Control and Settings"), this);
  m_settingsDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
  m_settingsDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

  m_widgetsGroup = new ZWidgetsGroup("All", NULL, 1);

  QMenu *cameraMenu = new QMenu(this);
  QPushButton *cameraMenuButton = new QPushButton(tr("Camera"));
  cameraMenuButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  cameraMenuButton->setMenu(cameraMenu);

  QAction *resetCameraAction = new QAction("Reset Camera", this);
  connect(resetCameraAction, SIGNAL(triggered()), this, SLOT(resetCamera()));
  cameraMenu->addAction(resetCameraAction);

  QAction *flipViewAction = new QAction("Back View", this);
  connect(flipViewAction, SIGNAL(triggered()), this, SLOT(flipView()));
  cameraMenu->addAction(flipViewAction);

  QAction *xzViewAction = new QAction("X-Z View", this);
  connect(xzViewAction, SIGNAL(triggered()), this, SLOT(setXZView()));
  cameraMenu->addAction(xzViewAction);

  QAction *yzViewAction = new QAction("Y-Z View", this);
  connect(yzViewAction, SIGNAL(triggered()), this, SLOT(setYZView()));
  cameraMenu->addAction(yzViewAction);

  QAction *saveViewAction = new QAction("Save View", this);
  connect(saveViewAction, SIGNAL(triggered()), this, SLOT(saveView()));
  cameraMenu->addAction(saveViewAction);

  QAction *loadViewAction = new QAction("Load View", this);
  connect(loadViewAction, SIGNAL(triggered()), this, SLOT(loadView()));
  cameraMenu->addAction(loadViewAction);

  // reset camera button and some other utils
//  QPushButton *resetCameraButton = new QPushButton(tr("Reset Camera"));
//  resetCameraButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//  connect(resetCameraButton, SIGNAL(clicked()), this, SLOT(resetCamera()));

#if 0
  QPushButton *flipViewButton = new QPushButton(tr("Back view"));
  flipViewButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  connect(flipViewButton, SIGNAL(clicked()), this, SLOT(flipView()));

#  ifdef _DEBUG_
  QPushButton *recordViewButton = new QPushButton(tr("Record view"));
  recordViewButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  connect(recordViewButton, SIGNAL(clicked()), this, SLOT(recordView()));

  QPushButton *saveViewButton = new QPushButton(tr("Save view"));
  saveViewButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  connect(saveViewButton, SIGNAL(clicked()), this, SLOT(saveView()));

//  QPushButton *diffViewButton = new QPushButton(tr("Calculate view difference"));
//  diffViewButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//  connect(diffViewButton, SIGNAL(clicked()), this, SLOT(diffView()));
#  endif

#endif

  const NeutubeConfig &config = NeutubeConfig::getInstance();
  if (config.getZ3DWindowConfig().isUtilsOn()) {
    ZWidgetsGroup *utils = new ZWidgetsGroup("General", m_widgetsGroup, 1);
    new ZWidgetsGroup(cameraMenuButton, utils, 1);
    new ZWidgetsGroup(getCamera(), utils, 1);
    new ZWidgetsGroup(m_compositor->getParameter("Multisample Anti-Aliasing"), utils, 1);
    new ZWidgetsGroup(m_compositor->getParameter("Transparency"), utils, 1);
//    utils->setVisible(false);
  }

  // capture function
  m_screenShotWidget = new Z3DTakeScreenShotWidget(false, this);
  m_screenShotWidget->setCaptureStereoImage(m_isStereoView);
  connect(m_screenShotWidget, SIGNAL(takeScreenShot(QString, Z3DScreenShotType)),
          this, SLOT(takeScreenShot(QString, Z3DScreenShotType)));
  connect(m_screenShotWidget, SIGNAL(takeScreenShot(QString,int,int,Z3DScreenShotType)),
          this, SLOT(takeScreenShot(QString,int,int,Z3DScreenShotType)));
  connect(m_screenShotWidget, SIGNAL(takeSeriesScreenShot(QDir,QString,glm::vec3,bool,int,Z3DScreenShotType)),
          this, SLOT(takeSeriesScreenShot(QDir,QString,glm::vec3,bool,int,Z3DScreenShotType)));
  connect(m_screenShotWidget, SIGNAL(takeSeriesScreenShot(QDir,QString,glm::vec3,bool,int,int,int,Z3DScreenShotType)),
          this, SLOT(takeSeriesScreenShot(QDir,QString,glm::vec3,bool,int,int,int,Z3DScreenShotType)));
  ZWidgetsGroup *capture = new ZWidgetsGroup("Capture", m_widgetsGroup, 1);
  new ZWidgetsGroup(m_screenShotWidget, capture, 1);

  //volume
  if (config.getZ3DWindowConfig().isVolumeOn()) {
    if (m_volumeSource != NULL) {
      ZWidgetsGroup *wg = m_volumeRaycaster->getWidgetsGroup();
      wg->mergeGroup(m_volumeSource->getWidgetsGroup(), false);
      connect(wg, SIGNAL(requestAdvancedWidget(QString)), this, SLOT(openAdvancedSetting(QString)));
      m_widgetsGroup->addChildGroup(wg);
    }
  }

  ZWidgetsGroup *wg = NULL;

  if (config.getZ3DWindowConfig().isGraphOn()) {
#if defined(_FLYEM_)
    //graph
    wg = m_graphFilter->getWidgetsGroup();
    connect(wg, SIGNAL(requestAdvancedWidget(QString)),
            this, SLOT(openAdvancedSetting(QString)));
    m_widgetsGroup->addChildGroup(wg);
#endif
  }

  if (config.getZ3DWindowConfig().isGraphOn()) {
#if defined(_FLYEM_)
    //graph
    wg = m_surfaceFilter->getWidgetsGroup();
    connect(wg, SIGNAL(requestAdvancedWidget(QString)), this, SLOT(openAdvancedSetting(QString)));
    m_widgetsGroup->addChildGroup(wg);
#endif
  }

#if defined(_FLYEM_)
  wg = m_todoFilter->getWidgetsGroup();
  connect(wg, SIGNAL(requestAdvancedWidget(QString)),
          this, SLOT(openAdvancedSetting(QString)));
  m_widgetsGroup->addChildGroup(wg);
#endif

  if (config.getZ3DWindowConfig().isSwcsOn()) {
    //swc
    wg = m_swcFilter->getWidgetsGroup();
    connect(wg, SIGNAL(requestAdvancedWidget(QString)),
            this, SLOT(openAdvancedSetting(QString)));
    m_widgetsGroup->addChildGroup(wg);
  }

#if !defined(_NEUTUBE_LIGHT_)
  //puncta
  if (config.getZ3DWindowConfig().isPunctaOn()) {
    wg = m_punctaFilter->getWidgetsGroup();
    connect(wg, SIGNAL(requestAdvancedWidget(QString)),
            this, SLOT(openAdvancedSetting(QString)));
    m_widgetsGroup->addChildGroup(wg);
  }
#endif

  //background
  wg = m_compositor->getBackgroundWidgetsGroup();
  connect(wg, SIGNAL(requestAdvancedWidget(QString)),
          this, SLOT(openAdvancedSetting(QString)));
  m_widgetsGroup->addChildGroup(wg);

  //axis
  wg = m_axis->getWidgetsGroup();
  connect(wg, SIGNAL(requestAdvancedWidget(QString)),
          this, SLOT(openAdvancedSetting(QString)));
  m_widgetsGroup->addChildGroup(wg);

  //QWidget *widget = m_widgetsGroup->createWidget(this, true);

  //m_settingsDockWidget->setWidget(widget);

  QTabWidget *tabs = createBasicSettingTabWidget();
  m_settingsDockWidget->setWidget(tabs);
  m_viewMenu->addSeparator();
  m_viewMenu->addAction(m_settingsDockWidget->toggleViewAction());
  connect(m_widgetsGroup, SIGNAL(widgetsGroupChanged()), this, SLOT(updateSettingsDockWidget()));

  m_objectsDockWidget = new QDockWidget(tr("Objects"), this);
  m_objectsDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
  m_objectsDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  ZObjsManagerWidget* omw = new ZObjsManagerWidget(getDocument(), m_objectsDockWidget);
  connect(omw, SIGNAL(swcDoubleClicked(ZSwcTree*)), this, SLOT(swcDoubleClicked(ZSwcTree*)));
  connect(omw, SIGNAL(swcNodeDoubleClicked(Swc_Tree_Node*)), this, SLOT(swcNodeDoubleClicked(Swc_Tree_Node*)));
  connect(omw, SIGNAL(punctaDoubleClicked(ZPunctum*)), this, SLOT(punctaDoubleClicked(ZPunctum*)));
  m_objectsDockWidget->setWidget(omw);
  m_viewMenu->addAction(m_objectsDockWidget->toggleViewAction());

  m_roiDockWidget = new ZROIWidget(tr("ROIs"), this);
  m_roiDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
  m_roiDockWidget->setVisible(false);
  m_viewMenu->addAction(m_roiDockWidget->toggleViewAction());

  addDockWidget(Qt::RightDockWidgetArea, m_roiDockWidget);
  addDockWidget(Qt::RightDockWidgetArea, m_objectsDockWidget);
  addDockWidget(Qt::RightDockWidgetArea, m_settingsDockWidget);

  customizeDockWindows(tabs);
}

int Z3DWindow::channelNumber()
{
  if (m_volumeSource == NULL) {
    return 0;
  }

  if (m_volumeSource->isEmpty()) {
    return 0;
  }

  if (m_doc.get() == NULL) {
    return 0;
  }

  if (m_doc->hasStack()) {
    return m_doc->getStack()->channelNumber();
  } else {
    return 0;
  }
}

void Z3DWindow::customizeDockWindows(QTabWidget *m_settingTab)
{
  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    m_settingTab->setCurrentIndex(1);
  }
}

bool Z3DWindow::hasVolume()
{
  return channelNumber() > 0;
}

void Z3DWindow::resetCamera()
{
  setupCamera(m_boundBox, Z3DCamera::ResetAll);
}

void Z3DWindow::resetCameraCenter()
{
  setupCamera(m_boundBox, Z3DCamera::PreserveViewVector);
}

void Z3DWindow::setupCamera(const std::vector<double> &bound,
                            Z3DCamera::ResetCameraOptions options)
{
  getCamera()->resetCamera(bound, options);
}

void Z3DWindow::flipView()
{
  getCamera()->flipViewDirection();
}

void Z3DWindow::setXZView()
{
  resetCamera();
  getCamera()->rotate90X();
}

void Z3DWindow::setYZView()
{
  resetCamera();
  getCamera()->rotate90XZ();
}

void Z3DWindow::recordView()
{
  m_cameraRecord = getCamera()->get();
}

void Z3DWindow::diffView()
{
  std::cout << "Eye: " << getCamera()->getEye() - m_cameraRecord.getEye()
            << std::endl;
  std::cout << "Center: "
            << getCamera()->getCenter() - m_cameraRecord.getCenter() << std::endl;
  std::cout << "Up vector: "
            << getCamera()->getUpVector() - m_cameraRecord.getUpVector() << std::endl;
}

void Z3DWindow::saveView()
{
  QString filename = QFileDialog::getSaveFileName(
        this, tr("Save View Parameters"), m_lastOpenedFilePath,
         tr("Json files (*.json) "));


  if (!filename.isEmpty()) {
    m_lastOpenedFilePath = filename;
    ZJsonObject cameraJson =getCamera()->get().toJsonObject();
    cameraJson.dump(filename.toStdString());
  }
}

void Z3DWindow::loadView()
{
  QString fileName = QFileDialog::getOpenFileName(
        this, tr("Load View Parameters"), m_lastOpenedFilePath,
        tr("Json files (*.json) "));

  if (!fileName.isEmpty()) {
    ZJsonObject cameraJson;
    cameraJson.load(fileName.toStdString());
    getCamera()->get().set(cameraJson);
    getCamera()->updatePara();
  }
}

void Z3DWindow::configureLayer(ERendererLayer layer, const ZJsonObject &obj)
{
  Z3DGeometryFilter *filter = getFilter(layer);
  if (filter != NULL) {
    if (obj.hasKey("visible")) {
      setVisible(layer, ZJsonParser::booleanValue(obj["visible"]));
    }
    if (obj.hasKey("front")) {
      filter->setStayOnTop(ZJsonParser::booleanValue(obj["front"]));
    }
  }
}

std::string Z3DWindow::GetLayerString(ERendererLayer layer)
{
  switch (layer) {
  case LAYER_GRAPH:
    return "Graph";
  case LAYER_SWC:
    return "SWC";
  case LAYER_PUNCTA:
    return "Puncta";
  case LAYER_SURFACE:
    return "Surface";
  case LAYER_TODO:
    return "Todo";
  case LAYER_VOLUME:
    return "Volume";
  }

  return "";
}

void Z3DWindow::configure(const ZJsonObject &obj)
{
  for (QList<ERendererLayer>::const_iterator iter = m_layerList.begin();
       iter != m_layerList.end(); ++iter) {
    ERendererLayer layer = *iter;
    std::string layerKey = GetLayerString(layer);
    if (obj.hasKey(layerKey.c_str())) {
      ZJsonObject layerJson(obj.value(layerKey.c_str()));
      configureLayer(layer, layerJson);
    }
  }
}

ZJsonObject Z3DWindow::getConfigJson(ERendererLayer layer) const
{
  ZJsonObject configJson;
  Z3DGeometryFilter *filter = getFilter(layer);
  if (filter != NULL) {
    configJson.setEntry("visible", isVisible(layer));
    configJson.setEntry("front", filter->isStayOnTop());
  }

  return configJson;
}

void Z3DWindow::readSettings()
{
  QString windowKey = NeuTube3D::GetWindowKeyString(getWindowType()).c_str();
  QString settingString = NeutubeConfig::GetSettings().value(windowKey).
      toString();

  ZJsonObject jsonObj;

  qDebug() << settingString;

  jsonObj.decodeString(settingString.toStdString().c_str());

  configure(jsonObj);
}

void Z3DWindow::writeSettings()
{
  //ignore general window type for now
  if (getWindowType() != NeuTube3D::TYPE_GENERAL) {
    ZJsonObject configJson;
    for (QList<ERendererLayer>::const_iterator iter = m_layerList.begin();
         iter != m_layerList.end(); ++iter) {
      ERendererLayer layer = *iter;
      ZJsonObject layerJson = getConfigJson(layer);
      configJson.setEntry(GetLayerString(layer).c_str(), layerJson);
    }

    std::string settingString = configJson.dumpString(0);

#ifdef _DEBUG_
    std::cout << settingString << std::endl;
#endif
    NeutubeConfig::GetSettings().setValue(
          NeuTube3D::GetWindowKeyString(getWindowType()).c_str(),
          settingString.c_str());
  }
}

bool Z3DWindow::hasRectRoi() const
{
  return getCanvas()->getInteractionEngine()->hasRectDecoration();
}

ZRect2d Z3DWindow::getRectRoi() const
{
  return getCanvas()->getInteractionEngine()->getRectDecoration();
}

void Z3DWindow::resetCameraClippingRange()
{
  getCamera()->resetCameraNearFarPlane(m_boundBox);
}

QPointF Z3DWindow::getScreenProjection(
    double x, double y, double z, ERendererLayer layer)
{
  Z3DRendererBase *base = getRendererBase(layer);

  QPointF pt(0, 0);

  if (base != NULL) {
    glm::vec3 coord = getRendererBase(layer)->getViewCoord(
          x, y, z, getCanvas()->width(), getCanvas()->height());
    pt.setX(coord[0]);
    pt.setY(coord[1]);
  }

  return pt;
}

void Z3DWindow::updateVolumeBoundBox()
{
  m_volumeBoundBox[0] = m_volumeBoundBox[2] = m_volumeBoundBox[4] =
      std::numeric_limits<double>::max();
  m_volumeBoundBox[1] = m_volumeBoundBox[3] = m_volumeBoundBox[5] =
      -std::numeric_limits<double>::max();
  if (hasVolume()) {
    m_volumeBoundBox = m_volumeSource->getVolume(0)->getWorldBoundBox();
  }
}

void Z3DWindow::updateSwcBoundBox()
{
  m_swcBoundBox[0] = m_swcBoundBox[2] = m_swcBoundBox[4] = std::numeric_limits<double>::max();
  m_swcBoundBox[1] = m_swcBoundBox[3] = m_swcBoundBox[5] = -std::numeric_limits<double>::max();

  QList<ZSwcTree*> swcList = m_doc->getSwcList();

  for (int i=0; i< swcList.size(); i++) {
    std::vector<double> boundBox = m_swcFilter->getTreeBound(swcList.at(i));
    m_swcBoundBox[0] = std::min(boundBox[0], m_swcBoundBox[0]);
    m_swcBoundBox[1] = std::max(boundBox[1], m_swcBoundBox[1]);
    m_swcBoundBox[2] = std::min(boundBox[2], m_swcBoundBox[2]);
    m_swcBoundBox[3] = std::max(boundBox[3], m_swcBoundBox[3]);
    m_swcBoundBox[4] = std::min(boundBox[4], m_swcBoundBox[4]);
    m_swcBoundBox[5] = std::max(boundBox[5], m_swcBoundBox[5]);
  }
}

void Z3DWindow::updateGraphBoundBox()
{
  m_graphBoundBox = m_graphFilter->boundBox();
}


void Z3DWindow::updateSurfaceBoundBox()
{
    m_surfaceBoundBox = m_surfaceFilter->boundBox();
}

void Z3DWindow::updateTodoBoundBox()
{
  if (m_todoFilter != NULL) {
    m_todoBoundBox = m_todoFilter->boundBox();
  }
}

/*
void Z3DWindow::updateDecorationBoundBox()
{
  m_decorationBoundBox = m_decorationFilter->boundBox();
}
*/

void Z3DWindow::updatePunctaBoundBox()
{
  m_punctaBoundBox[0] = m_punctaBoundBox[2] = m_punctaBoundBox[4] = std::numeric_limits<double>::max();
  m_punctaBoundBox[1] = m_punctaBoundBox[3] = m_punctaBoundBox[5] = -std::numeric_limits<double>::max();

  QList<ZPunctum*> punctaList = m_doc->getPunctumList();
  for (int i=0; i<punctaList.size(); i++) {
    std::vector<double> boundBox = m_punctaFilter->getPunctumBound(punctaList.at(i));
    m_punctaBoundBox[0] = std::min(boundBox[0], m_punctaBoundBox[0]);
    m_punctaBoundBox[1] = std::max(boundBox[1], m_punctaBoundBox[1]);
    m_punctaBoundBox[2] = std::min(boundBox[2], m_punctaBoundBox[2]);
    m_punctaBoundBox[3] = std::max(boundBox[3], m_punctaBoundBox[3]);
    m_punctaBoundBox[4] = std::min(boundBox[4], m_punctaBoundBox[4]);
    m_punctaBoundBox[5] = std::max(boundBox[5], m_punctaBoundBox[5]);
  }
}

Z3DCameraParameter *Z3DWindow::getCamera()
{
  return m_compositor->getCamera();
}

Z3DTrackballInteractionHandler *Z3DWindow::getInteractionHandler()
{
  return m_compositor->getInteractionHandler();
}

void Z3DWindow::cleanup()
{
  if (!m_isClean) {
    delete m_widgetsGroup;
    m_widgetsGroup = NULL;
    m_canvas->setNetworkEvaluator(NULL);
    m_networkEvaluator->deinitializeNetwork();
    delete m_networkEvaluator;
    m_networkEvaluator = NULL;

    delete m_volumeSource;
    m_volumeSource = NULL;
    delete m_volumeRaycaster;
    m_volumeRaycaster = NULL;
    delete m_punctaFilter;
    m_punctaFilter = NULL;
    delete m_swcFilter;
    m_swcFilter = NULL;
    delete m_graphFilter;
    m_graphFilter = NULL;
    delete m_surfaceFilter;
    m_surfaceFilter = NULL;
    delete m_todoFilter;
    m_todoFilter = NULL;
//    delete m_decorationFilter;
//    m_decorationFilter = NULL;
    delete m_compositor;
    m_compositor = NULL;
    delete m_axis;
    m_axis = NULL;
    delete m_canvasRenderer;
    m_canvasRenderer = NULL;

    delete m_canvas;
    m_isClean = true;

    m_buttonStatus[0] = true;  // showgraph
    m_buttonStatus[1] = false; // settings
    m_buttonStatus[2] = false; // objects
    m_buttonStatus[3] = false; // rois
  }
}

void Z3DWindow::volumeChanged()
{
  if (m_volumeSource == NULL) {
    m_volumeSource = new Z3DVolumeSource(getDocument());
  }

  m_volumeSource->reloadVolume();
  updateVolumeBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::swcChanged()
{
  m_swcFilter->setData(m_doc->getSwcList());
  updateSwcBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::updateNetworkDisplay()
{
  if (m_doc->swcNetwork() != NULL) {
    ZPointNetwork *network = m_doc->swcNetwork()->toPointNetwork();
    m_graphFilter->setData(*network, NULL);

    delete network;
  }
}

void Z3DWindow::update3DGraphDisplay()
{
  ZOUT(LTRACE(), 5) << "Update 3d graph";
  m_graphFilter->setData(m_doc->get3DGraphDecoration());
  TStackObjectList objList = m_doc->getObjectList(ZStackObject::TYPE_3D_GRAPH);
  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    Z3DGraph *graph = dynamic_cast<Z3DGraph*>(*iter);
    if (graph->isVisible()) {
      m_graphFilter->addData(*graph);
    }
  }
  updateGraphBoundBox();
//  updateDecorationBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::update3DCubeDisplay()
{
  m_surfaceFilter->clearSources();
  ZOUT(LTRACE(), 5) << "Update 3d cube";
  TStackObjectList objList = m_doc->getObjectList(ZStackObject::TYPE_3D_CUBE);
  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZCubeArray *cubeArray = dynamic_cast<ZCubeArray*>(*iter);
    if (cubeArray->isVisible()) {
      m_surfaceFilter->addData(cubeArray);
    }
  }
  m_surfaceFilter->updateSurfaceVisibleState();

  updateSurfaceBoundBox();
//  updateDecorationBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::updateTodoList()
{
#if defined(_FLYEM_)
  ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
  if (doc != NULL) {
    ZOUT(LTRACE(), 5) << "Update todo list";
    QList<ZFlyEmToDoItem*> objList = doc->getObjectList<ZFlyEmToDoItem>();
    m_todoFilter->setData(objList);
/*
    const TStackObjectList& objList =
        doc->getObjectList(ZStackObject::TYPE_FLYEM_TODO_ITEM);
//        doc->getDataDocument()->getObjectList(ZStackObject::TYPE_FLYEM_TODO_LIST);
    if (!objList.isEmpty()) {
      m_todoFilter->setData(objList.front());
    }
    */
  }
#endif
}

void Z3DWindow::updateTodoDisplay()
{
  if (m_todoFilter != NULL) {
    updateTodoList();
    updateTodoBoundBox();
    updateOverallBoundBox();
    resetCameraClippingRange();
  }
}

void Z3DWindow::updateDisplay()
{
  volumeChanged();
  swcChanged();
  punctaChanged();
  updateNetworkDisplay();
  update3DGraphDisplay();
  updateTodoDisplay();
//  updateDecorationDisplay();
}

void Z3DWindow::punctaChanged()
{
  m_punctaFilter->setData(m_doc->getPunctumList());
  updatePunctaBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::volumeScaleChanged()
{
  if (!m_doc->hasStackData())
    return;
  updateVolumeBoundBox();
  updateOverallBoundBox();
  //setupCamera(m_boundBox, Z3DCamera::PreserveCenterDistance | Z3DCamera::PreserveViewVector);
  resetCameraClippingRange();
}

void Z3DWindow::swcCoordScaleChanged()
{
  if (!m_doc->hasSwc())
    return;
  updateSwcBoundBox();
  updateOverallBoundBox();
  //setupCamera(m_boundBox, Z3DCamera::PreserveCenterDistance | Z3DCamera::PreserveViewVector);
  resetCameraClippingRange();
}

void Z3DWindow::punctaCoordScaleChanged()
{
  ZOUT(LTRACE(), 5) << "Punctum scale changed";
  if (m_doc->getObjectList(ZStackObject::TYPE_PUNCTUM).empty())
    return;
  updatePunctaBoundBox();
  updateOverallBoundBox();
  //setupCamera(m_boundBox, Z3DCamera::PreserveCenterDistance | Z3DCamera::PreserveViewVector);
  resetCameraClippingRange();
}

void Z3DWindow::swcSizeScaleChanged()
{
  updateSwcBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::punctaSizeScaleChanged()
{
  updatePunctaBoundBox();
  updateOverallBoundBox();
  resetCameraClippingRange();
}

void Z3DWindow::selectdObjectChangedFrom3D(ZStackObject *p, bool append)
{
  if (p == NULL) {
    if (!append) {
      if (m_todoFilter != NULL) { //temporary hack
        m_doc->deselectAllObject(ZStackObject::TYPE_FLYEM_TODO_ITEM);
        m_todoFilter->invalidate();
      }
    }
    return;
  }

  switch (p->getType()) {
  case ZStackObject::TYPE_FLYEM_TODO_ITEM:
  {
    ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
    if (doc != NULL) {
      doc->selectObject(p, append);
    }
  }
    break;
  default:
    break;
  }
}

void Z3DWindow::selectedPunctumChangedFrom3D(ZPunctum *p, bool append)
{
  if (p == NULL) {
    if (!append)
      m_doc->deselectAllPuncta();
    return;
  }

  if (append) {
    m_doc->setPunctumSelected(p, true);
  } else {
    m_doc->deselectAllObject();
    m_doc->setPunctumSelected(p, true);
  }

  statusBar()->showMessage(p->toString().c_str());
}

void Z3DWindow::selectedSwcChangedFrom3D(ZSwcTree *p, bool append)
{
  if (!append) {
    if (m_doc->hasSelectedSwc()) {
      m_blockingTraceMenu = true;
    }
  }

  if (p == NULL) {
    if (!append)
      m_doc->deselectAllSwcs();
    return;
  }

  if (append) {
    m_doc->setSwcSelected(p, true);
  } else {
    m_doc->deselectAllObject();
    m_doc->setSwcSelected(p, true);
  }

  statusBar()->showMessage(p->getSource().c_str());
}

void Z3DWindow::selectedSwcTreeNodeChangedFrom3D(Swc_Tree_Node *p, bool append)
{
  if (!append) {
    if (m_doc->hasSelectedSwcNode()) {
      m_blockingTraceMenu = true;
    }
  }

  if (p == NULL) {
    if (!append) {
      if (m_doc->hasSelectedSwcNode()) {
        m_doc->deselectAllSwcTreeNodes();
      }
    }
    return;
  }

  if (!append) {
    m_doc->deselectAllObject();
  }

  if (m_doc->isSwcNodeSelected(p) == 0) {
    m_doc->setSwcTreeNodeSelected(p, true);
  } else {
    m_doc->setSwcTreeNodeSelected(p, false);
  }

  statusBar()->showMessage(SwcTreeNode::toString(p).c_str());
}

void Z3DWindow::addNewSwcTreeNode(double x, double y, double z, double r)
{
  m_doc->executeAddSwcNodeCommand(
        ZPoint(x, y, z), r, ZStackObjectRole::ROLE_NONE);
      /*
  QUndoCommand *insertNewSwcTreeNodeCommand =
      new ZStackDocAddSwcNodeCommand(m_doc.get(), p);
  m_doc->undoStack()->push(insertNewSwcTreeNodeCommand);
      */
}

void Z3DWindow::extendSwcTreeNode(double x, double y, double z, double r)
{
  m_doc->executeSwcNodeExtendCommand(ZPoint(x, y, z), r);
}

void Z3DWindow::removeSwcTurn()
{
  m_doc->executeRemoveTurnCommand();
}

void Z3DWindow::startConnectingSwcNode()
{
  notifyUser("Click on the target node to connect.");
  getSwcFilter()->setInteractionMode(Z3DSwcFilter::ConnectSwcNode);
  m_canvas->getInteractionContext().setSwcEditMode(
        ZInteractiveContext::SWC_EDIT_CONNECT);
  m_canvas->updateCursor();
  //m_canvas->setCursor(Qt::SizeBDiagCursor);
}

void Z3DWindow::connectSwcTreeNode(Swc_Tree_Node *tn)
{
  if (getDocument()->hasSelectedSwcNode()) {
    Swc_Tree_Node *target = SwcTreeNode::findClosestNode(
          getDocument()->getSelectedSwcNodeSet(), tn);
    m_doc->executeConnectSwcNodeCommand(target, tn);
    getSwcFilter()->setInteractionMode(Z3DSwcFilter::Select);
    m_canvas->getInteractionContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_OFF);
    m_canvas->updateCursor();
    //m_canvas->setCursor(Qt::ArrowCursor);
  }
}

void Z3DWindow::punctaSelectionChanged()
{
  m_punctaFilter->invalidate();
}

void Z3DWindow::swcSelectionChanged()
{
  m_swcFilter->invalidate();
}

void Z3DWindow::swcTreeNodeSelectionChanged()
{
  m_swcFilter->invalidate();
}

void Z3DWindow::updateObjectSelection(
    QList<ZStackObject *> selected, QList<ZStackObject *> deselected)
{
  QSet<ZStackObject::EType> typeSet;
  for (QList<ZStackObject*>::const_iterator iter = selected.begin();
       iter != selected.end(); ++iter) {
    ZStackObject *obj = *iter;
    typeSet.insert(obj->getType());
  }

  for (QList<ZStackObject*>::const_iterator iter = deselected.begin();
       iter != deselected.end(); ++iter) {
    ZStackObject *obj = *iter;
    typeSet.insert(obj->getType());
  }

  for (QSet<ZStackObject::EType>::const_iterator iter = typeSet.begin();
       iter != typeSet.end(); ++iter) {
    ZStackObject::EType  type = *iter;
    switch (type) {
    case ZStackObject::TYPE_SWC:
      swcSelectionChanged();
      break;
    case ZStackObject::TYPE_PUNCTA:
      punctaSelectionChanged();
      break;
    case ZStackObject::TYPE_FLYEM_TODO_ITEM:
      if (m_todoFilter != NULL) {
        m_todoFilter->invalidate();
      }
      break;
    default:
      break;
    }
  }
}

void Z3DWindow::swcDoubleClicked(ZSwcTree *tree)
{
  std::vector<double> boundBox = m_swcFilter->getTreeBound(tree);
  gotoPosition(boundBox, 0);
}

void Z3DWindow::swcNodeDoubleClicked(Swc_Tree_Node *node)
{
  std::vector<double> boundBox(6, 0);
  m_swcFilter->getTreeNodeBound(node, boundBox);
  gotoPosition(boundBox, 0);
}

void Z3DWindow::punctaDoubleClicked(ZPunctum *p)
{
  std::vector<double> boundBox = m_punctaFilter->getPunctumBound(p);
  if (hasVolume() > 0) {
    if (m_volumeSource->isSubvolume())
      m_volumeSource->exitZoomInView();
  }
  gotoPosition(boundBox);
}

void Z3DWindow::pointInVolumeLeftClicked(
    QPoint pt, glm::ivec3 pos, Qt::KeyboardModifiers modifiers)
{
  glm::vec3 fpos = glm::vec3(pos);
  m_lastClickedPosInVolume = glm::ivec3(fpos);
  LDEBUG() << "Point in volume left clicked" << fpos;
  // only do tracing when we are not editing swc nodes or the preconditions for editing swc node are not met
  if (hasVolume() && channelNumber() == 1 &&
      m_toggleSmartExtendSelectedSwcNodeAction->isChecked() &&
      m_doc->getSelectedSwcNodeNumber() == 1) {
    if (modifiers == Qt::ControlModifier) {
      m_doc->executeSwcNodeExtendCommand(ZPoint(fpos[0], fpos[1], fpos[2]));
    } else {
      m_doc->executeSwcNodeSmartExtendCommand(ZPoint(fpos[0], fpos[1], fpos[2]));
    }
    // todo: check modifier and use normal extend if possible
    return;
  }
  //  if (m_toogleExtendSelectedSwcNodeAction->isChecked() && m_doc->selectedSwcTreeNodes()->size() == 1) {
  //    return;
  //  }
  if (m_blockingTraceMenu) {
    m_blockingTraceMenu = false;
  } else {
    if (hasVolume() && channelNumber() == 1 &&
        !m_toogleAddSwcNodeModeAction->isChecked() &&
        !m_toggleMoveSelectedObjectsAction->isChecked() &&
        !m_toggleSmartExtendSelectedSwcNodeAction->isChecked() &&
        NeutubeConfig::getInstance().getMainWindowConfig().isTracingOn()) {
      m_contextMenuGroup["trace"]->popup(m_canvas->mapToGlobal(pt));
    }
  }
}

void Z3DWindow::show3DViewContextMenu(QPoint pt)
{
  notifyUser(" ");
  if (m_toogleAddSwcNodeModeAction->isChecked()) {
    m_toogleAddSwcNodeModeAction->setChecked(false);
    return;
  } else if (m_toggleMoveSelectedObjectsAction->isChecked()) {
    m_toggleMoveSelectedObjectsAction->setChecked(false);
    return;
  } else if (m_toggleSmartExtendSelectedSwcNodeAction->isChecked()) {
    m_toggleSmartExtendSelectedSwcNodeAction->setChecked(false);
    return;
  } else if (getSwcFilter()->getInteractionMode() ==
             Z3DSwcFilter::ConnectSwcNode) {
    getSwcFilter()->setInteractionMode(Z3DSwcFilter::Select);
    m_canvas->setCursor(Qt::ArrowCursor);
    return;
  } else if (m_canvas->suppressingContextMenu()) {
    return;
  }

  m_contextMenu = m_menuFactory->makeContextMenu(this, m_contextMenu);
  if (!m_contextMenu->isEmpty()) {
    m_contextMenu->popup(m_canvas->mapToGlobal(pt));
    return;
  }

  QList<QAction*> actions;

  if (m_doc->hasSelectedSwc() > 0) {
    //m_contextMenuGroup["swc"]->popup(m_canvas->mapToGlobal(pt));
    QList<QAction*> acts = m_contextMenuGroup["swc"]->actions();
    if (actions.empty()) {
      actions = acts;
    } else {
      while (true) {
        int i;
        for (i=0; i<actions.size(); ++i) {
          if (!acts.contains(actions[i])) {
            break;
          }
        }
        if (i == actions.size())
          break;
        else {
          actions.removeAt(i);
          continue;
        }
      }
    }
  } else {
    if (m_doc->hasSelectedSwcNode()) {
      updateContextMenu("swcnode");
      //m_contextMenuGroup["swcnode"]->popup(m_canvas->mapToGlobal(pt));
      QList<QAction*> acts = m_contextMenuGroup["swcnode"]->actions();
      if (actions.empty()) {
        actions = acts;
      } else {
        while (true) {
          int i;
          for (i=0; i<actions.size(); ++i) {
            if (!acts.contains(actions[i])) {
              break;
            }
          }
          if (i == actions.size())
            break;
          else {
            actions.removeAt(i);
            continue;
          }
        }
      }
    }
  }


  if (m_doc->hasSelectedPuncta() > 0) {
    updateContextMenu("puncta");
    //m_contextMenuGroup["puncta"]->popup(m_canvas->mapToGlobal(pt));
    QList<QAction*> acts = m_contextMenuGroup["puncta"]->actions();
    if (actions.empty()) {
      actions = acts;
    } else {
      while (true) {
        int i;
        for (i=0; i<actions.size(); ++i) {
          if (!acts.contains(actions[i])) {
            break;
          }
        }
        if (i == actions.size())
          break;
        else {
          actions.removeAt(i);
          continue;
        }
      }
    }
  }

  if (!m_doc->hasSelectedPuncta() &&
      !m_doc->hasSelectedSwc() &&
      !m_doc->hasSelectedSwcNode()) {

    // first see if pt hit any position in volume
    if (channelNumber() > 0) {
      bool success;
#ifdef _QT5_
      glm::vec3 fpos = m_volumeRaycaster->get3DPosition(
            pt.x() * qApp->devicePixelRatio(), pt.y() * qApp->devicePixelRatio(),
            m_canvas->width() * qApp->devicePixelRatio(), m_canvas->height() * qApp->devicePixelRatio(), success);
#else
      glm::vec3 fpos = m_volumeRaycaster->get3DPosition(
            pt.x(), pt.y(), m_canvas->width(), m_canvas->height(), success);
#endif
      if (success) {
        m_lastClickedPosInVolume = glm::ivec3(fpos);
        updateContextMenu("volume");
        //m_contextMenuGroup["volume"]->popup(m_canvas->mapToGlobal(pt));
        QList<QAction*> acts = m_contextMenuGroup["volume"]->actions();
        if (actions.empty()) {
          actions = acts;
        } else {
          for (int i=0; i<acts.size(); ++i)
            if (!actions.contains(acts[i]))
              actions.push_back(acts[i]);
        }
        if (!actions.empty()) {
          m_mergedContextMenu->clear();
          m_mergedContextMenu->addActions(actions);
          m_mergedContextMenu->popup(m_canvas->mapToGlobal(pt));
        }
        return;
      }
    }

    updateContextMenu("empty");
    //m_contextMenuGroup["empty"]->popup(m_canvas->mapToGlobal(pt));
    QList<QAction*> acts = m_contextMenuGroup["empty"]->actions();
    if (actions.empty()) {
      actions = acts;
    } else {
      for (int i=0; i<acts.size(); ++i)
        if (!actions.contains(acts[i]))
          actions.push_back(acts[i]);
    }
  }

  if (!actions.empty()) {
    m_mergedContextMenu->clear();
    m_mergedContextMenu->addActions(actions);
    m_mergedContextMenu->popup(m_canvas->mapToGlobal(pt));
  }
}

void Z3DWindow::traceTube()
{
  /*
  m_doc->executeTraceTubeCommand(m_lastClickedPosInVolume[0],
      m_lastClickedPosInVolume[1],
      m_lastClickedPosInVolume[2]);
      */
  m_canvas->setCursor(Qt::BusyCursor);
  m_doc->executeTraceSwcBranchCommand(m_lastClickedPosInVolume[0],
      m_lastClickedPosInVolume[1],
      m_lastClickedPosInVolume[2]);
  getSwcFilter()->setInteractionMode(Z3DSwcFilter::Select);
  m_canvas->setCursor(Qt::ArrowCursor);
}

void Z3DWindow::openZoomInView()
{
  if (hasVolume()) {
    if (m_volumeSource->openZoomInView(m_lastClickedPosInVolume)) {
      gotoPosition(m_volumeSource->getZoomInBound());
    }
  }
}

void Z3DWindow::exitZoomInView()
{
  if (hasVolume()) {
    m_volumeSource->exitZoomInView();
  }
}

void Z3DWindow::removeSelectedObject()
{
  m_doc->executeRemoveSelectedObjectCommand();
#if 0
  if (!m_doc->selectedSwcTreeNodes()->empty()) {
    m_doc->executeDeleteSwcNodeCommand();
  }

  if (!(m_doc->selectedChains()->empty() && m_doc->selectedPuncta()->empty() &&
        m_doc->selectedSwcs()->empty() )) {
    QUndoCommand *removeSelectedObjectsCommand =
        new ZStackDocRemoveSelectedObjectCommand(m_doc.get());
    m_doc->undoStack()->push(removeSelectedObjectsCommand);
  }
#endif
}

void Z3DWindow::markSelectedPunctaProperty1()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::TYPE_PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty1("true");
    m_doc->updatePunctaObjsModel(punctum);
//    m_doc->punctaObjsModel()->updateData(punctum);
  }
}

void Z3DWindow::markSelectedPunctaProperty2()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::TYPE_PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty2("true");
    m_doc->updatePunctaObjsModel(punctum);
//    m_doc->punctaObjsModel()->updateData(punctum);
  }
}

void Z3DWindow::markSelectedPunctaProperty3()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::TYPE_PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty3("true");
    m_doc->updatePunctaObjsModel(punctum);
  }
}

void Z3DWindow::unmarkSelectedPunctaProperty1()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::TYPE_PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty1("");
    m_doc->updatePunctaObjsModel(punctum);
  }
}

void Z3DWindow::unmarkSelectedPunctaProperty2()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::TYPE_PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty2("");
    m_doc->updatePunctaObjsModel(punctum);
  }
}

void Z3DWindow::unmarkSelectedPunctaProperty3()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::TYPE_PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty3("");
    m_doc->updatePunctaObjsModel(punctum);
  }
}

void Z3DWindow::saveSelectedPunctaAs()
{
  QString filename =
    QFileDialog::getSaveFileName(this, tr("Save Selected Puncta"), m_lastOpenedFilePath,
         tr("Puncta files (*.apo) "));

  if (!filename.isEmpty()) {
    m_lastOpenedFilePath = filename;
    QList<ZPunctum*> punctaList =
        m_doc->getSelectedObjectList<ZPunctum>(ZStackObject::TYPE_PUNCTUM);
    ZPunctumIO::save(filename, punctaList.begin(), punctaList.end());
  }
}

void Z3DWindow::addTodoMarker()
{
  QList<Swc_Tree_Node*> swcNodeList = getDocument()->getSelectedSwcNodeList();
  if (swcNodeList.size() == 1) {
    ZIntPoint pt = SwcTreeNode::center(swcNodeList.front()).toIntPoint();
    emit addingTodoMarker(pt.getX(), pt.getY(), pt.getZ(), false);
  }
}

void Z3DWindow::addDoneMarker()
{
  QList<Swc_Tree_Node*> swcNodeList = getDocument()->getSelectedSwcNodeList();
  if (swcNodeList.size() == 1) {
    ZIntPoint pt = SwcTreeNode::center(swcNodeList.front()).toIntPoint();
    emit addingTodoMarker(pt.getX(), pt.getY(), pt.getZ(), true);
  }
}

void Z3DWindow::updateBody()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    doc->forceBodyUpdate();
  }
}

void Z3DWindow::changeSelectedPunctaName()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("New Name"),
                                       tr("Punctum name:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok) {
    //std::set<ZPunctum*> *punctumSet = m_doc->selectedPuncta();
    QList<ZPunctum*> punctumSet =
        m_doc->getSelectedObjectList<ZPunctum>(ZStackObject::TYPE_PUNCTUM);
    for (QList<ZPunctum*>::iterator iter = punctumSet.begin();
         iter != punctumSet.end(); ++iter) {
      ZPunctum *punctum = *iter;
      punctum->setName(text);
    }
    m_doc->notifyPunctumModified();
  }
}

void Z3DWindow::saveAllPunctaAs()
{
  QString filename =
    QFileDialog::getSaveFileName(this, tr("Save All Puncta"), m_lastOpenedFilePath,
         tr("Puncta files (*.apo) "));

  if (!filename.isEmpty()) {
    m_lastOpenedFilePath = filename;
    QList<ZPunctum*> punctaList = m_doc->getPunctumList();
    ZPunctumIO::save(filename, punctaList.begin(), punctaList.end());
  }
}

void Z3DWindow::markPunctum()
{
  m_doc->markPunctum(m_lastClickedPosInVolume[0], m_lastClickedPosInVolume[1], m_lastClickedPosInVolume[2]);
}

void Z3DWindow::takeScreenShot(QString filename, int width, int height, Z3DScreenShotType sst)
{
  if (!m_canvasRenderer->renderToImage(filename, width, height, sst)) {
    QMessageBox::critical(this, "Error", m_canvasRenderer->getRenderToImageError());
  }
}

void Z3DWindow::takeScreenShot(QString filename, Z3DScreenShotType sst)
{
  int h = m_canvas->height();
  if (h % 2 == 1) {
    ++h;
  }
  int w = m_canvas->width();
  if (w % 2 == 1) {
    ++w;
  }
  if (m_canvas->width() % 2 == 1 || m_canvas->height() % 2 == 1) {
    LINFO() << "Resize canvas size from (" << m_canvas->width() << "," << m_canvas->height() << ") to (" << w << "," << h << ").";
    m_canvas->resize(w, h);
  }
  if (!m_canvasRenderer->renderToImage(filename, sst)) {
    QMessageBox::critical(this, "Error", m_canvasRenderer->getRenderToImageError());
  }
  //m_compositor->savePickingBufferToImage(filename + "_pickingBuffer.tif");
}

void Z3DWindow::openAdvancedSetting(const QString &name)
{
  if (!m_advancedSettingDockWidget) {
    m_advancedSettingDockWidget = new QDockWidget(tr("Advanced Settings"), this);
    m_advancedSettingDockWidget->setContentsMargins(20, 20, 20, 20);
    m_advancedSettingDockWidget->setMinimumSize(512, 512);
    QTabWidget *tabs = createAdvancedSettingTabWidget();
    m_advancedSettingDockWidget->setWidget(tabs);
    addDockWidget(Qt::RightDockWidgetArea, m_advancedSettingDockWidget);
    m_advancedSettingDockWidget->setFloating(true);
    m_viewMenu->addAction(m_advancedSettingDockWidget->toggleViewAction());
  }
  m_advancedSettingDockWidget->showNormal();
  QTabWidget *tabWidget = qobject_cast<QTabWidget*>(m_advancedSettingDockWidget->widget());
  for (int i=0; i<tabWidget->count(); i++) {
    if (tabWidget->tabText(i) == name) {
      tabWidget->setCurrentIndex(i);
      break;
    }
  }
}

void Z3DWindow::takeSeriesScreenShot(const QDir &dir, const QString &namePrefix, glm::vec3 axis,
                                 bool clockWise, int numFrame, int width, int height, Z3DScreenShotType sst)
{
  QString title = "Capturing Images...";
  if (sst == HalfSideBySideStereoView)
    title = "Capturing Half Side-By-Side Stereo Images...";
  else if (sst == FullSideBySideStereoView)
    title = "Capturing Full Side-By-Side Stereo Images...";
  QProgressDialog progress(title, "Cancel", 0, numFrame, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.show();
  double rAngle = M_PI * 2. / numFrame;
  for (int i=0; i<numFrame; i++) {
    progress.setValue(i);
    if (progress.wasCanceled())
      break;

    if (clockWise)
      getCamera()->rotate(rAngle, getCamera()->vectorEyeToWorld(axis), getCamera()->getCenter());
    else
      getCamera()->rotate(-rAngle, getCamera()->vectorEyeToWorld(axis), getCamera()->getCenter());
    resetCameraClippingRange();
    int fieldWidth = numDigits(numFrame);
    QString filename = QString("%1%2.tif").arg(namePrefix).arg(i, fieldWidth, 10, QChar('0'));
    QString filepath = dir.filePath(filename);
    takeScreenShot(filepath, width, height, sst);
  }
  progress.setValue(numFrame);
}

void Z3DWindow::takeSeriesScreenShot(const QDir &dir, const QString &namePrefix, glm::vec3 axis,
                                 bool clockWise, int numFrame, Z3DScreenShotType sst)
{
  QString title = "Capturing Images...";
  if (sst == HalfSideBySideStereoView)
    title = "Capturing Half Side-By-Side Stereo Images...";
  else if (sst == FullSideBySideStereoView)
    title = "Capturing Full Side-By-Side Stereo Images...";
  QProgressDialog progress(title, "Cancel", 0, numFrame, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.show();
  double rAngle = M_PI * 2. / numFrame;
  for (int i=0; i<numFrame; i++) {
    progress.setValue(i);
    if (progress.wasCanceled())
      break;

    if (clockWise)
      getCamera()->rotate(rAngle, getCamera()->vectorEyeToWorld(axis), getCamera()->getCenter());
    else
      getCamera()->rotate(-rAngle, getCamera()->vectorEyeToWorld(axis), getCamera()->getCenter());
    resetCameraClippingRange();
    int fieldWidth = numDigits(numFrame);
    QString filename = QString("%1%2.tif").arg(namePrefix).arg(i, fieldWidth, 10, QChar('0'));
    QString filepath = dir.filePath(filename);
    takeScreenShot(filepath, sst);
  }
  progress.setValue(numFrame);
}

void Z3DWindow::updateSettingsDockWidget()
{
  //  QScrollArea *oldSA = qobject_cast<QScrollArea*>(m_settingsDockWidget->widget());
  //  int oldScrollBarValue = 0;
  //  if (oldSA) {
  //    QScrollBar *bar = oldSA->verticalScrollBar();
  //    oldScrollBarValue = bar->value();
  //  }
  //  QScrollArea *newSA = qobject_cast<QScrollArea*>(m_widgetsGroup->createWidget(this, true));
  //  newSA->verticalScrollBar()->setValue(oldScrollBarValue);
  QTabWidget *old = qobject_cast<QTabWidget*>(m_settingsDockWidget->widget());
  int oldIndex = 0;
  if (old)
    oldIndex = old->currentIndex();
  QTabWidget *tabs = createBasicSettingTabWidget();
  if (oldIndex >= 0 && oldIndex < tabs->count())
    tabs->setCurrentIndex(oldIndex);
  m_settingsDockWidget->setUpdatesEnabled(false);
  m_settingsDockWidget->setWidget(tabs);
  m_settingsDockWidget->setUpdatesEnabled(true);
  if (old) {
    old->setParent(NULL);
    delete old;
  }

  // for advanced setting widget
  if (m_advancedSettingDockWidget) {
    QTabWidget *old = qobject_cast<QTabWidget*>(m_advancedSettingDockWidget->widget());
    int oldIndex = 0;
    if (old)
      oldIndex = old->currentIndex();
    QTabWidget *tabs = createAdvancedSettingTabWidget();
    if (oldIndex >= 0 && oldIndex < tabs->count())
      tabs->setCurrentIndex(oldIndex);
    m_advancedSettingDockWidget->setUpdatesEnabled(false);
    m_advancedSettingDockWidget->setWidget(tabs);
    m_advancedSettingDockWidget->setUpdatesEnabled(true);
    if (old) {
      old->setParent(NULL);
      delete old;
    }
  }
}

void Z3DWindow::toogleAddSwcNodeMode(bool checked)
{
  if (checked) {
    //    if (m_toogleExtendSelectedSwcNodeAction->isChecked()) {
    //      m_toogleExtendSelectedSwcNodeAction->blockSignals(true);
    //      m_toogleExtendSelectedSwcNodeAction->setChecked(false);
    //      m_toogleExtendSelectedSwcNodeAction->blockSignals(false);
    //    }
    if (m_toggleSmartExtendSelectedSwcNodeAction->isChecked()) {
      m_toggleSmartExtendSelectedSwcNodeAction->blockSignals(true);
      m_toggleSmartExtendSelectedSwcNodeAction->setChecked(false);
      m_toggleSmartExtendSelectedSwcNodeAction->blockSignals(false);
    }
    m_swcFilter->setInteractionMode(Z3DSwcFilter::AddSwcNode);
    m_canvas->getInteractionContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_ADD_NODE);
    //m_canvas->setCursor(Qt::PointingHandCursor);
    notifyUser("Click to add a node");
  } else {
    m_swcFilter->setInteractionMode(Z3DSwcFilter::Select);
    m_canvas->getInteractionContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_OFF);
    //m_canvas->setCursor(Qt::ArrowCursor);
  }
  m_canvas->updateCursor();
}

//void Z3DWindow::toogleExtendSelectedSwcNodeMode(bool checked)
//{
//  if (checked) {
//    if (m_toogleAddSwcNodeModeAction->isChecked()) {
//      m_toogleAddSwcNodeModeAction->blockSignals(true);
//      m_toogleAddSwcNodeModeAction->setChecked(false);
//      m_toogleAddSwcNodeModeAction->blockSignals(false);
//    }
//    if (m_toogleSmartExtendSelectedSwcNodeAction->isChecked()) {
//      m_toogleSmartExtendSelectedSwcNodeAction->blockSignals(true);
//      m_toogleSmartExtendSelectedSwcNodeAction->setChecked(false);
//      m_toogleSmartExtendSelectedSwcNodeAction->blockSignals(false);
//    }
//    m_canvas->setCursor(Qt::PointingHandCursor);
//  } else {
//    m_canvas->setCursor(Qt::ArrowCursor);
//  }
//}

void Z3DWindow::toogleSmartExtendSelectedSwcNodeMode(bool checked)
{
  if (checked) {
    if (m_toogleAddSwcNodeModeAction->isChecked()) {
      m_toogleAddSwcNodeModeAction->blockSignals(true);
      m_toogleAddSwcNodeModeAction->setChecked(false);
      m_toogleAddSwcNodeModeAction->blockSignals(false);
    }
    //    if (m_toogleExtendSelectedSwcNodeAction->isChecked()) {
    //      m_toogleExtendSelectedSwcNodeAction->blockSignals(true);
    //      m_toogleExtendSelectedSwcNodeAction->setChecked(false);
    //      m_toogleExtendSelectedSwcNodeAction->blockSignals(false);
    //    }
    notifyUser("Left click to extend. Path calculation is off when 'Cmd/Ctrl' is held."
               "Right click to exit extending mode.");
    if (getDocument()->hasStackData()) {
      m_swcFilter->setInteractionMode(Z3DSwcFilter::SmartExtendSwcNode);
    } else {
      m_swcFilter->setInteractionMode(Z3DSwcFilter::PlainExtendSwcNode);
    }
    m_canvas->getInteractionContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_SMART_EXTEND);
    //m_canvas->setCursor(Qt::PointingHandCursor);
  } else {
    m_swcFilter->setInteractionMode(Z3DSwcFilter::Select);
    m_canvas->getInteractionContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_OFF);
    //m_canvas->setCursor(Qt::ArrowCursor);
  }
  m_canvas->updateCursor();
}

void Z3DWindow::changeBackground()
{
  m_settingsDockWidget->show();
  int index = m_widgetsGroup->getChildGroups().indexOf(m_compositor->getBackgroundWidgetsGroup());
  QTabWidget *tab = qobject_cast<QTabWidget*>(m_settingsDockWidget->widget());
  tab->setCurrentIndex(index);
}

bool Z3DWindow::isBackgroundOn() const
{
  return m_compositor->showingBackground();
}

void Z3DWindow::toogleMoveSelectedObjectsMode(bool checked)
{
  getInteractionHandler()->setMoveObjects(checked);
  m_canvas->updateCursor();
  if (checked) {
    notifyUser("Shift + Mouse to move selected objects");
  }
}

void Z3DWindow::moveSelectedObjects(double x, double y, double z)
{
  m_doc->executeMoveObjectCommand(x, y, z, m_punctaFilter->getCoordScales().x,
                                  m_punctaFilter->getCoordScales().y,
                                  m_punctaFilter->getCoordScales().z,
                                  m_swcFilter->getCoordScales().x,
                                  m_swcFilter->getCoordScales().y,
                                  m_swcFilter->getCoordScales().z);

#if 0
  if (m_doc->selectedSwcs()->empty() && m_doc->selectedPuncta()->empty() &&
      m_doc->selectedSwcTreeNodes()->empty())
    return;
  ZStackDocMoveSelectedObjectCommand *moveSelectedObjectCommand =
      new ZStackDocMoveSelectedObjectCommand(m_doc.get(),
                                             x, y, z);
  moveSelectedObjectCommand->setPunctaCoordScale(m_punctaFilter->getCoordScales().x,
                                                 m_punctaFilter->getCoordScales().y,
                                                 m_punctaFilter->getCoordScales().z);
  moveSelectedObjectCommand->setSwcCoordScale(m_swcFilter->getCoordScales().x,
                                              m_swcFilter->getCoordScales().y,
                                              m_swcFilter->getCoordScales().z);
  m_doc->undoStack()->push(moveSelectedObjectCommand);
#endif
}

void Z3DWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void Z3DWindow::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();
  m_doc->loadFileList(urls);
}

void Z3DWindow::closeEvent(QCloseEvent */*event*/)
{
  writeSettings();
  emit closed();
}

void Z3DWindow::keyPressEvent(QKeyEvent *event)
{
  ZInteractionEngine::EKeyMode keyMode = ZInteractionEngine::KM_NORMAL;
  switch(event->key())
  {
  case Qt::Key_Backspace:
  case Qt::Key_Delete:
  {
    deleteSelectedSwcNode();
    removeSelectedObject();
  }
    break;
  case Qt::Key_A:
    if (event->modifiers() == Qt::ControlModifier) {
      getDocument()->selectAllSwcTreeNode();
    }
    break;
  case Qt::Key_C:
  if (getDocument()->getTag() != NeuTube::Document::FLYEM_BODY_3D_COARSE &&
      getDocument()->getTag() != NeuTube::Document::FLYEM_BODY_3D){
    if (event->modifiers() == Qt::ControlModifier) {
      std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
      if (nodeSet.size() > 0) {
        SwcTreeNode::clearClipboard();
      }
      for (std::set<Swc_Tree_Node*>::const_iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        SwcTreeNode::addToClipboard(*iter);
      }
    } else if (event->modifiers() == Qt::NoModifier) {
      if (m_doc->hasSelectedSwcNode()) {
        if (m_doc->getSelectedSwcNodeNumber() > 1) {
          m_doc->executeConnectSwcNodeCommand();
        } else {
          if (m_toggleMoveSelectedObjectsAction->isChecked()) {
            m_toggleMoveSelectedObjectsAction->toggle();
          }
          if (m_toggleSmartExtendSelectedSwcNodeAction->isChecked()) {
            m_toggleSmartExtendSelectedSwcNodeAction->toggle();
          }
          startConnectingSwcNode();
        }
      }
    }
  }
    break;
  case Qt::Key_P:
  {
    if (event->modifiers() == Qt::ControlModifier) {
      for (size_t i = 0; i < SwcTreeNode::clipboard().size(); ++i) {
        std::cout << SwcTreeNode::clipboard()[i] << std::endl;
      }
    }
  }
    break;
  case Qt::Key_S:
    if (event->modifiers() == Qt::ControlModifier) {
      m_doc->saveSwc(this);
    } else if (event->modifiers() == Qt::NoModifier) {
      if (getDocument()->getTag() == NeuTube::Document::NORMAL ||
          getDocument()->getTag() == NeuTube::Document::FLYEM_SKELETON ||
          getDocument()->getTag() == NeuTube::Document::BIOCYTIN_STACK) {
        keyMode = ZInteractionEngine::KM_SWC_SELECTION;
      }
    }
    break;
  case Qt::Key_I:
    getDocument()->executeInsertSwcNode();
    break;
  case Qt::Key_B:
#ifdef _DEBUG_2
    if (event->modifiers() == Qt::ControlModifier) {
      QList<ZSwcTree*> *treeList = m_doc->swcList();
      foreach (ZSwcTree *tree, *treeList) {
        ZString source = tree->source();
        std::cout << source.lastInteger() << std::endl;
      }
    }
#endif
    if (event->modifiers() == Qt::NoModifier) {
      m_doc->executeBreakSwcConnectionCommand();
    }
    break;
  case Qt::Key_Equal: // increase swc size scale
  {
    if (event->modifiers() == Qt::ControlModifier) {
      getSwcFilter()->setSizeScale(getSwcFilter()->getSizeScale() + .1);
    }
  }
    break;
  case Qt::Key_Minus:  // decrease swc size scale
  {
    if (event->modifiers() == Qt::ControlModifier) {
      getSwcFilter()->setSizeScale(std::max(.1, getSwcFilter()->getSizeScale() - .1));
    }
  }
    break;
  case Qt::Key_G:  // change swc display mode
  {
    if (event->modifiers() == Qt::ControlModifier) {
      ZOptionParameter<QString> *sm =
          (ZOptionParameter<QString>*) (
            getSwcFilter()->getParameter("Geometry"));
      sm->selectNext();
    }
  }
    break;
  case Qt::Key_Comma:
  case Qt::Key_Q:
    getDocument()->executeSwcNodeChangeSizeCommand(-0.5);
    break;
  case Qt::Key_Period:
  case Qt::Key_E:
    getDocument()->executeSwcNodeChangeSizeCommand(0.5);
    break;
  case Qt::Key_X:
    if (event->modifiers() == Qt::NoModifier) {
      getDocument()->executeDeleteSwcNodeCommand();
    }
    break;
  case Qt::Key_N:
    if (event->modifiers() == Qt::NoModifier) {
      getDocument()->executeConnectIsolatedSwc();
    }
    break;
  case Qt::Key_Z:
    if (event->modifiers() == Qt::NoModifier) {
      ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
      bool located = false;
      if (doc != NULL) {
        ZFlyEmToDoItem *item = doc->getOneSelectedTodoItem();
        if (item !=NULL) {
          locate2DView(item->getPosition().toPoint(), item->getRadius());
          located = true;
        }
        /*
        if (doc->hadTodoItemSelected()) {
          locateTodoItemIn2DView();
        }
        */
      }

      if (!located) {
        if (getDocument()->hasSelectedSwcNode()) {
          locateSwcNodeIn2DView();
        } else if (getDocument()->hasSelectedPuncta()) {
          locatePunctumIn2DView();
        }
      }
    }
    break;
  case Qt::Key_V:
    if (event->modifiers() == Qt::NoModifier) {
      if (!m_toggleMoveSelectedObjectsAction->isChecked()) {
        if (m_toggleSmartExtendSelectedSwcNodeAction->isChecked()) {
          m_toggleSmartExtendSelectedSwcNodeAction->toggle();
        }
        if (getSwcFilter()->getInteractionMode() == Z3DSwcFilter::ConnectSwcNode) {
            getSwcFilter()->setInteractionMode(Z3DSwcFilter::Select);
            m_canvas->setCursor(Qt::ArrowCursor);
        }
        m_toggleMoveSelectedObjectsAction->toggle();
      }
    }
    break;
  case Qt::Key_Space:
    if (getDocument()->getSelectedSwcNodeNumber() == 1) {
      if (!m_toggleSmartExtendSelectedSwcNodeAction->isChecked()) {
        if (m_toggleMoveSelectedObjectsAction->isChecked()) {
          m_toggleMoveSelectedObjectsAction->toggle();
        }
        if (getSwcFilter()->getInteractionMode() == Z3DSwcFilter::ConnectSwcNode) {
            getSwcFilter()->setInteractionMode(Z3DSwcFilter::Select);
            m_canvas->setCursor(Qt::ArrowCursor);
        }
        m_toggleSmartExtendSelectedSwcNodeAction->toggle();
      }
    } else {
      if (GET_APPLICATION_NAME == "FlyEM") {
        if (event->modifiers() == Qt::ShiftModifier) {
          QCursor oldCursor = m_canvas->cursor();
          m_canvas->setCursor(Qt::BusyCursor);
          getDocument()->runSeededWatershed();
          notifyUser("Body splitted");
          m_canvas->setCursor(oldCursor);
        }
      }
    }
    break;
  case Qt::Key_R:
    if (event->modifiers() == Qt::NoModifier) {
      m_doc->executeResetBranchPoint();
    } else {
      m_doc->executeSetRootCommand();
    }
    break;
  case Qt::Key_1:
    if (getCanvas()->getInteractionEngine()->getKeyMode() ==
        ZInteractionEngine::KM_SWC_SELECTION) {
      m_doc->selectDownstreamNode();
    }
    break;
  case Qt::Key_2:
    if (getCanvas()->getInteractionEngine()->getKeyMode() ==
        ZInteractionEngine::KM_SWC_SELECTION) {
      m_doc->selectUpstreamNode();
    }
    break;
  case Qt::Key_3:
    if (getCanvas()->getInteractionEngine()->getKeyMode() ==
        ZInteractionEngine::KM_SWC_SELECTION) {
      m_doc->selectConnectedNode();
    }
    break;
  case Qt::Key_4:
    if (getCanvas()->getInteractionEngine()->getKeyMode() ==
        ZInteractionEngine::KM_SWC_SELECTION) {
      m_doc->inverseSwcNodeSelection();
    }
    break;
  case Qt::Key_5:
    if (getCanvas()->getInteractionEngine()->getKeyMode() ==
        ZInteractionEngine::KM_SWC_SELECTION) {
      m_doc->selectNoisyTrees();
    }
    break;
  default:
    break;
  }

  getCanvas()->setKeyMode(keyMode);
}

QTabWidget *Z3DWindow::createBasicSettingTabWidget()
{
  QTabWidget *tabs = new QTabWidget();
  tabs->setElideMode(Qt::ElideNone);
  tabs->setUsesScrollButtons(true);
  const QList<ZWidgetsGroup*>& groups = m_widgetsGroup->getChildGroups();
  for (int i=0; i<groups.size(); i++) {
    if (groups[i]->isGroup()) {
      QWidget *widget = groups[i]->createWidget(this, true);
      tabs->addTab(widget, groups[i]->getGroupName());
      //widget->setVisible(groups[i]->isVisible());
    }
  }
  return tabs;
}

QTabWidget *Z3DWindow::createAdvancedSettingTabWidget()
{
  QTabWidget *tabs = new QTabWidget();
  tabs->setElideMode(Qt::ElideNone);
  const QList<ZWidgetsGroup*>& groups = m_widgetsGroup->getChildGroups();
  for (int i=0; i<groups.size(); i++) {
    if (groups[i]->isGroup() && groups[i]->getGroupName() != "Capture" &&
        groups[i]->getGroupName() != "Utils") {
      tabs->addTab(groups[i]->createWidget(this, false), groups[i]->getGroupName());
    }
  }
  return tabs;
}

void Z3DWindow::updateContextMenu(const QString &group)
{
  if (group == "empty") {
    m_contextMenuGroup["empty"]->clear();
    if (channelNumber() > 0 && m_volumeSource->volumeNeedDownsample() &&
        m_volumeSource->isSubvolume()) {
      m_contextMenuGroup["empty"]->addAction(m_exitVolumeZoomInViewAction);
    }
    //if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      //m_contextMenuGroup["empty"]->addAction(m_toogleExtendSelectedSwcNodeAction);
    /*
    if (channelNumber() > 0 && !m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["empty"]->addAction(m_toogleSmartExtendSelectedSwcNodeAction);
*/
    if (m_doc->hasSwc() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["empty"]->addAction(m_toogleAddSwcNodeModeAction);
    if (m_doc->hasSwc() || m_doc->hasPuncta())
      m_contextMenuGroup["empty"]->addAction(m_toggleMoveSelectedObjectsAction);
    m_contextMenuGroup["empty"]->addAction(m_changeBackgroundAction);

  }
  if (group == "volume") {
    m_contextMenuGroup["volume"]->clear();
    if (m_volumeSource->volumeNeedDownsample()) {
      if (m_volumeSource->isSubvolume()) {
        m_contextMenuGroup["volume"]->addAction(m_exitVolumeZoomInViewAction);
      } else {
        m_contextMenuGroup["volume"]->addAction(m_openVolumeZoomInViewAction);
      }
    }
    //m_contextMenuGroup["volume"]->addAction(m_markPunctumAction);
    //if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      //m_contextMenuGroup["volume"]->addAction(m_toogleExtendSelectedSwcNodeAction);
    /*
    if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["volume"]->addAction(m_toogleSmartExtendSelectedSwcNodeAction);
*/
    if (m_doc->hasSwc() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["volume"]->addAction(m_toogleAddSwcNodeModeAction);
    if (m_doc->hasSwc() || m_doc->hasPuncta())
      m_contextMenuGroup["volume"]->addAction(m_toggleMoveSelectedObjectsAction);
    m_contextMenuGroup["volume"]->addAction(m_changeBackgroundAction);
    m_contextMenuGroup["volume"]->addAction(m_refreshTraceMaskAction);
    if (m_doc->getTag() == NeuTube::Document::FLYEM_SPLIT) {
      m_contextMenuGroup["volume"]->addAction(m_markPunctumAction);
    }
  }
  if (group == "swcnode") {
    getDocument()->updateSwcNodeAction();
    m_singleSwcNodeActionActivator.update(this);
  }
  if (group == "puncta") {
    m_saveSelectedPunctaAsAction->setEnabled(!m_doc->hasSelectedPuncta());
    m_saveAllPunctaAsAction->setEnabled(m_doc->hasPuncta());
    m_locatePunctumIn2DAction->setEnabled(
          m_doc->getSelected(ZStackObject::TYPE_PUNCTUM).size() == 1);
  }
}

void Z3DWindow::updateOverallBoundBox(std::vector<double> bound)
{
  if (bound.size() == 6) {
    if (bound[1] > bound[0]) {
      m_boundBox[0] = std::min(bound[0], m_boundBox[0]);
      m_boundBox[1] = std::max(bound[1], m_boundBox[1]);
    }
    if (bound[3] > bound[2]) {
      m_boundBox[2] = std::min(bound[2], m_boundBox[2]);
      m_boundBox[3] = std::max(bound[3], m_boundBox[3]);
    }
    if (bound[5] > bound[4]) {
      m_boundBox[4] = std::min(bound[4], m_boundBox[4]);
      m_boundBox[5] = std::max(bound[5], m_boundBox[5]);
    }
  }
}

void Z3DWindow::updateOverallBoundBox()
{
  m_boundBox[0] = m_boundBox[2] = m_boundBox[4] = std::numeric_limits<double>::max();
  m_boundBox[1] = m_boundBox[3] = m_boundBox[5] = -std::numeric_limits<double>::max();
  if (hasVolume()) {
    updateOverallBoundBox(m_volumeBoundBox);
  }
  updateOverallBoundBox(m_swcBoundBox);
  updateOverallBoundBox(m_punctaBoundBox);
  updateOverallBoundBox(m_graphBoundBox);
  updateOverallBoundBox(m_decorationBoundBox);
  updateOverallBoundBox(m_surfaceBoundBox);
  if (m_boundBox[0] > m_boundBox[1] || m_boundBox[2] > m_boundBox[3] || m_boundBox[4] > m_boundBox[5]) {
    // nothing visible
    m_boundBox[0] = m_boundBox [2] = m_boundBox[4] = 0.0f;
    m_boundBox[1] = m_boundBox [3] = m_boundBox[5] = 1.0f;
  }
}

void Z3DWindow::changeSelectedSwcNodeType()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
  if (nodeSet.size() > 0) {
    SwcTypeDialog dlg(ZSwcTree::SWC_NODE, NULL);
    if (dlg.exec()) {
      switch (dlg.pickingMode()) {
      case SwcTypeDialog::INDIVIDUAL:
        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
             iter != nodeSet.end(); ++iter) {
          SwcTreeNode::setType(*iter, dlg.type());
        }
        break;
      case SwcTypeDialog::DOWNSTREAM:
        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
             iter != nodeSet.end(); ++iter) {
          SwcTreeNode::setDownstreamType(*iter, dlg.type());
        }
        break;
      case SwcTypeDialog::CONNECTION:
      {
        Swc_Tree_Node *ancestor = *(nodeSet.begin());

        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
             iter != nodeSet.end(); ++iter) {
          ancestor = SwcTreeNode::commonAncestor(ancestor, *iter);
        }

        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
             iter != nodeSet.end(); ++iter) {
          SwcTreeNode::setUpstreamType(*iter, dlg.type(), ancestor);
        }
      }
        break;
      case SwcTypeDialog::LONGEST_LEAF:
      {
        Swc_Tree_Node *tn = SwcTreeNode::furthestNode(*(nodeSet.begin()),
                                                      SwcTreeNode::GEODESIC);
        SwcTreeNode::setPathType(*(nodeSet.begin()), tn, dlg.type());
      }
        break;
      default:
        break;
      }
      getSwcFilter()->addNodeType(dlg.type());

      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::setRootAsSelectedSwcNode()
{
  m_doc->executeSetRootCommand();
      /*
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();
  if (nodeSet->size() == 1) {
    m_doc->executeSetRootCommand();

    SwcTreeNode::setAsRoot(*nodeSet->begin());
    m_doc->notifySwcModified();

  }
      */
}

void Z3DWindow::breakSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
  if (nodeSet.size() >= 2) {
    m_doc->executeBreakSwcConnectionCommand();
#if 0
    for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
         iter != nodeSet->end(); ++iter) {
      if (nodeSet->count((*iter)->parent) > 0) {
        SwcTreeNode::detachParent(*iter);
        ZSwcTree *tree = new ZSwcTree();
        tree->setDataFromNode(*iter);
        /*
        std::ostringstream stream;
        stream << "node-break" << "-" << tree;
        tree->setSource(stream.str());
        */
        m_doc->addSwcTree(tree, false);
      }
    }

    m_doc->notifySwcModified();
#endif
    /*
    std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
    Swc_Tree_Node *tn1 = *iter;
    ++iter;
    Swc_Tree_Node *tn2 = *iter;
    Swc_Tree_Node *child = NULL;
    if (tn1->parent == tn2) {
      child = tn1;
    } else if (tn2->parent == tn1) {
      child = tn2;
    }
    if (child != NULL) {
      Swc_Tree_Node_Detach_Parent(child);
      ZSwcTree *tree = new ZSwcTree();
      tree->setDataFromNode(child);
      m_doc->addSwcTree(tree);
      m_doc->requestRedrawSwc();
    }
    */
  }
}


void Z3DWindow::mergeSelectedSwcNode()
{
  m_doc->executeMergeSwcNodeCommand();
#if 0
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();

  if (nodeSet->size() > 1) {
    Swc_Tree_Node *coreNode = SwcTreeNode::merge(*nodeSet);
    if (coreNode != NULL) {
      if (SwcTreeNode::parent(coreNode) == NULL) {
        ZSwcTree *tree = new ZSwcTree();
        tree->setDataFromNode(coreNode);
        //m_doc->addSwcTree(tree, false);
        m_doc->executeAddSwcCommand(tree);
      }

      m_doc->executeDeleteSwcNodeCommand();
      //SwcTreeNode::kill(*nodeSet);

      //m_doc->removeEmptySwcTree();
      m_doc->notifySwcModified();
    }
  }
#endif
}

void Z3DWindow::deleteSelectedSwcNode()
{
  m_doc->executeDeleteSwcNodeCommand();
}

void Z3DWindow::locateSwcNodeIn2DView()
{
  if (m_doc->hasSelectedSwcNode()) {
    if (m_doc->getParentFrame() != NULL) {
      m_doc->getParentFrame()->zoomToSelectedSwcNodes();
      m_doc->getParentFrame()->raise();
      /*
      ZCuboid cuboid = SwcTreeNode::boundBox(*m_doc->selectedSwcTreeNodes());
      int cx, cy, cz;
      ZPoint center = cuboid.center();
      cx = iround(center.x());
      cy = iround(center.y());
      cz = iround(center.z());
      int radius = iround(std::max(cuboid.width(), cuboid.height()) / 2.0);
      m_doc->getParentFrame()->viewRoi(cx, cy, cz, radius);
      */
    } else {
      const std::set<Swc_Tree_Node*> &nodeSet = m_doc->getSelectedSwcNodeSet();

      ZCuboid cuboid = SwcTreeNode::boundBox(nodeSet);
      ZPoint center = cuboid.center();
      int cx, cy, cz;

      //-= document()->getStackOffset();
      cx = iround(center.x());
      cy = iround(center.y());
      cz = iround(center.z());
      int radius = iround(std::max(cuboid.width(), cuboid.height()) / 2.0);
      const int minRadius = 400;
      if (radius < minRadius) {
        radius = minRadius;
      }

      ZStackViewParam param(NeuTube::COORD_STACK);
      param.setViewPort(iround(cx - radius), iround(cy - radius),
                        iround(cx + radius), iround(cy + radius));
      param.setZ(iround(cz));
      emit locating2DViewTriggered(param);
    }
  }
}

void Z3DWindow::locate2DView(const ZPoint &center, double radius)
{
  const int minRadius = 400;
  if (radius < minRadius) {
    radius = minRadius;
  }

  ZStackViewParam param(NeuTube::COORD_STACK);

  double cx = center.getX();
  double cy = center.getY();
  double cz = center.getZ();
  param.setViewPort(iround(cx - radius), iround(cy - radius),
                    iround(cx + radius), iround(cy + radius));
  param.setZ(iround(cz));

  emit locating2DViewTriggered(param);
}

void Z3DWindow::locatePunctumIn2DView()
{
  QList<ZPunctum*> punctumList =
      m_doc->getSelectedObjectList<ZPunctum>(ZStackObject::TYPE_PUNCTUM);
  if (punctumList.size() == 1) {
    ZPunctum* punc = *(punctumList.begin());

    locate2DView(punc->getCenter(), punc->radius());
#if 0
    ZStackViewParam param(NeuTube::COORD_STACK);

    double radius = punc->radius();
    const int minRadius = 400;
    if (radius < minRadius) {
      radius = minRadius;
    }

    double cx = punc->x();
    double cy = punc->y();
    double cz = punc->z();
    param.setViewPort(iround(cx - radius), iround(cy - radius),
                      iround(cx + radius), iround(cy + radius));
    param.setZ(iround(cz));

    emit locating2DViewTriggered(param);
    //      m_doc->getParentFrame()->viewRoi(
    //            punc->x(), punc->y(), iround(punc->z()), punc->radius() * 4);
#endif
  }
}

void Z3DWindow::tranlateSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();

  if (!nodeSet.empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    if (SwcTreeNode::clipboard().size() >= 2) {
      Swc_Tree_Node node[2];
      for (size_t i = 0; i < 2; ++i) {
        SwcTreeNode::paste(node + i, i);
      }

      ZPoint offset = SwcTreeNode::center(node + 1) - SwcTreeNode::center(node);
      dlg.setTranslateValue(offset.x(), offset.y(), offset.z());
    }
    if (dlg.exec()) {
      double dx = dlg.getTranslateValue(SwcSkeletonTransformDialog::X);
      double dy = dlg.getTranslateValue(SwcSkeletonTransformDialog::Y);
      double dz = dlg.getTranslateValue(SwcSkeletonTransformDialog::Z);

      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        if (dlg.isTranslateFirst()) {
          SwcTreeNode::translate(*iter, dx, dy, dz);
        }

        SwcTreeNode::setPos(
              *iter,
              SwcTreeNode::x(*iter) * dlg.getScaleValue(SwcSkeletonTransformDialog::X),
              SwcTreeNode::y(*iter) * dlg.getScaleValue(SwcSkeletonTransformDialog::Y),
              SwcTreeNode::z(*iter) * dlg.getScaleValue(SwcSkeletonTransformDialog::Z));

        if (!dlg.isTranslateFirst()) {
          SwcTreeNode::translate(*iter, dx, dy, dz);
        }
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::connectSelectedSwcNode()
{
  m_doc->executeConnectSwcNodeCommand();
  /*
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();
  if (SwcTreeNode::connect(*nodeSet)) {
    m_doc->removeEmptySwcTree();
    m_doc->notifySwcModified();
  }
  */
}

void Z3DWindow::changeSelectedSwcType()
{
  //std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::TYPE_SWC);

  if (!treeSet.empty()) {
    SwcTypeDialog dlg(ZSwcTree::WHOLE_TREE, NULL);

    if (dlg.exec()) {
      switch (dlg.pickingMode()) {
      case SwcTypeDialog::INDIVIDUAL:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          (*iter)->setType(dlg.type());
        }
        break;
      case SwcTypeDialog::MAIN_TRUNK:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.setDistanceWeight(15.0, 1.0);
          ZSwcPath branch = (*iter)->mainTrunk(&trunkAnalyzer);
          branch.setType(dlg.type());
        }
        break;
      case SwcTypeDialog::LONGEST_LEAF:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.setDistanceWeight(0.0, 1.0);
          ZSwcPath branch = (*iter)->mainTrunk(&trunkAnalyzer);
          branch.setType(dlg.type());
        }
        break;
      case SwcTypeDialog::FURTHEST_LEAF:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.setDistanceWeight(1.0, 0.0);
          ZSwcPath branch = (*iter)->mainTrunk(&trunkAnalyzer);
          branch.setType(dlg.type());
        }
        break;
      case SwcTypeDialog::TRAFFIC:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          ZSwcDistTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.labelTraffic(*iter, ZSwcTrunkAnalyzer::REACH_ROOT);
          (*iter)->setTypeByLabel();
        }
        break;
      case SwcTypeDialog::TRUNK_LEVEL:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          //ZSwcDistTrunkAnalyzer trunkAnalyzer;
          ZSwcWeightTrunkAnalyzer trunkAnalyzer;
          (*iter)->setBranchSizeWeight();
          (*iter)->labelTrunkLevel(&trunkAnalyzer);
          //trunkAnalyzer.labelTrunk(*iter);
          (*iter)->setTypeByLabel();
        }
        break;
      case SwcTypeDialog::BRANCH_LEVEL:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          //(*iter)->labelBranchLevel(0);
          (*iter)->labelBranchLevelFromLeaf();
          (*iter)->setTypeByLabel();
        }
        break;
      case SwcTypeDialog::ROOT:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          Swc_Tree_Node *tn = (*iter)->firstRegularRoot();
          while (tn != NULL) {
            SwcTreeNode::setType(tn, dlg.type());
            tn = SwcTreeNode::nextSibling(tn);
          }
        }
        break;
      case SwcTypeDialog::SUBTREE:
      {
        ZSwcSubtreeAnalyzer analyzer;
        analyzer.setMinLength(10000.0);
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          ZSwcTree *tree = *iter;
          analyzer.decompose(tree);
          tree->setTypeByLabel();
        }
      }
      default:
        break;
      }

      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::changeSelectedSwcSize()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::TYPE_SWC);

  if (!treeSet.empty()) {
    SwcSizeDialog dlg(NULL);
    if (dlg.exec()) {
      for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
           iter != treeSet.end(); ++iter) {
        (*iter)->changeRadius(dlg.getAddValue(), dlg.getMulValue());
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::changeSelectedSwcNodeSize()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();

  if (!nodeSet.empty()) {
    SwcSizeDialog dlg(NULL);
    if (dlg.exec()) {
      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        SwcTreeNode::changeRadius(*iter, dlg.getAddValue(), dlg.getMulValue());
      }

      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::transformSelectedPuncta()
{
  std::set<ZPunctum*> punctaSet =
      m_doc->getSelectedObjectSet<ZPunctum>(ZStackObject::TYPE_PUNCTUM);
  if (!punctaSet.empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    dlg.setWindowTitle("Transform Puncta");
    if (dlg.exec()) {
      double dx = dlg.getTranslateValue(SwcSkeletonTransformDialog::X);
      double dy = dlg.getTranslateValue(SwcSkeletonTransformDialog::Y);
      double dz = dlg.getTranslateValue(SwcSkeletonTransformDialog::Z);

      if (dlg.isTranslateFirst()) {
        for (std::set<ZPunctum*>::iterator iter = punctaSet.begin();
             iter != punctaSet.end(); ++iter) {
          (*iter)->setCenter(
                (*iter)->x() + dx, (*iter)->y() + dy, (*iter)->z() + dz);
        }
      }

      for (std::set<ZPunctum*>::iterator iter = punctaSet.begin();
           iter != punctaSet.end(); ++iter) {
        (*iter)->setCenter(
              (*iter)->x() * dlg.getScaleValue(SwcSkeletonTransformDialog::X),
              (*iter)->y() * dlg.getScaleValue(SwcSkeletonTransformDialog::Y),
              (*iter)->z() * dlg.getScaleValue(SwcSkeletonTransformDialog::Z));
      }

      if (!dlg.isTranslateFirst()) {
        for (std::set<ZPunctum*>::iterator iter = punctaSet.begin();
             iter != punctaSet.end(); ++iter) {
          (*iter)->setCenter(
                (*iter)->x() + dx, (*iter)->y() + dy, (*iter)->z() + dz);
        }
      }
    }
    m_doc->notifyPunctumModified();
  }
}

void Z3DWindow::changeSelectedPunctaColor()
{
  std::set<ZPunctum*> punctaSet =
      m_doc->getSelectedObjectSet<ZPunctum>(ZStackObject::TYPE_PUNCTUM);
  if (!punctaSet.empty()) {
    QColorDialog dlg;

    if (dlg.exec()) {
      for (std::set<ZPunctum*>::iterator iter = punctaSet.begin();
           iter != punctaSet.end(); ++iter) {
        ZPunctum *punctum = *iter;
        punctum->setColor(dlg.currentColor());
      }

      m_doc->notifyPunctumModified();
    }
  }
}

void Z3DWindow::transformAllPuncta()
{
  QList<ZPunctum*> punctaSet = m_doc->getPunctumList();
  if (!punctaSet.empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    dlg.setWindowTitle("Transform Puncta");
    if (dlg.exec()) {
      double dx = dlg.getTranslateValue(SwcSkeletonTransformDialog::X);
      double dy = dlg.getTranslateValue(SwcSkeletonTransformDialog::Y);
      double dz = dlg.getTranslateValue(SwcSkeletonTransformDialog::Z);

      if (dlg.isTranslateFirst()) {
        for (QList<ZPunctum*>::iterator iter = punctaSet.begin();
             iter != punctaSet.end(); ++iter) {
          ZPunctum *punctum = *iter;
          punctum->translate(dx, dy, dz);
        }
      }

      for (QList<ZPunctum*>::iterator iter = punctaSet.begin();
           iter != punctaSet.end(); ++iter) {
        (*iter)->setCenter(
              (*iter)->x() * dlg.getScaleValue(SwcSkeletonTransformDialog::X),
              (*iter)->y() * dlg.getScaleValue(SwcSkeletonTransformDialog::Y),
              (*iter)->z() * dlg.getScaleValue(SwcSkeletonTransformDialog::Z));
      }

      if (!dlg.isTranslateFirst()) {
        for (QList<ZPunctum*>::iterator iter = punctaSet.begin();
             iter != punctaSet.end(); ++iter) {
          ZPunctum *punctum = *iter;
          punctum->translate(dx, dy, dz);
        }
      }
      m_doc->notifyPunctumModified();
    }
  }
}

void Z3DWindow::convertPunctaToSwc()
{
  QList<ZPunctum*> punctaSet = m_doc->getPunctumList();
  if (!punctaSet.empty()) {
    ZSwcTree *tree = new ZSwcTree();
    for (QList<ZPunctum*>::iterator iter = punctaSet.begin();
         iter != punctaSet.end(); ++iter) {
      ZPoint pos((*iter)->x(), (*iter)->y(), (*iter)->z());
      Swc_Tree_Node *tn = SwcTreeNode::makePointer(pos, (*iter)->radius());
      tree->addRegularRoot(tn);
    }

    m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
    m_doc->addObject(tree, false);
    m_doc->removeSelectedPuncta();
    m_doc->endObjectModifiedMode();
    m_doc->notifyObjectModified();

//    m_doc->notifyPunctumModified();
//    m_doc->notifySwcModified();
  }
}

void Z3DWindow::transformSelectedSwc()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::TYPE_SWC);

  if (!treeSet.empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    if (SwcTreeNode::clipboard().size() >= 2) {
      Swc_Tree_Node node[2];
      for (size_t i = 0; i < 2; ++i) {
        SwcTreeNode::paste(node + i, i);
      }

      ZPoint offset = SwcTreeNode::center(node + 1) - SwcTreeNode::center(node);
      dlg.setTranslateValue(offset.x(), offset.y(), offset.z());
    } else {
      ZIntPoint offset = getDocument()->getStackOffset();
      dlg.setTranslateValue(-offset[0], -offset[1], -offset[2]);
    }
    if (dlg.exec()) {
      for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
           iter != treeSet.end(); ++iter) {
        if (dlg.isTranslateFirst()) {
          (*iter)->translate(dlg.getTranslateValue(SwcSkeletonTransformDialog::X),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Y),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Z));
        }

        (*iter)->scale(dlg.getScaleValue(SwcSkeletonTransformDialog::X),
                       dlg.getScaleValue(SwcSkeletonTransformDialog::Y),
                       dlg.getScaleValue(SwcSkeletonTransformDialog::Z));

        if (!dlg.isTranslateFirst()) {
          (*iter)->translate(dlg.getTranslateValue(SwcSkeletonTransformDialog::X),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Y),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Z));
        }
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::groupSelectedSwc()
{
  m_doc->executeGroupSwcCommand();
  /*
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  if (treeSet->size() > 1) {
    std::set<ZSwcTree*>::iterator iter = treeSet->begin();
    Swc_Tree_Node *root = (*iter)->root();

    for (++iter; iter != treeSet->end(); ++iter) {
      Swc_Tree_Node *subroot = (*iter)->firstRegularRoot();
      SwcTreeNode::setParent(subroot, root);
    }
    m_doc->removeEmptySwcTree();
    m_doc->notifySwcModified();
  }
  */
}

void Z3DWindow::showSeletedSwcNodeLength()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
  double length = SwcTreeNode::segmentLength(nodeSet);

  InformationDialog dlg;

  std::ostringstream textStream;

  textStream << "<p>Overall length of selected branches: " << length << "</p>";

  if (nodeSet.size() == 2) {
    std::set<Swc_Tree_Node*>::const_iterator iter = nodeSet.begin();
    Swc_Tree_Node *tn1 = *iter;
    ++iter;
    Swc_Tree_Node *tn2 = *iter;
    textStream << "<p>Straight line distance between the two selected nodes: "
               << SwcTreeNode::distance(tn1, tn2) << "</p>";
  }

  dlg.setText(textStream.str());
  dlg.exec();
}

void Z3DWindow::showSelectedSwcInfo()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::TYPE_SWC);

  InformationDialog dlg;

  std::ostringstream textStream;

  if (!treeSet.empty()) {
    for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
         iter != treeSet.end(); ++iter) {
      textStream << "<p><font color=\"blue\">" + (*iter)->getSource() + "</font></p>";
      textStream << "<p>Overall length: " << (*iter)->length() << "</p>";
      std::set<int> typeList = (*iter)->typeSet();
      if (typeList.size() > 1) {
        textStream << "<p>Typed branch length: ";
        textStream << "<ul>";
        for (std::set<int>::const_iterator typeIter = typeList.begin();
             typeIter != typeList.end(); ++typeIter) {
          textStream << "<li>Type " << *typeIter << ": " << (*iter)->length(*typeIter)
                    << "</li>";
        }
        textStream << "</ul>";
        textStream << "</p>";
      }
      textStream << "<p>Lateral-vertical ratio: "
                 << ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(*(*iter))
                 << "</p>";
    }
  }

  dlg.setText(textStream.str());
  dlg.exec();
}

void Z3DWindow::refreshTraceMask()
{
  getDocument()->refreshTraceMask();
}

void Z3DWindow::changeSelectedSwcColor()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::TYPE_SWC);
  if (!treeSet.empty()) {
    QColorDialog dlg;
    dlg.setCurrentColor((*treeSet.begin())->getColor());

    if (dlg.exec()) {
      for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
           iter != treeSet.end(); ++iter) {
        (*iter)->setColor(dlg.currentColor().red(),
                          dlg.currentColor().green(),
                          dlg.currentColor().blue());
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::changeSelectedSwcAlpha()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::TYPE_SWC);
  if (!treeSet.empty()) {
    ZAlphaDialog dlg;
    if (dlg.exec()) {
      int alpha = dlg.getAlpha();
      for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
           iter != treeSet.end(); ++iter) {
        (*iter)->setAlpha(alpha);
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::test()
{
  const NeutubeConfig &config = NeutubeConfig::getInstance();

  UNUSED_PARAMETER(&config);

  ZMovieMaker director;
  ZMovieScript script;


  /*
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();
  treeSet->clear();
  m_doc->requestRedrawSwc();

  QList<ZSwcTree*> *treeList =  m_doc->swcList();
  std::vector<ZMovieActor*> cast(treeList->size());
  std::vector<ZSwcMovieActor> swcCast(treeList->size());

  int index = 0;
  for (QList<ZSwcTree*>::iterator iter = treeList->begin();
       iter != treeList->end(); ++iter, ++index) {
    swcCast[index].setActor(*iter);
    swcCast[index].setMovingOffset((index + 1) * 10, 0, 0);
    cast[index] = &(swcCast[index]);
  }
*/
/*
  ZMovieScene scene(m_doc.get(), this);

  for (index = 0; index < 5; ++index) {
    std::ostringstream stream;
    stream << config.getPath(NeutubeConfig::DATA) + "/test/";
    stream << std::setw(3) << std::setfill('0') << index << ".tif";

    for (std::vector<ZMovieActor*>::iterator iter = cast.begin();
         iter != cast.end(); ++iter) {
      (*iter)->perform();
    }
    scene.saveToImage(stream.str(), 1024, 1024);
  }
*/
  /*
  for (QList<ZSwcTree*>::iterator iter = treeList->begin();
       iter != treeList->end(); ++iter, ++index) {

    scene.saveToImage(stream.str(), 1024, 1024);
    //takeScreenShot(stream.str().c_str(),
    //               1024, 1024, MonoView);
    (*iter)->setVisible(false);
    getInteractionHandler()->getTrackball()->rotate(glm::vec3(0,1,0), 0.5);
    getInteractionHandler()->getTrackball()->zoom(1.1);
  }
*/

  /*
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
       iter != treeSet->end(); ++iter) {
    (*iter)->setVisible(false);
  }
  treeSet->clear();
  */


}

void Z3DWindow::breakSelectedSwc()
{
  m_doc->executeBreakForestCommand();
  //Need to change to m_doc->executeBreakSwcCommand();
#if 0
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  if (!treeSet->empty()) {
    for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
         iter != treeSet->end(); ++iter) {
      Swc_Tree_Node *root = (*iter)->firstRegularRoot();
      if (root != NULL) {
        root = SwcTreeNode::nextSibling(root);
        while (root != NULL) {
          Swc_Tree_Node *sibling = SwcTreeNode::nextSibling(root);
          SwcTreeNode::detachParent(root);
          ZSwcTree *tree = new ZSwcTree;
          tree->setDataFromNode(root);
          /*
          std::ostringstream stream;
          stream << (*iter)->source().c_str() << "-" << m_doc->swcList()->size() + 1;
          tree->setSource(stream.str());
          */
          m_doc->addSwcTree(tree, false);
          root = sibling;
        }
      }
    }
    m_doc->notifySwcModified();
  }
#endif

}

void Z3DWindow::saveSelectedSwc()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::TYPE_SWC);

  QString fileName = "";

  if (!treeSet.empty()) {
    if (!(*treeSet.begin())->getSource().empty()) {
      if ((*treeSet.begin())->getSource()[0] != '#') {
        fileName = QString((*treeSet.begin())->getSource().c_str());
      }
    }
  }

  if (fileName.isEmpty()) {
    ZString stackSource = m_doc->stackSourcePath();
    if (!stackSource.empty()) {
      fileName = stackSource.changeExt("Edit.swc").c_str();
    }
  }

  if (fileName.isEmpty()) {
    fileName = "untitled.swc";
  }

  if (GET_APPLICATION_NAME == "Biocytin") {
    ZStackFrame *frame = m_doc->getParentFrame();
    if (frame != NULL) {
      fileName = m_doc->getParentFrame()->swcFilename;
    }
      //fileName =ZBiocytinFileNameParser::getSwcEditPath(fileName.toStdString()).c_str();
  }

  fileName =
      QFileDialog::getSaveFileName(this, tr("Save SWC"), fileName,
                                   tr("SWC File"), 0);

  if (!fileName.isEmpty()) {
    if (!fileName.endsWith(".swc", Qt::CaseInsensitive)) {
      fileName += ".swc";
    }

    if (treeSet.size() > 1) {
      ZSwcTree tree;

      for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
           iter != treeSet.end(); ++iter) {
        tree.merge((*iter)->cloneData(), true);
      }

      tree.resortId();
      tree.save(fileName.toStdString().c_str());
    } else {
      ZSwcTree *tree = *(treeSet.begin());
      tree->resortId();
      tree->save(fileName.toStdString().c_str());
      tree->setSource(fileName.toStdString().c_str());
      getDocument()->notifySwcModified();
    }
  }
}

void Z3DWindow::convertSelectedChainToSwc()
{
  std::set<ZLocsegChain*> chainSet =
      m_doc->getSelectedObjectSet<ZLocsegChain>(ZStackObject::TYPE_LOCSEG_CHAIN);

  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  for (std::set<ZLocsegChain*>::iterator iter = chainSet.begin();
       iter != chainSet.end(); ++iter) {
    Swc_Tree_Node *tn = TubeModel::createSwc((*iter)->data());
    if (tn != NULL) {
      ZSwcTree *tree = new ZSwcTree;
      tree->setDataFromNode(tn);
      m_doc->addObject(tree, false);
    }
  }
  //chainSet->clear();

  m_doc->executeRemoveTubeCommand();
  m_doc->endObjectModifiedMode();

  m_doc->notifyObjectModified();

//  m_doc->notifySwcModified();
//  m_doc->notifyChainModified();
}

bool Z3DWindow::hasSwc() const
{
  return m_doc->hasSwc();
}

bool Z3DWindow::hasSelectedSwc() const
{
  return m_doc->hasSelectedSwc();
}

bool Z3DWindow::hasSelectedSwcNode() const
{
  return !m_doc->hasSelectedSwcNode();
}

bool Z3DWindow::hasMultipleSelectedSwcNode() const
{
  return m_doc->hasMultipleSelectedSwcNode();
}

void Z3DWindow::notifyUser(const QString &message)
{
  statusBar()->showMessage(message);
}
#if 0
void Z3DWindow::addStrokeFrom3dPaint(ZStroke2d *stroke)
{
  bool success;


  QList<ZStroke2d*> strokeList;

  for (size_t i = 0; i < stroke->getPointNumber(); ++i) {
    double x = 0;
    double y = 0;
    stroke->getPoint(&x, &y, i);
    glm::vec3 fpos = m_volumeRaycaster->get3DPosition(
          x, y, m_canvas->width(), m_canvas->height(), success);

    /*
    ZLineSegment seg = m_volumeRaycaster->getScreenRay(
          x, y, m_canvas->width(), m_canvas->height(), success);
*/

    if (success) {
      //ZObject3d *obj = ZVoxelGraphics::createLineObject(seg);

#ifdef _DEBUG_2
      std::cout << fpos << std::endl;
#endif
      //for (size_t i = 0; i < obj->size(); ++i) {
        ZStroke2d *strokeData = new ZStroke2d;
        strokeData->setWidth(stroke->getWidth());
        strokeData->setLabel(stroke->getLabel());

        strokeData->append(fpos[0], fpos[1]);
        strokeData->setZ(iround(fpos[2]));
        //strokeData->append(obj->x(i), obj->y(i));
        //strokeData->setZ(obj->z(i));

        if (!strokeData->isEmpty()) {
          strokeList.append(strokeData);
        } else {
          delete strokeData;
        }
      //}

      //delete obj;
    }
  }

  m_doc->executeAddStrokeCommand(strokeList);
}
#endif

void Z3DWindow::addStrokeFrom3dPaint(ZStroke2d *stroke)
{
  //bool success = false;

  ZObject3d *baseObj = stroke->toObject3d();

  double x = 0.0;
  double y = 0.0;

  stroke->getPoint(&x, &y, stroke->getPointNumber() / 2);
  /*
  ZLineSegment seg = m_volumeRaycaster->getScreenRay(
        iround(x), iround(y), m_canvas->width(), m_canvas->height());
*/
  ZObject3d *obj = new ZObject3d;
  for (size_t i = 0; i < baseObj->size(); ++i) {
    ZLineSegment seg = m_volumeRaycaster->getScreenRay(
          baseObj->getX(i), baseObj->getY(i),
          m_canvas->width(), m_canvas->height());
    ZPoint slope = seg.getEndPoint() - seg.getStartPoint();
    //if (success) {
//      ZIntCuboid box = m_doc->stackRef()->getBoundBox();
//      ZIntCuboid box = m_volumeBoundBox;
#if 0
      ZCuboid rbox(box.getFirstCorner().getX(), box.getFirstCorner().getY(),
                   box.getFirstCorner().getZ(),
                   box.getLastCorner().getX(), box.getLastCorner().getY(),
                   box.getLastCorner().getZ());
#endif
      ZCuboid rbox(m_volumeBoundBox[0], m_volumeBoundBox[2], m_volumeBoundBox[4],
          m_volumeBoundBox[1], m_volumeBoundBox[3], m_volumeBoundBox[5]);

      ZLineSegment stackSeg;
      if (rbox.intersectLine(seg.getStartPoint(), slope, &stackSeg)) {
        ZObject3d *scanLine = ZVoxelGraphics::createLineObject(stackSeg);
        obj->append(*scanLine);
#ifdef _DEBUG_2
        scanLine->print();
        obj->print();
        break;
#endif
        delete scanLine;
      }
    //}
  }
  obj->setLabel(stroke->getLabel());
  ZLabelColorTable colorTable;
  obj->setColor(colorTable.getColor(obj->getLabel()));

  delete baseObj;

  if (!obj->isEmpty()) {
    obj->setRole(ZStackObjectRole::ROLE_SEED);
    m_doc->executeAddObjectCommand(obj);
  } else {
    delete obj;
  }
}

void Z3DWindow::addPolyplaneFrom3dPaint(ZStroke2d *stroke)
{
  //bool success = false;
  if (m_doc->hasStack()) {
    std::vector<ZIntPoint> polyline1;
    std::vector<ZIntPoint> polyline2;
#if 0
    ZIntCuboid box = m_doc->stackRef()->getBoundBox();
    ZCuboid rbox(box.getFirstCorner().getX(), box.getFirstCorner().getY(),
                 box.getFirstCorner().getZ(),
                 box.getLastCorner().getX(), box.getLastCorner().getY(),
                 box.getLastCorner().getZ());
#endif
    ZCuboid rbox(m_volumeBoundBox[0], m_volumeBoundBox[2], m_volumeBoundBox[4],
        m_volumeBoundBox[1], m_volumeBoundBox[3], m_volumeBoundBox[5]);
    for (size_t i = 0; i < stroke->getPointNumber(); ++i) {
      double x = 0.0;
      double y = 0.0;
      stroke->getPoint(&x, &y, i);
      ZLineSegment seg = m_volumeRaycaster->getScreenRay(
            iround(x), iround(y), m_canvas->width(), m_canvas->height());
      //if (success) {
      ZPoint slope = seg.getEndPoint() - seg.getStartPoint();
      ZLineSegment stackSeg;
      if (rbox.intersectLine(seg.getStartPoint(), slope, &stackSeg)) {
        ZPoint slope2 = stackSeg.getEndPoint() - stackSeg.getStartPoint();
        if (slope.dot(slope2) < 0.0) {
          stackSeg.invert();
        }
        polyline1.push_back(ZIntPoint(stackSeg.getStartPoint().toIntPoint()));
        polyline2.push_back(ZIntPoint(stackSeg.getEndPoint().toIntPoint()));
      }
      //}
    }

    ZObject3d *obj = ZVoxelGraphics::createPolyPlaneObject(polyline1, polyline2);

    if (obj != NULL) {
      ZObject3d *processedObj = NULL;

      const ZStack *stack = NULL;
      int xIntv = 0;
      int yIntv = 0;
      int zIntv = 0;

      if (getDocument()->hasSparseStack()) {
        stack = getDocument()->getSparseStack()->getStack();
        ZIntPoint dsIntv = getDocument()->getSparseStack()->getDownsampleInterval();
        xIntv = dsIntv.getX();
        yIntv = dsIntv.getY();
        zIntv = dsIntv.getZ();
      } else {
        stack = getDocument()->getStack();
      }

      processedObj = new ZObject3d;
      for (size_t i = 0; i < obj->size(); ++i) {
        int x = obj->getX(i) / (xIntv + 1) - stack->getOffset().getX();
        int y = obj->getY(i) / (yIntv + 1) - stack->getOffset().getY();
        int z = obj->getZ(i) / (zIntv + 1) - stack->getOffset().getZ();
        int v = 0;
        for (int dz = -1; dz <= 1; ++dz) {
          for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
              v = stack->getIntValueLocal(x + dx, y + dy, z + dz);
              if (v > 0) {
                break;
              }
            }
          }
        }
        if (v > 0) {
          processedObj->append(obj->getX(i), obj->getY(i), obj->getZ(i));
        }
      }
      delete obj;
      obj = processedObj;
    }

    if (obj != NULL) {
#ifdef _DEBUG_2
      ZStack *stack = obj->toStackObject();
      stack->save(GET_TEST_DATA_DIR + "/test.tif");
      delete stack;
#endif

      if (!obj->isEmpty()) {
#ifdef _DEBUG_2
        ZSwcTree *tree = ZSwcGenerator::createSwc(*obj, 1.0, 3);
        tree->save(GET_TEST_DATA_DIR + "/test.swc");
#endif

        obj->setLabel(stroke->getLabel());
        ZLabelColorTable colorTable;
        obj->setColor(colorTable.getColor(obj->getLabel()));

        obj->setRole(ZStackObjectRole::ROLE_SEED |
                     ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
        m_doc->executeAddObjectCommand(obj);
        //m_doc->notifyVolumeModified();
      } else {
        delete obj;
      }
    }
  }

  delete stroke;
}

void Z3DWindow::markSwcSoma()
{
  ZMarkSwcSomaDialog dlg;
  if (dlg.exec()) {
    QList<ZSwcTree*> trees = m_doc->getSwcList();
    for (int i=0; i<trees.size(); ++i) {
      trees.at(i)->markSoma(dlg.getRadiusThre(), dlg.getSomaType(), dlg.getOtherType());
    }
    m_doc->notifySwcModified();
  }
}

void Z3DWindow::setBackgroundColor(
    const glm::vec3 &color1, const glm::vec3 &color2)
{
  getCompositor()->setBackgroundFirstColor(color1);
  getCompositor()->setBackgroundSecondColor(color2);
}

Z3DWindow* Z3DWindow::Make(
    ZStackDoc* doc, QWidget *parent, Z3DWindow::EInitMode mode)
{
  return Make(ZSharedPointer<ZStackDoc>(doc), parent, mode);
}

Z3DWindow* Z3DWindow::Open(
    ZStackDoc* doc, QWidget *parent, Z3DWindow::EInitMode mode)
{
  return Open(ZSharedPointer<ZStackDoc>(doc), parent, mode);
}

Z3DWindow* Z3DWindow::Make(
    ZSharedPointer<ZStackDoc> doc, QWidget *parent, Z3DWindow::EInitMode mode)
{
  ZWindowFactory factory;
  factory.setParentWidget(parent);
  return factory.make3DWindow(doc, mode);
}

Z3DWindow* Z3DWindow::Open(
    ZSharedPointer<ZStackDoc> doc, QWidget *parent, Z3DWindow::EInitMode mode)
{
  Z3DWindow *window = Make(doc, parent, mode);
  window->show();
  window->raise();

  return window;
}

Z3DGeometryFilter* Z3DWindow::getFilter(ERendererLayer layer) const
{
  switch (layer) {
  case LAYER_SWC:
    return getSwcFilter();
  case LAYER_GRAPH:
    return getGraphFilter();
  case LAYER_PUNCTA:
    return getPunctaFilter();
  case LAYER_TODO:
    return getTodoFilter();
  case LAYER_SURFACE:
    return getSurfaceFilter();
  case LAYER_VOLUME:
    break;
  }

  return NULL;
}

Z3DRendererBase* Z3DWindow::getRendererBase(ERendererLayer layer)
{
  switch (layer) {
  case LAYER_SWC:
  case LAYER_GRAPH:
  case LAYER_PUNCTA:
  case LAYER_TODO:
  case LAYER_SURFACE:
  {
    Z3DGeometryFilter *filter = getFilter(layer);
    if (filter != NULL) {
      return filter->getRendererBase();
    }
  }
    break;
  case LAYER_VOLUME:
    return getVolumeRaycasterRenderer()->getRendererBase();
  }

  return NULL;
}

void Z3DWindow::setZScale(ERendererLayer layer, double scale)
{
  if (getRendererBase(layer) != NULL) {
    getRendererBase(layer)->setZScale(scale);
  }
}

void Z3DWindow::setScale(ERendererLayer layer, double sx, double sy, double sz)
{
  Z3DRendererBase *base = getRendererBase(layer);
  if (base != NULL) {
    base->setXScale(sx);
    base->setYScale(sy);
    base->setZScale(sz);
  }
}

void Z3DWindow::setOpacity(ERendererLayer layer, double opacity)
{
  getRendererBase(layer)->setOpacity(opacity);
}

void Z3DWindow::setVisible(ERendererLayer layer, bool visible)
{
  switch (layer) {
  case LAYER_GRAPH:
    getGraphFilter()->setVisible(visible);
    break;
  case LAYER_SWC:
    getSwcFilter()->setVisible(visible);
    break;
  case LAYER_PUNCTA:
    getPunctaFilter()->setVisible(visible);
    break;
  case LAYER_TODO:
    if (getTodoFilter() != NULL) {
      getTodoFilter()->setVisible(visible);
    }
    break;
  case LAYER_SURFACE:
    if (getSurfaceFilter() != NULL) {
      getSurfaceFilter()->setVisible(visible);
    }
    break;
  case LAYER_VOLUME:
    break;
  }
}

bool Z3DWindow::isVisible(ERendererLayer layer) const
{
  switch (layer) {
  case LAYER_GRAPH:
    return getGraphFilter()->isVisible();
    break;
  case LAYER_SWC:
    return getSwcFilter()->isVisible();
    break;
  case LAYER_PUNCTA:
    return getPunctaFilter()->isVisible();
    break;
  case LAYER_TODO:
    if (getTodoFilter() != NULL) {
      return getTodoFilter()->isVisible();
    }
    break;
  case LAYER_SURFACE:
    if (getSurfaceFilter() != NULL) {
      return getSurfaceFilter()->isVisible();
    }
  case LAYER_VOLUME:
    break;
  }

  return true;
}

void Z3DWindow::setZScale(double scale)
{
  setZScale(LAYER_GRAPH, scale);
  setZScale(LAYER_SWC, scale);
  setZScale(LAYER_PUNCTA, scale);
  setZScale(LAYER_VOLUME, scale);
  setZScale(LAYER_TODO, scale);
  setZScale(LAYER_SURFACE, scale);
}

void Z3DWindow::setScale(double sx, double sy, double sz)
{
  setScale(LAYER_GRAPH, sx, sy, sz);
  setScale(LAYER_SWC, sx, sy, sz);
  setScale(LAYER_PUNCTA, sx, sy, sz);
  setScale(LAYER_VOLUME, sx, sy, sz);
  setScale(LAYER_TODO, sx, sy, sz);
  setScale(LAYER_SURFACE, sx, sy, sz);
}

void Z3DWindow::cropSwcInRoi()
{
  if (m_doc->getTag() == NeuTube::Document::FLYEM_BODY_3D_COARSE) {
//    m_doc->executeDeleteSwcNodeCommand();
    if (ZDialogFactory::Ask("Cropping", "Do you want to crop the body?", this)) {
      emit croppingSwcInRoi();
    }
  } else if (m_doc->getTag() == NeuTube::Document::FLYEM_BODY_3D ||
             m_doc->getTag() == NeuTube::Document::FLYEM_SKELETON) {
    QMessageBox::warning(
          this, "Action Failed", "Cropping only works in coarse body view.");
  } else {
    selectSwcTreeNodeInRoi(false);
    m_doc->executeDeleteSwcNodeCommand();
  }
}

void Z3DWindow::selectSwcTreeNodeInRoi(bool appending)
{
  if (hasRectRoi()) {
    QList<ZSwcTree*> treeList = m_doc->getSwcList();

    ZRect2d rect = getRectRoi();

    for (QList<ZSwcTree*>::const_iterator iter = treeList.begin();
         iter != treeList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      tree->recordSelection();
      if (!appending) {
        tree->deselectAllNode();
      }

      ZSwcTree::DepthFirstIterator nodeIter(tree);
      while (nodeIter.hasNext()) {
        Swc_Tree_Node *tn = nodeIter.next();
        if (SwcTreeNode::isRegular(tn)) {
          const QPointF &pt = getScreenProjection(
                SwcTreeNode::x(tn), SwcTreeNode::y(tn), SwcTreeNode::z(tn),
                LAYER_SWC);
          if (rect.contains(pt.x(), pt.y())) {
            tree->selectNode(tn, true);
          }
        }
      }
    }

    for (QList<ZSwcTree*>::const_iterator iter = treeList.begin();
         iter != treeList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      tree->processSelection();
    }

    m_doc->notifySwcTreeNodeSelectionChanged();
    removeRectRoi();
  }
}

void Z3DWindow::removeRectRoi()
{
  getCanvas()->getInteractionEngine()->removeRectDecoration();
}

QDockWidget* Z3DWindow::getSettingsDockWidget()
{
    return m_settingsDockWidget;
}

QDockWidget* Z3DWindow::getObjectsDockWidget()
{
    return m_objectsDockWidget;
}

ZROIWidget* Z3DWindow::getROIsDockWidget()
{
    return m_roiDockWidget;
}

void Z3DWindow::setButtonStatus(int index, bool v)
{
    m_buttonStatus[index] = v;

}

bool Z3DWindow::getButtonStatus(int index)
{
    return m_buttonStatus[index];
}
