#include "z3dfiltersetting.h"

#include "zjsonobject.h"
#include "zjsonparser.h"

const char* Z3DFilterSetting::FRONT_KEY = "front";
const char* Z3DFilterSetting::COLOR_MODE_KEY = "color";
const char* Z3DFilterSetting::SHAPE_MODE_KEY = "shape";
const char* Z3DFilterSetting::SIZE_SCALE_KEY = "size_scale";
const char* Z3DFilterSetting::VISIBLE_KEY = "visible";
const char* Z3DFilterSetting::OPACITY_KEY = "opacity";

Z3DFilterSetting::Z3DFilterSetting()
{
  init();
}

void Z3DFilterSetting::init()
{
  m_visible = true;
  m_sizeScale = 1.0;
  m_front = false;
}

void Z3DFilterSetting::setFront(bool front)
{
  m_front = front;
}

void Z3DFilterSetting::setSizeScale(double scale)
{
  m_sizeScale = scale;
}

void Z3DFilterSetting::setVisible(bool visible)
{
  m_visible = visible;
}

void Z3DFilterSetting::load(const ZJsonObject &obj)
{
  if (obj.hasKey(FRONT_KEY)) {
    m_front = ZJsonParser::booleanValue(obj[FRONT_KEY]);
  }

  if (obj.hasKey(VISIBLE_KEY)) {
    m_visible = ZJsonParser::booleanValue(obj[VISIBLE_KEY]);
  }

  if (obj.hasKey(SIZE_SCALE_KEY)) {
    m_sizeScale = ZJsonParser::numberValue(obj[SIZE_SCALE_KEY]);
  }

  if (obj.hasKey(SHAPE_MODE_KEY)) {
    m_shapeMode = ZJsonParser::stringValue(obj[SHAPE_MODE_KEY]).c_str();
  }

  if (obj.hasKey(COLOR_MODE_KEY)) {
    m_colorMode = ZJsonParser::stringValue(obj[COLOR_MODE_KEY]).c_str();
  }
}

bool Z3DFilterSetting::isVisible() const
{
  return m_visible;
}

bool Z3DFilterSetting::isFront() const
{
  return m_front;
}

QString Z3DFilterSetting::getColorMode() const
{
  return m_colorMode;
}

QString Z3DFilterSetting::getShapeMode() const
{
  return m_shapeMode;
}

double Z3DFilterSetting::getSizeScale() const
{
  return m_sizeScale;
}
