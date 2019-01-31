#ifndef Z3DRENDERERBASE_H
#define Z3DRENDERERBASE_H

#include "z3dtransformparameter.h"
#include "widgets/znumericparameter.h"
#include "widgets/zoptionparameter.h"
#include "z3dcamera.h"
#include "z3dglobalparameters.h"
#include <QObject>
#include <set>
#include <vector>

class Z3DPrimitiveRenderer;

class Z3DTexture;

class Z3DShaderProgram;

// contains basic properties such as lighting, method, size for rendering.
// A renderBase usually contains multiple primitive renderers. Some of those are
// combined to draw a complicated object. Some of those are just sharing the enviroment
// (rendering parameters).
class Z3DRendererBase : public QObject
{
  Q_OBJECT
public:
  enum class ShaderHookType
  {
    Normal, DualDepthPeelingInit, DualDepthPeelingPeel, WeightedAverageInit, WeightedBlendedInit
  };

  struct ShaderHookParameter
  {
    const Z3DTexture* dualDepthPeelingDepthBlenderTexture;
    const Z3DTexture* dualDepthPeelingFrontBlenderTexture;
  };

  explicit Z3DRendererBase(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);

  virtual ~Z3DRendererBase();

  inline void setCamera(const Z3DCamera& c)
  {
    m_camera = c;
    m_hasCustomCamera = true;
    makeCoordTransformNormalMatrix();
  }

  inline void unsetCamera()
  { m_hasCustomCamera = false; }

  inline Z3DCamera& camera()
  { return m_hasCustomCamera ? m_camera : m_globalParas.camera.get(); }

  inline Z3DCamera& globalCamera()
  { return m_globalParas.camera.get(); }

  inline Z3DCameraParameter& globalCameraPara()
  { return m_globalParas.camera; }

  inline Z3DGlobalParameters& globalParas()
  { return m_globalParas; }

  inline const Z3DGlobalParameters& globalParas() const
  { return m_globalParas; }

  inline ZStringIntOptionParameter& geometriesMultisampleModePara()
  { return m_globalParas.geometriesMultisampleMode; }

  inline ZStringIntOptionParameter& transparencyMethodPara()
  { return m_globalParas.transparencyMethod; }

  inline void setViewport(const glm::uvec4& viewport)
  {
    if (m_viewport != viewport) {
      m_viewport = viewport;
      makeViewportMatrix();
    }
  }

  inline void setViewport(const glm::uvec2& viewport)
  {
    if (m_viewport.zw() !=viewport) {//[3] != viewport[0]) || (m_viewport[2] != viewport[1])) {
      m_viewport = glm::ivec4(0, 0, viewport);
      makeViewportMatrix();
    }
  }

  inline glm::uvec4 viewport() const
  { return m_viewport; }

  // need valid camera and viewport
  void setGlobalShaderParameters(Z3DShaderProgram& shader, Z3DEye eye);

  void setGlobalShaderParameters(Z3DShaderProgram* shader, Z3DEye eye);

  QString generateHeader() const;

  QString generateGeomHeader() const;

  // renderer's constructor and deconstructor will take care of this
  void registerRenderer(Z3DPrimitiveRenderer* renderer);

  void unregisterRenderer(Z3DPrimitiveRenderer* renderer);

  Z3DTransformParameter& coordTransformPara()
  { return m_coordTransform; }

  const Z3DTransformParameter& coordTransformPara() const
  { return m_coordTransform; }

  ZFloatParameter& opacityPara()
  { return m_opacity; }

  inline void setSizeScale(float s)
  { m_sizeScale.set(s); }

  inline void setXScale(float s)
  { m_coordTransform.setXScale(s); }

  inline void setYScale(float s)
  { m_coordTransform.setYScale(s); }

  inline void setZScale(float s)
  { m_coordTransform.setZScale(s); }

  inline void setScale(float x, float y, float z)
  { m_coordTransform.setScale(glm::vec3(x, y, z)); }

