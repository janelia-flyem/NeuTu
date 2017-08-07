#ifndef Z3DSPHERERENDERER_H
#define Z3DSPHERERENDERER_H

#include "z3dprimitiverenderer.h"

class Z3DSphereRenderer : public Z3DPrimitiveRenderer
{
Q_OBJECT
public:
  // default use display list and lighting for opengl mode
  explicit Z3DSphereRenderer(Z3DRendererBase& rendererBase);

  void setData(std::vector<glm::vec4>* pointAndRadiusInput, std::vector<glm::vec4>* specularAndShininessInput = nullptr);

  void setDataColors(std::vector<glm::vec4>* pointColorsInput);

  void setDataPickingColors(std::vector<glm::vec4>* pointPickingColorsInput = nullptr);

  ZBoolParameter& useDynamicMaterialPara()
  { return m_useDynamicMaterial; }

protected:
  virtual void compile() override;

  QString generateHeader();

#ifndef ATLAS_USE_CORE_PROFILE
  virtual void renderUsingOpengl() override;
  virtual void renderPickingUsingOpengl() override;
#endif

  virtual void render(Z3DEye eye) override;

  virtual void renderPicking(Z3DEye eye) override;

  void appendDefaultColors();

protected:
  Z3DShaderGroup m_sphereShaderGrp;

  ZIntParameter m_sphereSlicesStacks;
  ZBoolParameter m_useDynamicMaterial;

private:
  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_specularAndShininess;
  std::vector<glm::vec4> m_pointColors;
  std::vector<glm::vec4> m_pointPickingColors;
  std::vector<GLfloat> m_allFlags;
  std::vector<GLuint> m_indexs;

  //std::vector<GLuint> m_VBOs;
  //std::vector<GLuint> m_pickingVBOs;
  ZVertexArrayObject m_VAOs;
  ZVertexArrayObject m_pickingVAOs;
  std::vector<ZVertexBufferObject> m_VBOs;
  std::vector<ZVertexBufferObject> m_pickingVBOs;
  bool m_dataChanged;
  bool m_pickingDataChanged;
  size_t m_oneBatchNumber;
};

#endif // Z3DSPHERERENDERER_H
