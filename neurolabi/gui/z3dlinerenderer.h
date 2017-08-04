#ifndef Z3DLINERENDERER_H
#define Z3DLINERENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dgpuinfo.h"
#include <QApplication>

class Z3DLineRenderer : public Z3DPrimitiveRenderer
{
Q_OBJECT
public:
  // default use display list but not lighing in opengl mode
  explicit Z3DLineRenderer(Z3DRendererBase& rendererBase);

  // default false, must call before setData and setDataColor and setDataPickingColors
  void setLineStrip(bool v)
  { m_isLineStrip = v; }

  void setData(std::vector<glm::vec3>* linesInput);

  void setLineWidth(const std::vector<float>& lineWidthArray)
  {
    m_lineWidthArray = lineWidthArray;
  }

  // use vertice color
  void setDataColors(std::vector<glm::vec4>* lineColorsInput);

  // use 1d texture color
  void setTexture(Z3DTexture* tex);

  void setDataPickingColors(std::vector<glm::vec4>* linePickingColorsInput = nullptr);

  // default true since glLineWidth only support 1 pixel width line from now on
  inline void setUseSmoothLine(bool v)
  { m_useSmoothLine = v; }

  inline void setLineWidth(float v)
  {
    m_srcLineWidth = std::max(1.f, v);
    updateLineWidth();
  }

  inline void setEnableMultisample(bool v)
  {
    m_enableMultisample = v;
    updateLineWidth();
  }

  // default true, will disable screen align
  void setRoundCap(bool v);

  // default false, will disable round cap
  void setScreenAlign(bool v);

protected:
  virtual void compile() override;

  QString generateHeader();

  virtual float lineWidth() const;

  virtual std::vector<glm::vec4>* lineColors();

#ifndef ATLAS_USE_CORE_PROFILE
  virtual void renderUsingOpengl() override;
  virtual void renderPickingUsingOpengl() override;
#endif

  virtual void render(Z3DEye eye) override;

  virtual void renderPicking(Z3DEye eye) override;

  //void enableLineSmooth();
  //void disableLineSmooth();

private:
  void updateLineWidth()
  {
    if (m_enableMultisample && m_rendererBase.geometriesMultisampleModePara().isSelected("2x2"))
      m_lineWidth = (m_srcLineWidth - 0.9) * 2.f;
    else
      m_lineWidth = m_srcLineWidth - 0.9;

    m_lineWidth *= qApp->devicePixelRatio();
  }

  Z3DShaderGroup& currentShaderGrp()
  { return m_useGeomLineShader && m_useSmoothLine ? m_smoothLineShaderGrp : m_lineShaderGrp; }

protected:
  Z3DShaderGroup m_lineShaderGrp;
  Z3DShaderGroup m_smoothLineShaderGrp;
  Z3DShaderGroup m_smoothLineShaderGrp1;

  std::vector<glm::vec3>* m_linesPt;
  std::vector<glm::vec4>* m_lineColorsPt;
  std::vector<glm::vec4>* m_linePickingColorsPt;

  bool m_useSmoothLine;
  float m_srcLineWidth;
  float m_lineWidth;
  bool m_enableMultisample;
  std::vector<float> m_lineWidthArray;

  Z3DTexture* m_texture;

private:
  std::vector<glm::vec4> m_lineColors;

  ZVertexArrayObject m_VAO;
  ZVertexArrayObject m_pickingVAO;
  ZVertexBufferObject m_VBOs;
  ZVertexBufferObject m_pickingVBOs;
  bool m_dataChanged;
  bool m_pickingDataChanged;
  bool m_isLineStrip;

  bool m_useTextureColor;
  bool m_screenAligned;
  bool m_roundCap;

  void renderSmooth(Z3DEye eye);

  void renderSmoothPicking(Z3DEye eye);

  std::vector<glm::vec3> m_smoothLineP0s;
  std::vector<glm::vec3> m_smoothLineP1s;
  std::vector<glm::vec4> m_smoothLineP0Colors;
  std::vector<glm::vec4> m_smoothLineP1Colors;
  std::vector<glm::vec4> m_smoothLinePickingColors;
  std::vector<GLfloat> m_allFlags;
  std::vector<GLuint> m_indexs;

  ZVertexArrayObject m_VAOs;
  ZVertexArrayObject m_pickingVAOs;
  std::vector<ZVertexBufferObject> m_batchVBOs;
  std::vector<ZVertexBufferObject> m_batchPickingVBOs;
  size_t m_oneBatchNumber;
  bool m_useGeomLineShader;
};

#endif // Z3DLINERENDERER_H