  inline void setOffset(float x, float y, float z)
  { m_coordTransform.translate(x, y, z); }

  inline void setRotationCenter(const glm::vec3& c)
  { m_coordTransform.setCenter(c); }

  inline void setOpacity(float o)
  { m_opacity.set(o); }

  inline void setMaterialSpecular(const glm::vec4& v)
  { m_materialSpecular.set(v); }

  inline void setMaterialAmbient(const glm::vec4& v)
  { m_materialAmbient.set(v); }

  void setClipPlanes(std::vector<glm::vec4>* clipPlanes);

  void setClipEnabled(bool v)
  { m_clipEnabled = v; }

  void addParameter(ZParameter& para)
  { addParameter(&para); }

  void addParameter(ZParameter* para)
  { m_parameters.push_back(para); }

  // parameters of rendererbase
  const std::vector<ZParameter*>& parameters() const
  { return m_parameters; }

  // only global parameters
  const std::vector<ZParameter*>& globalParameters() const
  { return m_globalParas.parameters(); }

  inline glm::mat4 coordTransform() const
  { return m_coordTransform.get(); }

  inline float opacity() const
  { return m_opacity.get(); }

  inline float sizeScale() const
  { return m_sizeScale.get(); }

  void render(Z3DEye eye, Z3DPrimitiveRenderer& renderer)
  { render(eye, &renderer); }

  void render(Z3DEye eye, Z3DPrimitiveRenderer& renderer1, Z3DPrimitiveRenderer& renderer2)
  { render(eye, &renderer1, &renderer2); }

  void render(Z3DEye eye, Z3DPrimitiveRenderer& renderer1, Z3DPrimitiveRenderer& renderer2,
              Z3DPrimitiveRenderer& renderer3)
  { render(eye, &renderer1, &renderer2, &renderer3); }

  void render(Z3DEye eye, Z3DPrimitiveRenderer& renderer1, Z3DPrimitiveRenderer& renderer2,
              Z3DPrimitiveRenderer& renderer3, Z3DPrimitiveRenderer& renderer4)
  { render(eye, &renderer1, &renderer2, &renderer3, &renderer4); }

  void render(Z3DEye eye, Z3DPrimitiveRenderer* renderer);

  void render(Z3DEye eye, Z3DPrimitiveRenderer* renderer1, Z3DPrimitiveRenderer* renderer2);

  void render(Z3DEye eye, Z3DPrimitiveRenderer* renderer1, Z3DPrimitiveRenderer* renderer2,
              Z3DPrimitiveRenderer* renderer3);

  void render(Z3DEye eye, Z3DPrimitiveRenderer* renderer1, Z3DPrimitiveRenderer* renderer2,
              Z3DPrimitiveRenderer* renderer3, Z3DPrimitiveRenderer* renderer4);

  void render(Z3DEye eye, const std::vector<Z3DPrimitiveRenderer*>& renderers);

  void renderPicking(Z3DEye eye, Z3DPrimitiveRenderer& renderer)
  { renderPicking(eye, &renderer); }

  void renderPicking(Z3DEye eye, Z3DPrimitiveRenderer& renderer1, Z3DPrimitiveRenderer& renderer2)
  { renderPicking(eye, &renderer1, &renderer2); }

  void renderPicking(Z3DEye eye, Z3DPrimitiveRenderer& renderer1, Z3DPrimitiveRenderer& renderer2,
                     Z3DPrimitiveRenderer& renderer3)
  { renderPicking(eye, &renderer1, &renderer2, &renderer3); }

  void renderPicking(Z3DEye eye, Z3DPrimitiveRenderer* renderer);

  void renderPicking(Z3DEye eye, Z3DPrimitiveRenderer* renderer1, Z3DPrimitiveRenderer* renderer2);

