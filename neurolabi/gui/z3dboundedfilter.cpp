#include "z3dboundedfilter.h"

#include "logging/zqslog.h"
#include "zintcuboid.h"
#include <boost/math/constants/constants.hpp>

#include "zlinesegment.h"

Z3DBoundedFilter::Z3DBoundedFilter(Z3DGlobalParameters& globalPara, QObject* parent)
  : Z3DFilter(parent)
  , m_rendererBase(globalPara)
  , m_baseBoundBoxRenderer(m_rendererBase)
  , m_selectionBoundBoxRenderer(m_rendererBase)
  , m_visible("Visible", true)
  , m_xCut("X Cut", glm::vec2(0, 0), 0, 0)
  , m_yCut("Y Cut", glm::vec2(0, 0), 0, 0)
  , m_zCut("Z Cut", glm::vec2(0, 0), 0, 0)
  , m_boundBoxMode("Bound Box")
  , m_boundBoxLineWidth("Bound Box Line Width", 1, 1, 100)
  , m_boundBoxLineColor("Bound Box Line Color", glm::vec4(0.f, 1.f, 1.f, 1.f))
  //, m_boundBoxLineColor("Bound Box Line Color")
  , m_selectionLineWidth("Selection Line Width", 1, 1, 100)
  , m_selectionLineColor("Selection Line Color", glm::vec4(1.f, 1.f, 0.f, 1.f))
  , m_canUpdateClipPlane(true)
  , m_transformEnabled(true)
{
  m_boundBoxMode.addOptions("No Bound Box", "Bound Box", "Axis Aligned Bound Box");
  m_boundBoxMode.select("No Bound Box");

  connect(&m_rendererBase, &Z3DRendererBase::coordTransformChanged, this,
          &Z3DBoundedFilter::updateAxisAlignedBoundBox);
  connect(&m_rendererBase, &Z3DRendererBase::sizeScaleChanged, this,
          &Z3DBoundedFilter::updateBoundBox);
  connect(&m_rendererBase, SIGNAL(opacityChanged(double)),
          this, SIGNAL(opacityChanged(double)));

  m_xCut.setSingleStep(1);
  m_yCut.setSingleStep(1);
  m_zCut.setSingleStep(1);
  connect(&m_xCut, &ZFloatSpanParameter::valueChanged, this, &Z3DBoundedFilter::setClipPlanes);
  connect(&m_yCut, &ZFloatSpanParameter::valueChanged, this, &Z3DBoundedFilter::setClipPlanes);
  connect(&m_zCut, &ZFloatSpanParameter::valueChanged, this, &Z3DBoundedFilter::setClipPlanes);
  connect(&m_boundBoxMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DBoundedFilter::onBoundBoxModeChanged);
  m_boundBoxLineColor.setStyle("COLOR");
  //m_boundBoxLineColor.get().reset(0., 1., QColor(133,163,240,255), QColor(248,60,35,255));
  //m_boundBoxLineColor.get().addKey(ZColorMapKey(.1, QColor(233,239,235,255)));
  //m_boundBoxLineColor.get().addKey(ZColorMapKey(.2, QColor(240,241,237,255)));
  //m_boundBoxLineColor.get().addKey(ZColorMapKey(.3, QColor(248,205,165,255)));
  connect(&m_boundBoxLineColor, &ZVec4Parameter::valueChanged, this, &Z3DBoundedFilter::updateBoundBoxLineColors);
  m_selectionLineColor.setStyle("COLOR");
  connect(&m_selectionLineColor, &ZVec4Parameter::valueChanged, this, &Z3DBoundedFilter::updateSelectionLineColors);

  connect(&m_visible, &ZBoolParameter::boolChanged,
          this, &Z3DBoundedFilter::objVisibleChanged);

  addParameter(m_visible);
  addParameter(m_xCut);
  addParameter(m_yCut);
  addParameter(m_zCut);
  addParameter(m_boundBoxMode);
  addParameter(m_boundBoxLineWidth);
  addParameter(m_boundBoxLineColor);
  addParameter(m_selectionLineWidth);
  addParameter(m_selectionLineColor);

  onBoundBoxModeChanged();

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  m_baseBoundBoxRenderer.setUseDisplayList(false);
#endif
  m_baseBoundBoxRenderer.setFollowSizeScale(false);
  m_baseBoundBoxRenderer.setFollowCoordTransform(false);
  m_baseBoundBoxRenderer.setLineWidth(m_boundBoxLineWidth.get());
  connect(&m_boundBoxLineWidth, &ZIntParameter::valueChanged, this, &Z3DBoundedFilter::onBoundBoxLineWidthChanged);
  updateBoundBoxLineColors();

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  m_selectionBoundBoxRenderer.setUseDisplayList(false);
#endif
  m_selectionBoundBoxRenderer.setFollowCoordTransform(false);
  m_selectionBoundBoxRenderer.setFollowSizeScale(false);
  m_selectionBoundBoxRenderer.setFollowOpacity(false);
  m_selectionBoundBoxRenderer.setEnableMultisample(false);
  m_selectionBoundBoxRenderer.setLineWidth(m_selectionLineWidth.get());
  connect(&m_selectionLineWidth, &ZIntParameter::valueChanged, this,
          &Z3DBoundedFilter::onSelectionBoundBoxLineWidthChanged);
  updateSelectionLineColors();

  const std::vector<ZParameter*>& globalParas = m_rendererBase.globalParameters();
  for (size_t i = 0; i < globalParas.size(); ++i) {
    connect(globalParas[i], &ZParameter::valueChanged, this, &Z3DBoundedFilter::invalidateResult);
  }
  const std::vector<ZParameter*>& paras = m_rendererBase.parameters();
  for (size_t i = 0; i < paras.size(); ++i) {
    addParameter(*paras[i]);
  }
}

