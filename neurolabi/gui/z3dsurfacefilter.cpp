#include "z3dsurfacefilter.h"
#include "z3dcuberenderer.h"
#include "neutubeconfig.h"

using namespace std;

//
Z3DCube::Z3DCube()
{

}

Z3DCube::~Z3DCube()
{

}

//
Z3DSurfaceFilter::Z3DSurfaceFilter() :
  Z3DGeometryFilter(),
    m_showCube("Visible", true),
    m_cubeRenderer(NULL)
  , m_dataIsInvalid(false)
  , m_widgetsGroup(NULL)
{
    addParameter(m_showCube);

    const NeutubeConfig::Z3DWindowConfig::GraphTabConfig &config =
            NeutubeConfig::getInstance().getZ3DWindowConfig().getGraphTabConfig();
    m_showCube.set(config.isVisible());
    //  m_rendererBase->setRenderMethod("Old openGL");
    //  adjustWidgets();
}

Z3DSurfaceFilter::~Z3DSurfaceFilter()
{

}

void Z3DSurfaceFilter::initialize()
{
    Z3DGeometryFilter::initialize();

    m_cubeRenderer = new Z3DCubeRenderer();
    m_rendererBase->addRenderer(m_cubeRenderer);

    std::vector<ZParameter*> paras = m_rendererBase->getParameters();
    for (size_t i=0; i<paras.size(); i++) {
        //connect(paras[i], SIGNAL(valueChanged()), this, SLOT(invalidateResult()));
        addParameter(paras[i]);
    }
}

void Z3DSurfaceFilter::deinitialize()
{
    std::vector<ZParameter*> paras = m_rendererBase->getParameters();
    for (size_t i=0; i<paras.size(); i++) {
        removeParameter(paras[i]);
    }
    Z3DGeometryFilter::deinitialize();
}

void Z3DSurfaceFilter::setVisible(bool v)
{
    m_showCube.set(v);
}

void Z3DSurfaceFilter::render(Z3DEye eye)
{
    if(m_cubeRenderer->isEmpty())
    {
        return;
    }

    if (!m_showCube.get())
        return;

    m_rendererBase->activateRenderer(m_cubeRenderer);

    m_rendererBase->render(eye);
}

void Z3DSurfaceFilter::process(Z3DEye)
{
    if (m_dataIsInvalid) {
        prepareData();
    }
}

void Z3DSurfaceFilter::prepareData()
{
    if (!m_dataIsInvalid)
        return;

//    m_cubeRenderer->clearCube();
    for (size_t i = 0; i < m_cubeArray.size(); ++i) {
      const Z3DCube &cube = m_cubeArray[i];
      m_cubeRenderer->addCube(
            cube.length, cube.length, cube.length, cube.x, cube.y, cube.z, cube.color);
    }


    m_dataIsInvalid = false;
}

void Z3DSurfaceFilter::addData(const Z3DCube &cube)
{
  m_cubeArray.push_back(cube);

//    m_cubeRenderer->addCube(cube.length, cube.length, cube.length, cube.x, cube.y, cube.z, cube.color);

    m_dataIsInvalid = true;
    invalidateResult();
}

ZWidgetsGroup *Z3DSurfaceFilter::getWidgetsGroup()
{
    if (!m_widgetsGroup) {
        m_widgetsGroup = new ZWidgetsGroup("Surface", NULL, 1);
        new ZWidgetsGroup(&m_showCube, m_widgetsGroup, 1);

        new ZWidgetsGroup(&m_stayOnTop, m_widgetsGroup, 1);
        std::vector<ZParameter*> paras = m_rendererBase->getParameters();
        for (size_t i=0; i<paras.size(); i++) {
            ZParameter *para = paras[i];
            if (para->getName() == "Z Scale")
                new ZWidgetsGroup(para, m_widgetsGroup, 2);
            else if (para->getName() == "Size Scale")
                new ZWidgetsGroup(para, m_widgetsGroup, 3);
            else if (para->getName() == "Rendering Method")
                new ZWidgetsGroup(para, m_widgetsGroup, 4);
            else if (para->getName() == "Opacity")
                new ZWidgetsGroup(para, m_widgetsGroup, 5);
            else
                new ZWidgetsGroup(para, m_widgetsGroup, 7);
        }
        m_widgetsGroup->setBasicAdvancedCutoff(5);
    }
    return m_widgetsGroup;
}

bool Z3DSurfaceFilter::isReady(Z3DEye eye) const
{
  qDebug() << Z3DGeometryFilter::isReady(eye);
  qDebug() << m_showCube.get();
  qDebug() << m_cubeRenderer->isEmpty();
    return Z3DGeometryFilter::isReady(eye) && m_showCube.get() &&
            !m_cubeRenderer->isEmpty();
}
