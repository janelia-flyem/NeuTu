#include "z3dgeometryfilter.h"

#include "zjsonobject.h"
#include "z3dfiltersetting.h"
#include "zjsonparser.h"
#include "z3drendertarget.h"

Z3DGeometryFilter::Z3DGeometryFilter(Z3DGlobalParameters& globalPara, QObject* parent)
  : Z3DBoundedFilter(globalPara, parent)
  , m_outPort("GeometryFilter", this)
  , m_visible("Visible", true)
  , m_stayOnTop("Stay On Top", false)
  , m_pickingManager(nullptr)
  , m_pickingObjectsRegistered(false)
{
  addPort(m_outPort);
  addParameter(m_stayOnTop);
}

void Z3DGeometryFilter::setVisible(bool v)
{
  m_visible.set(v);
}

bool Z3DGeometryFilter::isVisible() const
{
  return m_visible.get();
}

glm::uvec3 Z3DGeometryFilter::getPickingTexSize() const
{
  return getPickingManager()->renderTarget().attachment(
        GL_COLOR_ATTACHMENT0)->dimension();
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
  obj.setEntry(Z3DFilterSetting::SIZE_SCALE_KEY, sizeScale());

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

void Z3DGeometryFilter::get3DRayUnderScreenPoint(
    glm::vec3 &v1, glm::vec3 &v2, int x, int y, int width, int height)
{
  glm::mat4 projection = getCamera().projectionMatrix(Z3DEye::Mono);
  glm::mat4 modelview = getCamera().viewMatrix(Z3DEye::Mono);

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
  glm::dmat4 projection = glm::dmat4(getCamera().projectionMatrix(Z3DEye::Mono));
  glm::dmat4 modelview = glm::dmat4(getCamera().viewMatrix(Z3DEye::Mono));

  glm::ivec4 viewport;
  viewport[0] = 0;
  viewport[1] = 0;
  viewport[2] = width;
  viewport[3] = height;

  v1 = glm::unProject(glm::dvec3(x, height-y, 0.f), modelview, projection, viewport);
  v2 = glm::unProject(glm::dvec3(x, height-y, 1.f), modelview, projection, viewport);
  v2 = glm::normalize(v2-v1) + v1;
}
