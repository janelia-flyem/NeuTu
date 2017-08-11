#include "z3dsurfacefilter.h"
#include "neutubeconfig.h"

using namespace std;


Z3DSurfaceFilter::Z3DSurfaceFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
{

}

//
//Z3DSurfaceFilter::Z3DSurfaceFilter() :
//    Z3DGeometryFilter(),
////    m_showCube("Visible", true),
//    m_cubeRenderer(NULL),
//    m_dataIsInvalid(false),
//    m_widgetsGroup(NULL)
//{
////    addParameter(m_showCube);

//    const NeutubeConfig::Z3DWindowConfig::GraphTabConfig &config =
//            NeutubeConfig::getInstance().getZ3DWindowConfig().getGraphTabConfig();
////    m_showCube.set(config.isVisible());
//    setVisible(config.isVisible());
//    //  m_rendererBase->setRenderMethod("Old openGL");
//    //  adjustWidgets();

//    setFilterName(QString("surfacefilter"));
//    //setNeedBlending(true);
//    setOpacity(0.75);

//    //
//    m_initialized = false;

//    //
//    m_sourceSet = false;

//    //
//    setStayOnTop(false);

//    //
//    connect(m_rendererBase, SIGNAL(opacityChanged(double)), this, SLOT(indicateOpacityChanged(double)));
//}

//void Z3DSurfaceFilter::initialize()
//{
//    Z3DGeometryFilter::initialize();

////    m_cubeRenderer = new Z3DCubeRenderer();
////    m_rendererBase->addRenderer(m_cubeRenderer);

////    std::vector<ZParameter*> paras = m_rendererBase->getParameters();
////    for (size_t i=0; i<paras.size(); i++) {
////        //connect(paras[i], SIGNAL(valueChanged()), this, SLOT(invalidateResult()));
////        addParameter(paras[i]);
////    }

//    //
//    if(m_sourceSet)
//    {
//        initRenderers(m_nSources);
//    }

//    //
//    m_initialized = true;
//}

void Z3DSurfaceFilter::initRenderers(size_t n)
{
//    m_nSources = n;

//    //
//    for(size_t i=0; i<n; i++)
//    {
//        Z3DCubeRenderer *cubeRenderer = new Z3DCubeRenderer();
//        m_cubeRenderers.push_back(cubeRenderer); //

//        m_rendererBase->addRenderer(m_cubeRenderers.back());
//    }

//    std::vector<ZParameter*> paras = m_rendererBase->getParameters();
//    for (size_t i=0; i<paras.size(); i++) {
//        //connect(paras[i], SIGNAL(valueChanged()), this, SLOT(invalidateResult()));
//        addParameter(paras[i]);
//    }

//    //
//    m_sourceSet = false;

//    //
//    m_initialized = true;
}

//void Z3DSurfaceFilter::render(Z3DEye eye)
//{
////    if(m_cubeRenderer->isEmpty())
////    {
////        return;
////    }

//    if(m_sourceList.size()<=0)
//        return;

//    if (!isVisible())
//        return;

//    //m_rendererBase->activateRenderer(m_cubeRenderer);

//    int count = 0;
//    for(size_t i=0; i<m_renderCubes.size(); ++i)
//    {
//        if(m_renderCubes[i])
//        {
////            qDebug()<<"### active cuberenderer "<<i;

//            if(count == 0)
//            {
//                m_rendererBase->activateRenderer(m_cubeRenderers[i]);
//            }
//            else
//            {
//                m_rendererBase->activateRenderer(m_cubeRenderers[i], Z3DRendererBase::None);
//            }

//            count++;
//        }
//    }

//    m_rendererBase->render(eye);
//}

void Z3DSurfaceFilter::process(Z3DEye)
{
    if (m_dataIsInvalid) {
        prepareData();
    }
}

void Z3DSurfaceFilter::invalidateRenderer(const string &source)
{
//  for(size_t i=0; i<m_cubeArrayList.size(); ++i) {
//    if (m_cubeArrayList[i].getSource() == source) {
//      m_cubeRenderers[i]->clearData();
//    }
//  }
//  for (size_t i = 0; i < m_cubeArrayList.size(); ++i) {
//    if (m_cubeArrayList[i].getSource() == source) {
//      m_cubeArrayList[i].clear();
//    }
//  }
}

void Z3DSurfaceFilter::prepareData()
{
//    if (!m_dataIsInvalid)
//        return;

//    if(!m_initialized)
//    {
//        initialize();
//    }

//    // reset renderCubes
//    for(size_t j=0; j< m_cubeArrayList.size(); ++j)
//    {
//        m_renderCubes[j] = false;
//        //m_cubeRenderers[j]->clearData();
//    }

//    // has ROI(s) for rendering
//    if(m_sourceList.size()>0)
//    {
//        // assign ROI to cubeRenderer
//        for(size_t i=0; i<m_sourceList.size(); ++i)
//        {
//            for(size_t j=0; j<m_cubeArrayList.size(); ++j)
//            {
//                if(m_cubeArrayList[j].getSource() == m_sourceList[i])
//                {
//                    if(m_cubeRenderers[j]->isEmpty())
//                    {
//                        m_cubeRenderers[j]->addCubes(m_cubeArrayList.at(j));
//                    }
//                    else
//                    {
//                        qreal r,g,b,a;
//                        m_cubeArrayList.at(j).getColor().getRgbF(&r, &g, &b, &a); // QColor -> glm::vec4

//                        m_cubeRenderers[j]->setColor(glm::vec4(r,g,b,a));
//                    }

//                    m_renderCubes[j] = true;
//                }
//            }
//        }

//        m_dataIsInvalid = false;
//    }
//    else
//    {
//        // test code here
//    }
}