void Z3DBoundedFilter::renderSelectionBox(Z3DEye eye)
{
  m_selectionLines.clear();
  addSelectionLines();
  if (!m_selectionLines.empty()) {
    m_selectionBoundBoxRenderer.setData(&m_selectionLines);
    if (m_selectionLineColors.size() < m_selectionLines.size()) {
      for (size_t i = m_selectionLineColors.size(); i < m_selectionLines.size(); ++i) {
        m_selectionLineColors.push_back(m_selectionLineColor.get());
      }
      m_selectionBoundBoxRenderer.setDataColors(&m_selectionLineColors);
    }
    m_rendererBase.setClipEnabled(false);
    m_rendererBase.render(eye, m_selectionBoundBoxRenderer);
    m_rendererBase.setClipEnabled(true);
  }
}

void Z3DBoundedFilter::setOpacity(float o)
{
  m_rendererBase.setOpacity(o);
//  emit opacityChanged(o);
}

void Z3DBoundedFilter::setOpacityQuitely(float o)
{
  blockSignals(true);
  m_rendererBase.setOpacity(o);
  blockSignals(false);
}

glm::vec3 Z3DBoundedFilter::getViewCoord(double x, double y, double z, double w, double h)
{
  glm::ivec4 viewport(0, 0, w, h);
  glm::vec3 pt = m_rendererBase.camera().worldToScreen(glm::applyMatrix(m_rendererBase.coordTransform(), glm::vec3(x, y, z)), viewport);

  pt[1] = h - pt[1];

  return pt;
}

void Z3DBoundedFilter::resetCut()
{
  setXCutLower(xCutMin());
  setYCutLower(yCutMin());
  setZCutLower(zCutMin());

  setXCutUpper(xCutMax());
  setYCutUpper(yCutMax());
  setZCutUpper(zCutMax());
}

ZIntCuboid Z3DBoundedFilter::cutBox()
{
  ZIntCuboid box;
  box.set(m_xCut.lowerValue(), m_yCut.lowerValue(), m_zCut.lowerValue(),
          m_xCut.upperValue(), m_yCut.upperValue(), m_zCut.upperValue());

  return box;
}

void Z3DBoundedFilter::setCutBox(const ZIntCuboid& box)
{
  m_xCut.set(glm::vec2(box.getFirstCorner().getX(), box.getLastCorner().getX()));
  m_yCut.set(glm::vec2(box.getFirstCorner().getY(), box.getLastCorner().getY()));
  m_zCut.set(glm::vec2(box.getFirstCorner().getZ(), box.getLastCorner().getZ()));
}

