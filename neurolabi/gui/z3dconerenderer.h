#ifndef Z3DCONERENDERER_H
#define Z3DCONERENDERER_H

#include "z3dprimitiverenderer.h"

class Z3DConeRenderer : public Z3DPrimitiveRenderer
{
Q_OBJECT
public:
  // default use display list and lighting in opengl mode
  // Round cap style might have bug. It only works when we are dealing with cylinder with slightly different radius.
  explicit Z3DConeRenderer(Z3DRendererBase& rendererBase);

  // base radius should be smaller than top radius
  void setData(std::vector<glm::vec4>* baseAndBaseRadius, std::vector<glm::vec4>* axisAndTopRadius);

  void setDataColors(std::vector<glm::vec4>* coneColors);

  void setDataColors(std::vector<glm::vec4>* coneBaseColors, std::vector<glm::vec4>* coneTopColors);

  void setDataPickingColors(std::vector<glm::vec4>* conePickingColors = nullptr);

  ZStringStringOptionParameter& coneCapStylePara()
  { return m_coneCapStyle; }

protected:
  virtual void compile() override;

  QString generateHeader();

#ifndef _USE_CORE_PROFILE_
  virtual void renderUsingOpengl() override;
  virtual void renderPickingUsingOpengl() override;
#endif

  virtual void render(Z3DEye eye) override;

  virtual void renderPicking(Z3DEye eye) override;

  void appendDefaultColors();

protected:
  Z3DShaderGroup m_coneShaderGrp;

  ZStringStringOptionParameter m_coneCapStyle;
  ZIntParameter m_cylinderSubdivisionAroundZ;
  ZIntParameter m_cylinderSubdivisionAlongZ;

private:
  std::vector<glm::vec4> m_baseAndBaseRadius;
  std::vector<glm::vec4> m_axisAndTopRadius;
  std::vector<glm::vec4> m_coneBaseColors;
  std::vector<glm::vec4> m_coneTopColors;
  std::vector<glm::vec4> m_conePickingColors;
  std::vector<GLfloat> m_allFlags;
  std::vector<GLuint> m_indexs;

  bool m_sameColorForBaseAndTop;

  bool m_useConeShader2;

  ZVertexArrayObject m_VAO;
  ZVertexArrayObject m_pickingVAO;
  ZVertexBufferObject m_VBOs;
  ZVertexBufferObject m_pickingVBOs;
  bool m_dataChanged;
  bool m_pickingDataChanged;
};

#endif // Z3DCONERENDERER_H