//void Z3DSurfaceFilter::addData(const Z3DCube &cube)
//{
//    m_cubeArray.push_back(cube);

//    //
//    updateSurfaceVisibleState();
//}

//void Z3DSurfaceFilter::addData(ZCubeArray *cubes)
//{
//    std::string source = cubes->getSource();
//    m_sourceList.push_back(source);

//    bool sourceAdded = false;
//    for(size_t i=0; i<m_cubeArrayList.size(); i++)
//    {
//        if(m_cubeArrayList[i].getSource() == source.c_str())
//        {
//            // update color
//          if (!m_cubeArrayList[i].isEmpty()) {
//            m_cubeArrayList[i].setColor(cubes->getColor());
//          } else {
//            m_cubeArrayList[i] = *cubes;
//          }
//          //
//          sourceAdded = true;
//          continue;
//        }
//    }

//    if(!sourceAdded)
//    {
//        bool renderCube = true;
//        m_renderCubes.push_back(renderCube);

//        m_cubeArrayList.push_back(*cubes); // add source
//    }

//    m_sourceSet = true;

//    //
//    updateSurfaceVisibleState();
//}

//void Z3DSurfaceFilter::clearData()
//{
//    m_cubeArray.clear();
//    m_cubeArrayList.clear();
//}

//void Z3DSurfaceFilter::clearSources()
//{
//    m_sourceList.clear();
//}

//vector<double> Z3DSurfaceFilter::boundBox()
//{
//    vector<double> result(6, 0);

//    for(size_t j=0; j< m_cubeArrayList.size(); ++j)
//    {
//        std::vector<Z3DCube> cubes = m_cubeArrayList[j].getCubeArray();

//        for (size_t i = 0; i < cubes.size(); ++i)
//        {
//            const Z3DCube &cube = cubes[i];

//#ifdef _DEBUG_2
//            std::cout << "Cube:" << cube.x << " " << cube.y << " " << cube.z
//                      << std::endl;
//#endif

//            if (cube.initByNodes) {
//              for (std::vector<glm::vec3>::const_iterator iter = cube.nodes.begin();
//                   iter != cube.nodes.end(); ++iter) {
//                const glm::vec3 &node = *iter;
//                result[0] = min(result[0], double(node[0]));
//                result[1] = max(result[1], double(node[0]));
//                result[2] = min(result[2], double(node[1]));
//                result[3] = max(result[3], double(node[1]));
//                result[4] = min(result[4], double(node[2]));
//                result[5] = max(result[5], double(node[2]));
//              }
//            } else {
//              float radius = cube.length / 2.0;

//              result[0] = min(result[0], cube.x - radius);
//              result[1] = max(result[1], cube.x + radius);
//              result[2] = min(result[2], cube.y - radius);
//              result[3] = max(result[3], cube.y + radius);
//              result[4] = min(result[4], cube.z - radius);
//              result[5] = max(result[5], cube.z + radius);
//            }
//        }
//    }

//    return result;
//}

void Z3DSurfaceFilter::updateNotTransformedBoundBoxImpl()
{

}

bool Z3DSurfaceFilter::isReady(Z3DEye eye) const
{
  //return Z3DGeometryFilter::isReady(eye) && isVisible() && !m_sourceList.empty();
  return false;
}

std::shared_ptr<ZWidgetsGroup> Z3DSurfaceFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("Surface", 1);
    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_stayOnTop, 1);

    const std::vector<ZParameter*>& paras = m_rendererBase.parameters();
    for (auto para : paras) {
      if (para->name() == "Coord Transform")
        m_widgetsGroup->addChild(*para, 2);
      else if (para->name() == "Size Scale")
        m_widgetsGroup->addChild(*para, 3);
      else if (para->name() == "Rendering Method")
        m_widgetsGroup->addChild(*para, 4);
      else if (para->name() == "Opacity")
        m_widgetsGroup->addChild(*para, 5);
      else
        m_widgetsGroup->addChild(*para, 7);
    }

    m_widgetsGroup->addChild(m_xCut, 5);
    m_widgetsGroup->addChild(m_yCut, 5);
    m_widgetsGroup->addChild(m_zCut, 5);

    //m_widgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_widgetsGroup;
}

void Z3DSurfaceFilter::renderOpaque(Z3DEye eye)
{

}

void Z3DSurfaceFilter::renderTransparent(Z3DEye eye)
{

}

void Z3DSurfaceFilter::updateSurfaceVisibleState()
{
    m_dataIsInvalid = true;
    invalidateResult();
}

void Z3DSurfaceFilter::indicateOpacityChanged(double v)
{
    emit opacityValueChanged(v);
}
