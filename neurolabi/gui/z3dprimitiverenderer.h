#ifndef Z3DPRIMITIVERENDERER_H
#define Z3DPRIMITIVERENDERER_H

#include "z3drendererbase.h"
#include "z3dshadergroup.h"
#include "zvertexarrayobject.h"
#include "zvertexbufferobject.h"
#include <QObject>

class ZMesh;

class Z3DPrimitiveRenderer : public QObject
{
Q_OBJECT
public:
  explicit Z3DPrimitiveRenderer(Z3DRendererBase& rendererBase);

  virtual ~Z3DPrimitiveRenderer();

  inline QString className() const
  { return metaObject()->className(); }

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  // for opengl mode only, if set, display list will be build in opengl mode.
  // for large amount of objects, display list can render faster but it is expensive to build.
  // not necessary if number of objects is small (can be rendered in a few opengl calls)
  // default is false, subclass should call this function if needed
  void setUseDisplayList(bool v)
  { m_useDisplayList = v; }

  inline bool useDisplayList()
  { return m_useDisplayList; }
#endif

  // If set, lighting will be enabled.
  // default is true, subclass should call this function if needed
  void setNeedLighting(bool v)
  { m_needLighting = v; }

  inline bool needLighting()
  { return m_needLighting; }

  //sometimes z scale transfrom is not appropriate, for example: bound box. we need to disable it and
  // precalc the correct location. Default is true
  void setFollowCoordTransform(bool v)
  { m_followCoordTransform = v; }

  //
  void setFollowOpacity(bool v)
  { m_followOpacity = v; }

  //
  void setFollowSizeScale(bool v)
  { m_followSizeScale = v; }

  inline glm::mat4 coordTransform() const
  {
    if (m_followCoordTransform)
      return m_rendererBase.coordTransform();
    else
      return glm::mat4(1.f);
  }

  /*
  void setRenderPickingOnly(bool on) {
    m_renderPickingOnly = on;
  }
  bool renderingPickingOnly() const {
    return m_renderPickingOnly;
  }
  */

  // commonly used render functions
  // Render a screen-aligned quad (whole screen)
  static void renderScreenQuad(const ZVertexArrayObject& vao, const Z3DShaderProgram& shader);

  // render a trianglelist with whatever it contains
  static void renderTriangleList(const ZVertexArrayObject& vao, const Z3DShaderProgram& shader, const ZMesh& mesh);

signals:

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  void openglRendererInvalid();
  void openglPickingRendererInvalid();
#endif

protected:
  virtual void compile() = 0;

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  void invalidateOpenglRenderer();
  void invalidateOpenglPickingRenderer();
#endif

  friend class Z3DRendererBase;

  void setShaderParameters(Z3DShaderProgram& shader);

  void setPickingShaderParameters(Z3DShaderProgram& shader);

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  virtual void renderUsingOpengl() {}
  virtual void renderPickingUsingOpengl() {}
#endif

  virtual void render(Z3DEye) = 0;

  virtual void renderPicking(Z3DEye /*unused*/)
  {}

protected:
  Z3DRendererBase& m_rendererBase;
  bool m_needLighting;
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  bool m_useDisplayList;
#endif
  bool m_followCoordTransform;
  bool m_followOpacity;
  bool m_followSizeScale;

  bool m_hardwareSupportVAO;
//  bool m_renderPickingOnly = false;
};

#endif // Z3DPRIMITIVERENDERER_H
