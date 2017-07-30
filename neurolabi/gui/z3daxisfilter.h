#ifndef Z3DAXISFILTER_H
#define Z3DAXISFILTER_H

#include "z3dgeometryfilter.h"
#include "z3darrowrenderer.h"
#include "z3dfontrenderer.h"
#include "z3dlinerenderer.h"

class ZWidgetsGroup;

class Z3DAxisFilter : public Z3DGeometryFilter
{
Q_OBJECT
public:
  explicit Z3DAxisFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);

  virtual bool isReady(Z3DEye eye) const override;

  void setVisible(bool visible);

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  virtual bool hasOpaque(Z3DEye /*unused*/) const override
  { return false; }

  virtual void renderOpaque(Z3DEye eye) override;

  virtual bool hasTransparent(Z3DEye /*unused*/) const override
  { return true; }

  virtual void renderTransparent(Z3DEye eye) override
  { renderOpaque(eye); }

protected:
  void prepareData(Z3DEye eye);

  void setupCamera();

protected:
  Z3DLineRenderer m_lineRenderer;
  Z3DArrowRenderer m_arrowRenderer;
  Z3DFontRenderer m_fontRenderer;

  ZBoolParameter m_showAxis;
  ZVec4Parameter m_XAxisColor;
  ZVec4Parameter m_YAxisColor;
  ZVec4Parameter m_ZAxisColor;
  ZFloatParameter m_axisRegionRatio;
  ZStringIntOptionParameter m_mode;

  std::vector<glm::vec4> m_tailPosAndTailRadius;
  std::vector<glm::vec4> m_headPosAndHeadRadius;
  std::vector<glm::vec4> m_lineColors;
  std::vector<glm::vec3> m_lines;
  std::vector<glm::vec4> m_textColors;
  std::vector<glm::vec3> m_textPositions;

  glm::vec3 m_XEnd;
  glm::vec3 m_YEnd;
  glm::vec3 m_ZEnd;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;
};

#endif // Z3DAXISFILTER_H