void Z3DBoundedFilter::updateBoundBox()
{
  updateNotTransformedBoundBox();
  m_normalBoundBoxLines.clear();
  appendBoundboxLines(m_notTransformedBoundBox, m_normalBoundBoxLines);
  if (m_boundBoxMode.isSelected("Bound Box"))
    m_baseBoundBoxRenderer.setData(&m_normalBoundBoxLines);
  updateAxisAlignedBoundBox();
}

void Z3DBoundedFilter::setClipPlanes()
{
  if (!m_canUpdateClipPlane)
    return;
  std::vector<glm::vec4> clipPlanes;
  if (m_xCut.lowerValue() != m_xCut.minimum())
    clipPlanes.emplace_back(1., 0., 0., -m_xCut.lowerValue());
  if (m_xCut.upperValue() != m_xCut.maximum())
    clipPlanes.emplace_back(-1., 0., 0., m_xCut.upperValue());
  if (m_yCut.lowerValue() != m_yCut.minimum())
    clipPlanes.emplace_back(0., 1., 0., -m_yCut.lowerValue());
  if (m_yCut.upperValue() != m_yCut.maximum())
    clipPlanes.emplace_back(0., -1., 0., m_yCut.upperValue());
  if (m_zCut.lowerValue() != m_zCut.minimum())
    clipPlanes.emplace_back(0., 0., 1., -m_zCut.lowerValue());
  if (m_zCut.upperValue() != m_zCut.maximum())
    clipPlanes.emplace_back(0., 0., -1., m_zCut.upperValue());
  m_rendererBase.setClipPlanes(&clipPlanes);
}

void Z3DBoundedFilter::initializeCutRange()
{
  m_canUpdateClipPlane = false;
  const ZBBox<glm::dvec3>& bound = notTransformedBoundBox();
  m_xCut.setRange(std::floor(bound.minCorner().x) - 1,
                  std::ceil(bound.maxCorner().x) + 1);
  m_xCut.set(m_xCut.range());
  m_yCut.setRange(std::floor(bound.minCorner().y) - 1,
                  std::ceil(bound.maxCorner().y) + 1);
  m_yCut.set(m_yCut.range());
  m_zCut.setRange(std::floor(bound.minCorner().z) - 1,
                  std::ceil(bound.maxCorner().z) + 1);
  m_zCut.set(m_zCut.range());
  m_canUpdateClipPlane = true;
  m_rendererBase.setClipPlanes(nullptr);
}

void Z3DBoundedFilter::initializeRotationCenter()
{
  const ZBBox<glm::dvec3>& bound = notTransformedBoundBox();
  m_rendererBase.setRotationCenter(glm::vec3((bound.minCorner() + bound.maxCorner()) / 2.0));
}

void Z3DBoundedFilter::renderBoundBox(Z3DEye eye)
{
  if (!m_boundBoxMode.isSelected("No Bound Box")) {
    m_rendererBase.setClipEnabled(false);
    m_rendererBase.render(eye, m_baseBoundBoxRenderer);
    m_rendererBase.setClipEnabled(true);
  }
}

void Z3DBoundedFilter::appendBoundboxLines(const ZBBox<glm::dvec3>& bound, std::vector<glm::vec3>& lines)
{
  float xmin = bound.minCorner().x;
  float xmax = bound.maxCorner().x;
  float ymin = bound.minCorner().y;
  float ymax = bound.maxCorner().y;
  float zmin = bound.minCorner().z;
  float zmax = bound.maxCorner().z;
  lines.emplace_back(xmin, ymin, zmin);
  lines.emplace_back(xmin, ymin, zmax);
  lines.emplace_back(xmin, ymax, zmin);
  lines.emplace_back(xmin, ymax, zmax);

  lines.emplace_back(xmax, ymin, zmin);
  lines.emplace_back(xmax, ymin, zmax);
  lines.emplace_back(xmax, ymax, zmin);
  lines.emplace_back(xmax, ymax, zmax);

  lines.emplace_back(xmin, ymin, zmin);
  lines.emplace_back(xmax, ymin, zmin);
  lines.emplace_back(xmin, ymax, zmin);
  lines.emplace_back(xmax, ymax, zmin);

  lines.emplace_back(xmin, ymin, zmax);
  lines.emplace_back(xmax, ymin, zmax);
  lines.emplace_back(xmin, ymax, zmax);
  lines.emplace_back(xmax, ymax, zmax);

  lines.emplace_back(xmin, ymin, zmin);
  lines.emplace_back(xmin, ymax, zmin);
  lines.emplace_back(xmax, ymin, zmin);
  lines.emplace_back(xmax, ymax, zmin);

  lines.emplace_back(xmin, ymin, zmax);
  lines.emplace_back(xmin, ymax, zmax);
  lines.emplace_back(xmax, ymin, zmax);
  lines.emplace_back(xmax, ymax, zmax);
}

