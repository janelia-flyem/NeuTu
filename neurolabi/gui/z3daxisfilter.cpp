#include "z3daxisfilter.h"

#include "zwidgetsgroup.h"

Z3DAxisFilter::Z3DAxisFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_lineRenderer(m_rendererBase)
  , m_arrowRenderer(m_rendererBase)
  , m_fontRenderer(m_rendererBase)
  , m_showAxis("Show Axis", true)
  , m_XAxisColor("X Axis Color", glm::vec4(1.f, 0.f, 0.f, 1.0f))
  , m_YAxisColor("Y Axis Color", glm::vec4(0.f, 1.f, 0.f, 1.0f))
  , m_ZAxisColor("Z Axis Color", glm::vec4(0.f, 0.f, 1.f, 1.0f))
  , m_axisRegionRatio("Axis Region Ratio", .2f, .1f, 1.f)
  , m_mode("mode")
{
  m_XAxisColor.setStyle("COLOR");
  m_YAxisColor.setStyle("COLOR");
  m_ZAxisColor.setStyle("COLOR");
  m_mode.addOptions("Arrow", "Line");
  m_mode.select("Arrow");
  addParameter(m_showAxis);
  addParameter(m_XAxisColor);
  addParameter(m_YAxisColor);
  addParameter(m_ZAxisColor);
  addParameter(m_axisRegionRatio);
  addParameter(m_mode);

  addParameter(m_fontRenderer.allFontNamesPara());
  addParameter(m_fontRenderer.fontPara());
  addParameter(m_fontRenderer.fontSizePara());
  addParameter(m_fontRenderer.fontSoftEdgeScalePara());
  addParameter(m_fontRenderer.showFontOutlinePara());
  addParameter(m_fontRenderer.fontOutlineModePara());
  addParameter(m_fontRenderer.fontOutlineColorPara());
  addParameter(m_fontRenderer.showFontShadowPara());
  addParameter(m_fontRenderer.fontShadowColorPara());

  m_arrowRenderer.setUseDisplayList(false);
  m_lineRenderer.setUseDisplayList(false);
  m_fontRenderer.setFollowCoordTransform(false);
  setupCamera();
}

bool Z3DAxisFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && m_showAxis.get();
}

void Z3DAxisFilter::setVisible(bool visible)
{
  m_showAxis.setValue(visible);
}

std::shared_ptr<ZWidgetsGroup> Z3DAxisFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("Axis", 1);
    m_widgetsGroup->addChild(m_showAxis, 1);
    m_widgetsGroup->addChild(m_mode, 1);
    m_widgetsGroup->addChild(m_axisRegionRatio, 1);
    m_widgetsGroup->addChild(m_XAxisColor, 1);
    m_widgetsGroup->addChild(m_YAxisColor, 1);
    m_widgetsGroup->addChild(m_ZAxisColor, 1);
    std::vector<ZParameter*> paras = m_rendererBase.parameters();
    for (size_t i = 0; i < paras.size(); ++i) {
      ZParameter* para = paras[i];
      if (para->name() == "Size Scale")
        m_widgetsGroup->addChild(*para, 1);
      else if (para->name() == "Rendering Method")
        m_widgetsGroup->addChild(*para, 3);
      else if (para->name() == "Opacity")
        m_widgetsGroup->addChild(*para, 3);
    }
    m_widgetsGroup->addChild(m_fontRenderer.allFontNamesPara(), 4);
    m_widgetsGroup->addChild(m_fontRenderer.fontPara(), 4);
    m_widgetsGroup->addChild(m_fontRenderer.fontSizePara(), 4);
    m_widgetsGroup->addChild(m_fontRenderer.fontSoftEdgeScalePara(), 4);
    m_widgetsGroup->addChild(m_fontRenderer.showFontOutlinePara(), 4);
    m_widgetsGroup->addChild(m_fontRenderer.fontOutlineModePara(), 4);
    m_widgetsGroup->addChild(m_fontRenderer.fontOutlineColorPara(), 4);
    m_widgetsGroup->addChild(m_fontRenderer.showFontShadowPara(), 4);
    m_widgetsGroup->addChild(m_fontRenderer.fontShadowColorPara(), 4);
    m_widgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_widgetsGroup;
}

void Z3DAxisFilter::renderOpaque(Z3DEye eye)
{
  prepareData(eye);
  m_rendererBase.coordTransformPara().blockSignals(true);
  m_rendererBase.coordTransformPara().set(glm::mat4(globalCamera().rotateMatrix(eye)));

  glm::uvec4 viewport = m_rendererBase.viewport();
  GLsizei size = std::min(viewport.z, viewport.w) * m_axisRegionRatio.get();
  glViewport(viewport.x, viewport.y, size, size);

  if (m_mode.get() == "Arrow")
    m_rendererBase.render(eye, m_arrowRenderer, m_fontRenderer);
  else
    m_rendererBase.render(eye, m_lineRenderer, m_fontRenderer);

  glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
  m_rendererBase.coordTransformPara().blockSignals(false);
}

