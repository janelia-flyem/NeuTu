#include "z3dgeometryfilter.h"
#include "zjsonobject.h"
#include "z3dfiltersetting.h"
#include "zjsonparser.h"
#include "z3drendertarget.h"

Z3DGeometryFilter::Z3DGeometryFilter(QObject *parent)
  : Z3DProcessor(parent)
  , m_outPort("GeometryFilter")
  , m_visible("Visible", true)
  , m_stayOnTop("Always in Front", true)
  , m_pickingManager(NULL)
  , m_pickingObjectsRegistered(false)
  , m_needBlending(false)
  , m_needOIT(false)
{
  addPort(m_outPort);
  m_rendererBase = new Z3DRendererBase();
  addParameter(m_stayOnTop);
  addParameter(m_visible);

  setFilterName("geometryfilter");
}

Z3DGeometryFilter::~Z3DGeometryFilter()
{
  delete m_rendererBase;
}

void Z3DGeometryFilter::setVisible(bool v)
{
  m_visible.set(v);
}

bool Z3DGeometryFilter::isVisible() const
{
  return m_visible.get();
}

void Z3DGeometryFilter::setPickingManager(Z3DPickingManager *pm)
{
  if (m_pickingManager != pm) {
    deregisterPickingObjects(m_pickingManager);
    m_pickingManager = pm;
    registerPickingObjects(m_pickingManager);
  }
}

void Z3DGeometryFilter::get3DRayUnderScreenPoint(
    glm::vec3 &v1, glm::vec3 &v2, int x, int y, int width, int height)
{
  glm::mat4 projection = getCamera().getProjectionMatrix(CenterEye);
  glm::mat4 modelview = getCamera().getViewMatrix(CenterEye);

  glm::ivec4 viewport;
  viewport[0] = 0;
  viewport[1] = 0;
  viewport[2] = width;
  viewport[3] = height;

  v1 = glm::unProject(glm::vec3(x, height-y, 0.f), modelview, projection, viewport);
  v2 = glm::unProject(glm::vec3(x, height-y, 1.f), modelview, projection, viewport);
  v2 = glm::normalize(v2-v1) + v1;
}

void Z3DGeometryFilter::get3DRayUnderScreenPoint(
    glm::dvec3 &v1, glm::dvec3 &v2, int x, int y, int width, int height)
{
  glm::dmat4 projection = glm::dmat4(getCamera().getProjectionMatrix(CenterEye));
  glm::dmat4 modelview = glm::dmat4(getCamera().getViewMatrix(CenterEye));

  glm::ivec4 viewport;
  viewport[0] = 0;
  viewport[1] = 0;
  viewport[2] = width;
  viewport[3] = height;

  v1 = glm::unProject(glm::dvec3(x, height-y, 0.f), modelview, projection, viewport);
  v2 = glm::unProject(glm::dvec3(x, height-y, 1.f), modelview, projection, viewport);
  v2 = glm::normalize(v2-v1) + v1;
}

glm::ivec3 Z3DGeometryFilter::getPickingTexSize() const
{
  return getPickingManager()->getRenderTarget()->getAttachment(
        GL_COLOR_ATTACHMENT0)->getDimensions();
}

void Z3DGeometryFilter::updatePickingTexSize()
{
  m_pickingTexSize = getPickingTexSize();
}

ZJsonObject Z3DGeometryFilter::getConfigJson() const
{
  ZJsonObject obj;

  obj.setEntry(Z3DFilterSetting::FRONT_KEY, isStayOnTop());
  obj.setEntry(Z3DFilterSetting::VISIBLE_KEY, isVisible());
  obj.setEntry(Z3DFilterSetting::SIZE_SCALE_KEY, getSizeScale());

  return obj;
}

void Z3DGeometryFilter::configure(const ZJsonObject &obj)
{
  if (obj.hasKey(Z3DFilterSetting::FRONT_KEY)) {
    setStayOnTop(ZJsonParser::booleanValue(obj[Z3DFilterSetting::FRONT_KEY]));
  }

  if (obj.hasKey(Z3DFilterSetting::SIZE_SCALE_KEY)) {
    setSizeScale(ZJsonParser::numberValue(obj[Z3DFilterSetting::SIZE_SCALE_KEY]));
  }

  if (obj.hasKey(Z3DFilterSetting::VISIBLE_KEY)) {
    setVisible(ZJsonParser::booleanValue(obj[Z3DFilterSetting::VISIBLE_KEY]));
  }

  if (obj.hasKey(Z3DFilterSetting::OPACITY_KEY)) {
    setOpacity(ZJsonParser::numberValue(obj[Z3DFilterSetting::OPACITY_KEY]));
  }
}