void Z3DBoundedFilter::rayUnderScreenPoint(
    glm::vec3& v1, glm::vec3& v2, int x, int y, int width, int height)
{
  const glm::mat4& projection = globalCamera().projectionMatrix(Z3DEye::Mono);
  const glm::mat4& modelview = globalCamera().viewMatrix(Z3DEye::Mono);

  glm::ivec4 viewport(0, 0, width, height);

  v1 = glm::unProject(glm::vec3(x, height - y, 0.f), modelview, projection, viewport);
  v2 = glm::unProject(glm::vec3(x, height - y, 1.f), modelview, projection, viewport);
  v2 = glm::normalize(v2 - v1) + v1;
}

void Z3DBoundedFilter::rayUnderScreenPoint(
    glm::dvec3& v1, glm::dvec3& v2, int x, int y, int width, int height)
{
  const glm::dmat4& projection = glm::dmat4(globalCamera().projectionMatrix(Z3DEye::Mono));
  const glm::dmat4& modelview = glm::dmat4(globalCamera().viewMatrix(Z3DEye::Mono));

  glm::ivec4 viewport(0, 0, width, height);

  v1 = glm::unProject(glm::dvec3(x, height - y, -1.0f), modelview, projection, viewport);
  v2 = glm::unProject(glm::dvec3(x, height - y, 1.f), modelview, projection, viewport);
  v2 = glm::normalize(v2 - v1) + v1;
}

ZLineSegment Z3DBoundedFilter::getScreenRay(int x, int y, int width, int height)
{
  ZLineSegment seg;
  glm::dvec3 v1;
  glm::dvec3 v2;
  rayUnderScreenPoint(v1, v2, x, y, width, height);

  seg.setStartPoint(v1.x, v1.y, v1.z);
  seg.setEndPoint(v2.x, v2.y, v2.z);

  return seg;
}

void Z3DBoundedFilter::updateAxisAlignedBoundBoxImpl()
{
  m_axisAlignedBoundBox.reset();
  if (!m_notTransformedBoundBox.empty()) {
    m_axisAlignedBoundBox.expand(glm::dvec3(worldLUF()));
    m_axisAlignedBoundBox.expand(glm::dvec3(worldLDB()));
    m_axisAlignedBoundBox.expand(glm::dvec3(worldLDF()));
    m_axisAlignedBoundBox.expand(glm::dvec3(worldLUB()));
    m_axisAlignedBoundBox.expand(glm::dvec3(worldRUF()));
    m_axisAlignedBoundBox.expand(glm::dvec3(worldRDB()));
    m_axisAlignedBoundBox.expand(glm::dvec3(worldRDF()));
    m_axisAlignedBoundBox.expand(glm::dvec3(worldRUB()));
  }
}

