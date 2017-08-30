#ifndef Z3DFONTRENDERER_H
#define Z3DFONTRENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dsdfont.h"

class Z3DFontRenderer : public Z3DPrimitiveRenderer
{
Q_OBJECT
public:
  explicit Z3DFontRenderer(Z3DRendererBase& rendererBase);

  void setData(std::vector<glm::vec3>* positions, const QStringList& texts);

  void setDataColors(std::vector<glm::vec4>* colors);

  void setDataPickingColors(std::vector<glm::vec4>* pickingColors = nullptr);

  ZStringIntOptionParameter& allFontNamesPara()
  { return m_allFontNames; }

  ZFloatParameter& fontSizePara()
  { return m_fontSize; }

  ZFloatParameter& fontSoftEdgeScalePara()
  { return m_fontSoftEdgeScale; }

  ZBoolParameter& showFontOutlinePara()
  { return m_showFontOutline; }

  ZStringIntOptionParameter& fontOutlineModePara()
  { return m_fontOutlineMode; }

  ZVec4Parameter& fontOutlineColorPara()
  { return m_fontOutlineColor; }

  ZBoolParameter& showFontShadowPara()
  { return m_showFontShadow; }

  ZVec4Parameter& fontShadowColorPara()
  { return m_fontShadowColor; }

protected:
  void adjustWidgets();

  virtual void compile() override;

  std::vector<glm::vec4>* getColors();

  QString generateHeader();

  virtual void render(Z3DEye eye) override;

  virtual void renderPicking(Z3DEye /*unused*/) override;

  void prepareFontShaderData(Z3DEye eye);

protected:
  Z3DShaderGroup m_fontShaderGrp;

  ZStringIntOptionParameter m_allFontNames;  // font name and index into m_allFonts
  ZFloatParameter m_fontSize;   //font size in world coordinate
  ZBoolParameter m_fontUseSoftEdge;
  ZFloatParameter m_fontSoftEdgeScale;
  ZBoolParameter m_showFontOutline;
  ZStringIntOptionParameter m_fontOutlineMode;
  ZVec4Parameter m_fontOutlineColor;
  ZBoolParameter m_showFontShadow;
  ZVec4Parameter m_fontShadowColor;

  std::vector<std::unique_ptr<Z3DSDFont>> m_allFonts;

  std::vector<glm::vec3>* m_positionsPt;
  std::vector<glm::vec4>* m_colorsPt;
  std::vector<glm::vec4>* m_pickingColorsPt;
  QStringList m_texts;
  std::vector<glm::vec4> m_colors;
  std::vector<glm::vec4> m_fontColors;
  std::vector<glm::vec4> m_fontPickingColors;
  std::vector<glm::vec3> m_fontPositions;
  std::vector<glm::vec2> m_fontTextureCoords;
  std::vector<GLuint> m_indexs;

  ZVertexArrayObject m_VAO;
  ZVertexBufferObject m_VBOs;
  ZVertexBufferObject m_pickingVBOs;
  bool m_dataChanged;
  bool m_pickingDataChanged;
};

#endif // Z3DFONTRENDERER_H