  void renderPicking(Z3DEye eye, Z3DPrimitiveRenderer* renderer1, Z3DPrimitiveRenderer* renderer2,
                     Z3DPrimitiveRenderer* renderer3);

  void renderPicking(Z3DEye eye, const std::vector<Z3DPrimitiveRenderer*>& renderers);

  inline void setShaderHookType(ShaderHookType t)
  { m_shaderHookType = t; }

  inline ShaderHookType shaderHookType() const
  { return m_shaderHookType; }

  inline ShaderHookParameter& shaderHookPara()
  { return m_shaderHookPara; }

  inline void setShaderHookParaDDPDepthBlenderTexture(const Z3DTexture* t)
  { m_shaderHookPara.dualDepthPeelingDepthBlenderTexture = t; }

  inline void setShaderHookParaDDPFrontBlenderTexture(const Z3DTexture* t)
  { m_shaderHookPara.dualDepthPeelingFrontBlenderTexture = t; }

  inline const glm::mat4& viewportMatrix() const
  { return m_viewportMatrix; }

  inline const glm::mat4& inverseViewportMatrix() const
  { return m_inverseViewportMatrix; }

signals:

  void coordTransformChanged();

  void sizeScaleChanged();
  void opacityChanged(double);

protected:

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  void generateDisplayList(const std::vector<Z3DPrimitiveRenderer *> &renderers);
  void generatePickingDisplayList(const std::vector<Z3DPrimitiveRenderer *> &renderers);

  void renderInstant(const std::vector<Z3DPrimitiveRenderer *> &renderers);
  void renderPickingInstant(const std::vector<Z3DPrimitiveRenderer *> &renderers);
#endif

  void renderUsingGLSL(Z3DEye eye, const std::vector<Z3DPrimitiveRenderer*>& renderers);

  void renderPickingUsingGLSL(Z3DEye eye, const std::vector<Z3DPrimitiveRenderer*>& renderers);

  bool needLighting(const std::vector<Z3DPrimitiveRenderer*>& renderers) const;

  bool useDisplayList(const std::vector<Z3DPrimitiveRenderer*>& renderers) const;

  inline bool hasClipPlanes()
  { return !m_clipPlanes.empty(); }

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  void activateClipPlanesOpenGL();
  void deactivateClipPlanesOpenGL();
#endif

  void activateClipPlanesGLSL();

  void deactivateClipPlanesGLSL();

  void makeViewportMatrix();

private:
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  void invalidateDisplayList();
  void invalidatePickingDisplayList();
#endif

  void compile();

  void makeCoordTransformNormalMatrix();

protected:
  Z3DGlobalParameters& m_globalParas;

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  // display list generated from the geometry.
  GLuint m_displayList;
  GLuint m_pickingDisplayList;
#endif

  Z3DTransformParameter m_coordTransform;
  ZStringIntOptionParameter m_renderMethod;
  glm::mat3 m_coordTransformNormalMatrices[3];

  ZFloatParameter m_sizeScale;
  ZFloatParameter m_opacity;

  ZBoolParameter m_filterNotFrontFacing;

  ZVec4Parameter m_materialAmbient;
  ZVec4Parameter m_materialSpecular;
  ZFloatParameter m_materialShininess;

  bool m_hasCustomCamera;
  Z3DCamera m_camera;
  glm::uvec4 m_viewport;
  glm::mat4 m_viewportMatrix;
  glm::mat4 m_inverseViewportMatrix;

  std::vector<ZParameter*> m_parameters;
  // renderers
  std::set<Z3DPrimitiveRenderer*> m_renderers;

  std::vector<glm::vec4> m_clipPlanes;
  std::vector<glm::dvec4> m_doubleClipPlanes;
  bool m_clipEnabled;

  ShaderHookType m_shaderHookType;
  ShaderHookParameter m_shaderHookPara;

private:
  std::set<Z3DPrimitiveRenderer*>::iterator m_renderersIt;
};

#endif // Z3DRENDERERBASE_H