void Z3DBoundedFilter::expandCutRange()
{
  m_canUpdateClipPlane = false;
  const ZBBox<glm::dvec3>& bound = notTransformedBoundBox();
  bool noLowXCut = m_xCut.lowerValue() == m_xCut.minimum();
  bool noHighXCut = m_xCut.upperValue() == m_xCut.maximum();
  bool noLowYCut = m_yCut.lowerValue() == m_yCut.minimum();
  bool noHighYCut = m_yCut.upperValue() == m_yCut.maximum();
  bool noLowZCut = m_zCut.lowerValue() == m_zCut.minimum();
  bool noHighZCut = m_zCut.upperValue() == m_zCut.maximum();
  m_xCut.setRange(std::min(m_xCut.minimum(), float(std::floor(bound.minCorner().x) - 1)),
                  std::max(m_xCut.maximum(), float(std::ceil(bound.maxCorner().x) + 1)));
  m_yCut.setRange(std::min(m_yCut.minimum(), float(std::floor(bound.minCorner().y) - 1)),
                  std::max(m_yCut.maximum(), float(std::ceil(bound.maxCorner().y) + 1)));
  m_zCut.setRange(std::min(m_yCut.minimum(), float(std::floor(bound.minCorner().z) - 1)),
                  std::max(m_zCut.maximum(), float(std::ceil(bound.maxCorner().z) + 1)));
  float xCutLow = noLowXCut ? m_xCut.minimum() : m_xCut.get().x;
  float xCutHigh = noHighXCut ? m_xCut.maximum() : m_xCut.get().y;
  float yCutLow = noLowYCut ? m_yCut.minimum() : m_yCut.get().x;
  float yCutHigh = noHighYCut ? m_yCut.maximum() : m_yCut.get().y;
  float zCutLow = noLowZCut ? m_zCut.minimum() : m_zCut.get().x;
  float zCutHigh = noHighZCut ? m_zCut.maximum() : m_zCut.get().y;
  m_xCut.set(glm::vec2(xCutLow, xCutHigh));
  m_yCut.set(glm::vec2(yCutLow, yCutHigh));
  m_zCut.set(glm::vec2(zCutLow, zCutHigh));
  m_canUpdateClipPlane = true;
  setClipPlanes();
}

void Z3DBoundedFilter::updateAxisAlignedBoundBox()
{
  if (m_rendererBase.coordTransform() == glm::mat4(1.f)) {
    m_axisAlignedBoundBox = m_notTransformedBoundBox;
  } else {
    updateAxisAlignedBoundBoxImpl();
  }
  m_axisAlignedBoundBoxLines.clear();
  appendBoundboxLines(m_axisAlignedBoundBox, m_axisAlignedBoundBoxLines);
  if (m_boundBoxMode.isSelected("Axis Aligned Bound Box")) {
    m_baseBoundBoxRenderer.setData(&m_axisAlignedBoundBoxLines);
  }

  m_center = glm::vec3((m_axisAlignedBoundBox.minCorner() + m_axisAlignedBoundBox.maxCorner()) / 2.0);

  emit boundBoxChanged();
}

void Z3DBoundedFilter::updateNotTransformedBoundBox()
{
  updateNotTransformedBoundBoxImpl();
  expandCutRange();
}

void Z3DBoundedFilter::onBoundBoxModeChanged()
{
  if (m_boundBoxMode.isSelected("Axis Aligned Bound Box")) {
    m_baseBoundBoxRenderer.setData(&m_axisAlignedBoundBoxLines);
    m_baseBoundBoxRenderer.setFollowCoordTransform(false);
  } else if (m_boundBoxMode.isSelected("Bound Box")) {
    m_baseBoundBoxRenderer.setData(&m_normalBoundBoxLines);
    m_baseBoundBoxRenderer.setFollowCoordTransform(true);
  }
  m_boundBoxLineWidth.setVisible(!m_boundBoxMode.isSelected("No Bound Box"));
  m_boundBoxLineColor.setVisible(!m_boundBoxMode.isSelected("No Bound Box"));
}

void Z3DBoundedFilter::updateBoundBoxLineColors()
{
  m_boundBoxLineColors.clear();
  m_boundBoxLineColors.resize(24, m_boundBoxLineColor.get());
  m_baseBoundBoxRenderer.setDataColors(&m_boundBoxLineColors);
  //m_baseBoundBoxRenderer->setTexture(m_boundBoxLineColor.get().getTexture());
}

void Z3DBoundedFilter::updateSelectionLineColors()
{
  m_selectionLineColors.clear();
  //m_selectionLineColors.resize(24, m_selectionLineColor.get());
  m_selectionBoundBoxRenderer.setDataColors(&m_selectionLineColors);
}

void Z3DBoundedFilter::onBoundBoxLineWidthChanged()
{
  m_baseBoundBoxRenderer.setLineWidth(m_boundBoxLineWidth.get());
}

void Z3DBoundedFilter::onSelectionBoundBoxLineWidthChanged()
{
  m_selectionBoundBoxRenderer.setLineWidth(m_selectionLineWidth.get());
}
