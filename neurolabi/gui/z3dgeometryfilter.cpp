#include "z3dgeometryfilter.h"

#include "zjsonobject.h"
#include "z3dfiltersetting.h"
#include "zjsonparser.h"
#include "z3drendertarget.h"
#include "z3dpickingmanager.h"

Z3DGeometryFilter::Z3DGeometryFilter(Z3DGlobalParameters& globalPara, QObject* parent)
  : Z3DBoundedFilter(globalPara, parent)
  , m_outPort("GeometryFilter", this)
  , m_stayOnTop("Stay On Top", false)
  , m_pickingObjectsRegistered(false)
{
  addPort(m_outPort);
  addParameter(m_stayOnTop);
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

const void* Z3DGeometryFilter::pickObject(int x, int y)
{
  if (pickingEnabled()) {
    return pickingManager().objectAtWidgetPos(glm::ivec2(x, y));
  }

  return NULL;
}