void Z3DAxisFilter::prepareData(Z3DEye eye)
{
  m_textPositions.clear();
  glm::mat3 rotMatrix = globalCamera().rotateMatrix(eye);
  m_XEnd = rotMatrix * glm::vec3(256.f, 0.f, 0.f);
  m_YEnd = rotMatrix * glm::vec3(0.f, 256.f, 0.f);
  m_ZEnd = rotMatrix * glm::vec3(0.f, 0.f, 256.f);

  m_textPositions.push_back(m_XEnd * glm::vec3(0.93));
  m_textPositions.push_back(m_YEnd * glm::vec3(0.93));
  m_textPositions.push_back(m_ZEnd * glm::vec3(0.93));
  QStringList texts;
  texts.push_back("X");
  texts.push_back("Y");
  texts.push_back("Z");

  m_fontRenderer.setData(&m_textPositions, texts);
}

void Z3DAxisFilter::setupCamera()
{
  Z3DCamera camera;
  glm::vec3 center(0.f);
  camera.setFieldOfView(glm::radians(10.f));

  float radius = 300.f;

  float distance = radius / std::sin(camera.fieldOfView() * 0.5);
  glm::vec3 vn(0, 0, 1);     //plane normal
  glm::vec3 position = center + vn * distance;
  camera.setCamera(position, center, glm::vec3(0.0, 1.0, 0.0));
  camera.setNearDist(distance - radius - 1);
  camera.setFarDist(distance + radius);

  m_rendererBase.setCamera(camera);

  m_tailPosAndTailRadius.clear();
  m_headPosAndHeadRadius.clear();
  m_lineColors.clear();
  m_lines.clear();
  m_textColors.clear();
  m_textPositions.clear();
  m_XEnd = glm::vec3(256.f, 0.f, 0.f);
  m_YEnd = glm::vec3(0.f, 256.f, 0.f);
  m_ZEnd = glm::vec3(0.f, 0.f, 256.f);
  glm::vec3 origin(0.f);
  m_lines.push_back(origin);
  m_lineColors.push_back(m_XAxisColor.get());
  m_lines.push_back(m_XEnd * glm::vec3(0.88));
  m_lineColors.push_back(m_XAxisColor.get());
  m_lines.push_back(origin);
  m_lineColors.push_back(m_YAxisColor.get());
  m_lines.push_back(m_YEnd * glm::vec3(0.88));
  m_lineColors.push_back(m_YAxisColor.get());
  m_lines.push_back(origin);
  m_lineColors.push_back(m_ZAxisColor.get());
  m_lines.push_back(m_ZEnd * glm::vec3(0.88));
  m_lineColors.push_back(m_ZAxisColor.get());

  m_textPositions.push_back(m_XEnd * glm::vec3(0.93));
  m_textPositions.push_back(m_YEnd * glm::vec3(0.93));
  m_textPositions.push_back(m_ZEnd * glm::vec3(0.93));
  m_textColors.push_back(m_XAxisColor.get());
  m_textColors.push_back(m_YAxisColor.get());
  m_textColors.push_back(m_ZAxisColor.get());
  QStringList texts;
  texts.push_back("X");
  texts.push_back("Y");
  texts.push_back("Z");

  float tailRadius = 5.f;
  float headRadius = 10.f;

  m_tailPosAndTailRadius.emplace_back(origin, tailRadius);
  m_headPosAndHeadRadius.emplace_back(m_XEnd * glm::vec3(0.88), headRadius);

  m_tailPosAndTailRadius.emplace_back(origin, tailRadius);
  m_headPosAndHeadRadius.emplace_back(m_YEnd * glm::vec3(0.88), headRadius);

  m_tailPosAndTailRadius.emplace_back(origin, tailRadius);
  m_headPosAndHeadRadius.emplace_back(m_ZEnd * glm::vec3(0.88), headRadius);

  m_lineRenderer.setData(&m_lines);
  m_lineRenderer.setDataColors(&m_lineColors);
  m_arrowRenderer.setArrowData(&m_tailPosAndTailRadius, &m_headPosAndHeadRadius, .1f);
  m_arrowRenderer.setArrowColors(&m_textColors);
  m_fontRenderer.setData(&m_textPositions, texts);
  m_fontRenderer.setDataColors(&m_textColors);
}
