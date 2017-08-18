#include "z3drendererbase.h"

#include "z3dgl.h"
#include "z3dprimitiverenderer.h"
#include "z3dgpuinfo.h"
#include "z3dshaderprogram.h"

Z3DRendererBase::Z3DRendererBase(Z3DGlobalParameters& globalParas, QObject* parent)
  : QObject(parent)
  , m_globalParas(globalParas)
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  , m_displayList(0)
  , m_pickingDisplayList(0)
#endif
  , m_coordTransform("Coord Transform", glm::mat4(1.f))
  //, m_coordXScale("X Scale", 1.0f, 0.1f, 50.f)
  //, m_coordYScale("Y Scale", 1.0f, 0.1f, 50.f)
  //, m_coordZScale("Z Scale", 1.0f, 0.1f, 50.f)
  , m_renderMethod("Rendering Method")
  , m_sizeScale("Size Scale", 1.f, .01f, std::numeric_limits<float>::max())
  , m_opacity("Opacity", 1.0f, .0f, 1.f)
  , m_filterNotFrontFacing("Filter Not Front Facing", true)
  , m_materialAmbient("Material Ambient", glm::vec4(0.1f, .1f, .1f, 1.f))
  , m_materialSpecular("Material Specular", glm::vec4(1.f, 1.f, 1.f, 1.f))
  , m_materialShininess("Material Shininess", 100.f, 1.f, 200.f)
  , m_hasCustomCamera(false)
  , m_viewport(-1)
  , m_clipEnabled(true)
  , m_shaderHookType(ShaderHookType::Normal)
{
  m_renderMethod.addOptions("GLSL", "Old openGL");
  m_renderMethod.select("GLSL");

  m_sizeScale.setSingleStep(0.1);
  m_sizeScale.setDecimal(1);
  m_sizeScale.setStyle("SPINBOX");

  addParameter(m_coordTransform);
  addParameter(m_sizeScale);
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  addParameter(m_renderMethod);
#endif
  addParameter(m_opacity);

  m_materialAmbient.setStyle("COLOR");
  m_materialSpecular.setStyle("COLOR");
  addParameter(m_materialAmbient);
  addParameter(m_materialSpecular);
  addParameter(m_materialShininess);

  connect(&m_globalParas.lightCount, &ZIntParameter::valueChanged, this, &Z3DRendererBase::compile);
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  connect(&m_globalParas.lightCount, &ZIntParameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
#endif

  connect(&m_coordTransform, &Z3DTransformParameter::valueChanged, this,
          &Z3DRendererBase::makeCoordTransformNormalMatrix);
  connect(&m_coordTransform, &Z3DTransformParameter::valueChanged, this, &Z3DRendererBase::coordTransformChanged);
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  connect(&m_coordTransform, &Z3DTransformParameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
  connect(&m_coordTransform, &Z3DTransformParameter::valueChanged, this, &Z3DRendererBase::invalidatePickingDisplayList);
#endif
  connect(&m_sizeScale, &ZFloatParameter::valueChanged, this, &Z3DRendererBase::sizeScaleChanged);
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  connect(&m_sizeScale, &ZFloatParameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
  connect(&m_sizeScale, &ZFloatParameter::valueChanged, this, &Z3DRendererBase::invalidatePickingDisplayList);
  connect(&m_opacity, &ZFloatParameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
  connect(&m_materialShininess, &ZFloatParameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
  connect(&m_materialSpecular, &ZVec4Parameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);

  for (size_t i=0; i<m_globalParas.lightPositions.size(); ++i) {
    connect(m_globalParas.lightPositions[i].get(), &ZVec4Parameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
    connect(m_globalParas.lightAmbients[i].get(), &ZVec4Parameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
    connect(m_globalParas.lightDiffuses[i].get(), &ZVec4Parameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
    connect(m_globalParas.lightSpeculars[i].get(), &ZVec4Parameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
    connect(m_globalParas.lightAttenuations[i].get(), &ZVec3Parameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
    connect(m_globalParas.lightSpotCutoff[i].get(), &ZFloatParameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
    connect(m_globalParas.lightSpotExponent[i].get(), &ZFloatParameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
    connect(m_globalParas.lightSpotDirection[i].get(), &ZVec3Parameter::valueChanged, this, &Z3DRendererBase::invalidateDisplayList);
  }
#endif

  // fog
  connect(&m_globalParas.fogMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DRendererBase::compile);

  makeCoordTransformNormalMatrix();
  connect(&m_globalParas.camera, &Z3DCameraParameter::valueChanged, this,
          &Z3DRendererBase::makeCoordTransformNormalMatrix);
}

Z3DRendererBase::~Z3DRendererBase()
{
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  if ((bool)glIsList(m_displayList))
    glDeleteLists(m_displayList, 1);
  if ((bool)glIsList(m_pickingDisplayList))
    glDeleteLists(m_pickingDisplayList, 1);
#endif
}

void Z3DRendererBase::setGlobalShaderParameters(Z3DShaderProgram& shader, Z3DEye eye)
{
  shader.setScreenDimUniform(glm::vec2(m_viewport.z, m_viewport.w));
  shader.setScreenDimRCPUniform(1.f / glm::vec2(m_viewport.z, m_viewport.w));
  shader.setCameraPositionUniform(camera().eye());
  shader.setViewMatrixUniform(camera().viewMatrix(eye));
  shader.setViewMatrixInverseUniform(camera().inverseViewMatrix(eye));
  shader.setProjectionMatrixUniform(camera().projectionMatrix(eye));
  shader.setProjectionMatrixInverseUniform(camera().inverseProjectionMatrix(eye));
  shader.setNormalMatrixUniform(camera().normalMatrix(eye));
  shader.setViewportMatrixUniform(m_viewportMatrix);
  shader.setViewportMatrixInverseUniform(m_inverseViewportMatrix);
  shader.setProjectionViewMatrixUniform(camera().projectionViewMatrix(eye));

  shader.setGammaUniform(2.f);

  shader.setSizeScaleUniform(m_sizeScale.get());
  shader.setPosTransformUniform(m_coordTransform.get());
  shader.setPosTransformNormalMatrixUniform(m_coordTransformNormalMatrices[enumToUnderlyingType(eye)]);

  shader.setLightsPositionUniform(m_globalParas.lightPositionArray(), m_globalParas.lightCount.get());
  shader.setLightsAmbientUniform(m_globalParas.lightAmbientArray(), m_globalParas.lightCount.get());
  shader.setLightsDiffuseUniform(m_globalParas.lightDiffuseArray(), m_globalParas.lightCount.get());
  shader.setLightsSpecularUniform(m_globalParas.lightSpecularArray(), m_globalParas.lightCount.get());
  shader.setLightsSpotCutoffUniform(m_globalParas.lightSpotCutoffArray(), m_globalParas.lightCount.get());
  shader.setLightsAttenuationUniform(m_globalParas.lightAttenuationArray(), m_globalParas.lightCount.get());
  shader.setLightsSpotExponentUniform(m_globalParas.lightSpotExponentArray(), m_globalParas.lightCount.get());
  shader.setLightsSpotDirectionUniform(m_globalParas.lightSpotDirectionArray(), m_globalParas.lightCount.get());

  shader.setMaterialSpecularUniform(m_materialSpecular.get());
  shader.setMaterialShininessUniform(m_materialShininess.get());
  shader.setMaterialAmbientUniform(m_materialAmbient.get());
  shader.setOrthoUniform(camera().isPerspectiveProjection() ? 0.f : 1.f);
  shader.setSceneAmbientUniform(m_globalParas.sceneAmbient.get());
  shader.setAlphaUniform(m_opacity.get());

  if (!m_globalParas.fogMode.isSelected("None")) {
    shader.setFogColorTopUniform(m_globalParas.fogTopColor.get());
    shader.setFogColorBottomUniform(m_globalParas.fogBottomColor.get());
  }
  // #define M_LOG2E     1.44269504088896340735992468100189214   /* log2(e)        */
  if (m_globalParas.fogMode.isSelected("Linear")) {
    shader.setFogEndUniform(static_cast<GLfloat>((m_globalParas.fogRange.get().y)));
    shader.setFogScaleUniform(
      static_cast<GLfloat>(1.f / (m_globalParas.fogRange.get().y - m_globalParas.fogRange.get().x)));
  } else if (m_globalParas.fogMode.isSelected("Exponential")) {
    shader.setFogDensityLog2eUniform(m_globalParas.fogDensity.get() * 1.44269504088896340735992468100189214f);
  } else if (m_globalParas.fogMode.isSelected("Squared Exponential")) {
    shader.setFogDensityDensityLog2eUniform(
      m_globalParas.fogDensity.get() * m_globalParas.fogDensity.get() * 1.44269504088896340735992468100189214f);
  }

  shader.setClipPlanesUniform(m_clipPlanes.data(), m_clipPlanes.size());
}

void Z3DRendererBase::setGlobalShaderParameters(Z3DShaderProgram* shader, Z3DEye eye)
{
  setGlobalShaderParameters(*shader, eye);
}

QString Z3DRendererBase::generateHeader() const
{
  QString glslVer = QString("%1%2").arg(Z3DGpuInfo::instance().glslMajorVersion()).arg(
    Z3DGpuInfo::instance().glslMinorVersion());
  if (glslVer.length() < 3) {
    glslVer += "0";
  }

  QString header = QString("#version %1\n").arg(glslVer);

  header += "#define lowp\n#define mediump\n#define highp\n";
  //header += "#ifndef GL_FRAGMENT_PRECISION_HIGH\n#define highp mediump\n#endif\n";

  header += QString("#define GLSL_VERSION %1\n").arg(glslVer);

  header += QString("#define LIGHT_COUNT %1\n").arg(m_globalParas.lightCount.get());

  if (!m_clipPlanes.empty()) {
    header += QString("#define HAS_CLIP_PLANE\n");
  }
  if (GLVersionGE(3, 0))
    header += QString("#define CLIP_PLANE_COUNT %1\n").arg(m_clipPlanes.size());

  if (m_globalParas.fogMode.isSelected("Linear")) {
    header += "#define USE_LINEAR_FOG\n";
  } else if (m_globalParas.fogMode.isSelected("Exponential")) {
    header += "#define USE_EXPONENTIAL_FOG\n";
  } else if (m_globalParas.fogMode.isSelected("Squared Exponential")) {
    header += "#define USE_SQUARED_EXPONENTIAL_FOG\n";
  }

  return header;
}

QString Z3DRendererBase::generateGeomHeader() const
{
  QString glslVer = QString("%1%2").arg(Z3DGpuInfo::instance().glslMajorVersion()).arg(
    Z3DGpuInfo::instance().glslMinorVersion());
  if (glslVer.length() < 3) {
    glslVer += "0";
  }

  QString header = QString("#version %1\n").arg(glslVer);

  if (!GLVersionGE(3, 2)) {
    header += QString("#extension GL_EXT_geometry_shader4 : enable\n");
  }

  header += QString("#define GLSL_VERSION %1\n").arg(glslVer);

  return header;
}

void Z3DRendererBase::registerRenderer(Z3DPrimitiveRenderer* renderer)
{
  CHECK(renderer && m_renderers.find(renderer) == m_renderers.end());

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  connect(renderer, &Z3DPrimitiveRenderer::openglRendererInvalid, this, &Z3DRendererBase::invalidateDisplayList);
  connect(renderer, &Z3DPrimitiveRenderer::openglPickingRendererInvalid, this, &Z3DRendererBase::invalidatePickingDisplayList);
#endif

  m_renderers.insert(renderer);
}

void Z3DRendererBase::unregisterRenderer(Z3DPrimitiveRenderer* renderer)
{
  CHECK(renderer && m_renderers.find(renderer) != m_renderers.end());

  m_renderers.erase(renderer);
}

void Z3DRendererBase::setClipPlanes(std::vector<glm::vec4>* clipPlanes)
{
  size_t nOldClipPlanes = m_clipPlanes.size();
  m_clipPlanes.clear();
  m_doubleClipPlanes.clear();
  if (clipPlanes) {
    glm::mat4 itCoordTrans = glm::inverse(glm::transpose(m_coordTransform.get()));
    for (size_t i = 0; i < clipPlanes->size(); ++i) {
      m_clipPlanes.push_back(itCoordTrans * (*clipPlanes)[i]);
    }
  }
  size_t nNewClipPlanes = m_clipPlanes.size();
  if (nNewClipPlanes != nOldClipPlanes)  // need to recompile shader to define or undefine HAS_CLIP_PLANE
    compile();
  for (size_t i = 0; i < m_clipPlanes.size(); ++i) {
    m_doubleClipPlanes.emplace_back(m_clipPlanes[i]);
  }
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  invalidateDisplayList();
  invalidatePickingDisplayList();
#endif
}

void Z3DRendererBase::render(Z3DEye eye, Z3DPrimitiveRenderer* renderer)
{
  CHECK(m_renderers.find(renderer) != m_renderers.end());
  std::vector<Z3DPrimitiveRenderer*> renderers;
  renderers.push_back(renderer);
  render(eye, renderers);
}

void Z3DRendererBase::render(Z3DEye eye, Z3DPrimitiveRenderer* renderer1, Z3DPrimitiveRenderer* renderer2)
{
  CHECK(m_renderers.find(renderer1) != m_renderers.end());
  CHECK(m_renderers.find(renderer2) != m_renderers.end());
  std::vector<Z3DPrimitiveRenderer*> renderers;
  renderers.push_back(renderer1);
  renderers.push_back(renderer2);
  render(eye, renderers);
}

void Z3DRendererBase::render(Z3DEye eye, Z3DPrimitiveRenderer* renderer1, Z3DPrimitiveRenderer* renderer2,
                             Z3DPrimitiveRenderer* renderer3)
{
  CHECK(m_renderers.find(renderer1) != m_renderers.end());
  CHECK(m_renderers.find(renderer2) != m_renderers.end());
  CHECK(m_renderers.find(renderer3) != m_renderers.end());
  std::vector<Z3DPrimitiveRenderer*> renderers;
  renderers.push_back(renderer1);
  renderers.push_back(renderer2);
  renderers.push_back(renderer3);
  render(eye, renderers);
}

void Z3DRendererBase::render(Z3DEye eye, Z3DPrimitiveRenderer* renderer1, Z3DPrimitiveRenderer* renderer2,
                             Z3DPrimitiveRenderer* renderer3, Z3DPrimitiveRenderer* renderer4)
{
  CHECK(m_renderers.find(renderer1) != m_renderers.end());
  CHECK(m_renderers.find(renderer2) != m_renderers.end());
  CHECK(m_renderers.find(renderer3) != m_renderers.end());
  CHECK(m_renderers.find(renderer4) != m_renderers.end());
  std::vector<Z3DPrimitiveRenderer*> renderers;
  renderers.push_back(renderer1);
  renderers.push_back(renderer2);
  renderers.push_back(renderer3);
  renderers.push_back(renderer4);
  render(eye, renderers);
}

void Z3DRendererBase::render(Z3DEye eye, const std::vector<Z3DPrimitiveRenderer*>& renderers)
{
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  if (m_renderMethod.isSelected("Old openGL")) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(getProjectionMatrix(eye)));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(getViewMatrix(eye)));

    if (!useDisplayList()) {
      renderInstant();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      return;
    }

    // check if render state changed and we need to regenerate
    // display list
    if (m_displayList != 0 &&
        m_lastOpenglRenderingState != m_renderers) {
      invalidateDisplayList();
    }

    if (m_displayList == 0) {
      generateDisplayList(renderers);
    }

    if (glIsList(m_displayList)) {
      glCallList(m_displayList);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  } else {
    renderUsingGLSL(eye, renderers);
  }
#else
  renderUsingGLSL(eye, renderers);
#endif
}

void Z3DRendererBase::renderPicking(Z3DEye eye, Z3DPrimitiveRenderer* renderer)
{
  CHECK(m_renderers.find(renderer) != m_renderers.end());
  std::vector<Z3DPrimitiveRenderer*> renderers;
  renderers.push_back(renderer);
  renderPicking(eye, renderers);
}

void
Z3DRendererBase::renderPicking(Z3DEye eye, Z3DPrimitiveRenderer* renderer1, Z3DPrimitiveRenderer* renderer2)
{
  CHECK(m_renderers.find(renderer1) != m_renderers.end());
  CHECK(m_renderers.find(renderer2) != m_renderers.end());
  std::vector<Z3DPrimitiveRenderer*> renderers;
  renderers.push_back(renderer1);
  renderers.push_back(renderer2);
  renderPicking(eye, renderers);
}

void Z3DRendererBase::renderPicking(Z3DEye eye, Z3DPrimitiveRenderer* renderer1, Z3DPrimitiveRenderer* renderer2,
                                    Z3DPrimitiveRenderer* renderer3)
{
  CHECK(m_renderers.find(renderer1) != m_renderers.end());
  CHECK(m_renderers.find(renderer2) != m_renderers.end());
  CHECK(m_renderers.find(renderer3) != m_renderers.end());
  std::vector<Z3DPrimitiveRenderer*> renderers;
  renderers.push_back(renderer1);
  renderers.push_back(renderer2);
  renderers.push_back(renderer3);
  renderPicking(eye, renderers);
}

void Z3DRendererBase::renderPicking(Z3DEye eye, const std::vector<Z3DPrimitiveRenderer*>& renderers)
{
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  if (m_renderMethod.isSelected("Old openGL")) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(getProjectionMatrix(eye)));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(getViewMatrix(eye)));

    if (!useDisplayList()) {
      renderPickingInstant();
      return;
    }

    // check if render state changed and we need to regenerate
    // display list
    if (m_pickingDisplayList != 0 &&
        m_lastOpenglPickingRenderingState != m_renderers) {
      invalidatePickingDisplayList();
    }

    if (m_pickingDisplayList == 0) {
      generatePickingDisplayList(renderers);
    }

    // render display list
    if (glIsList(m_pickingDisplayList)) {
      glCallList(m_pickingDisplayList);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  } else {
    renderPickingUsingGLSL(eye, renderers);
  }
#else
  renderPickingUsingGLSL(eye, renderers);
#endif
}

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
void Z3DRendererBase::generateDisplayList(const std::vector<Z3DPrimitiveRenderer *> &renderers)
{
  if ((bool)glIsList(m_displayList))
    glDeleteLists(m_displayList, 1);

  m_displayList = glGenLists(1);
  glNewList(m_displayList, GL_COMPILE);
  renderInstant(renderers);
  glEndList();
}

void Z3DRendererBase::generatePickingDisplayList(const std::vector<Z3DPrimitiveRenderer *> &renderers)
{
  if ((bool)glIsList(m_pickingDisplayList))
    glDeleteLists(m_pickingDisplayList, 1);

  m_pickingDisplayList = glGenLists(1);
  glNewList(m_pickingDisplayList, GL_COMPILE);
  renderPickingInstant(renderers);
  glEndList();
}

void Z3DRendererBase::renderInstant(const std::vector<Z3DPrimitiveRenderer *> &renderers)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  if (needLighting(renderers)) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, glm::value_ptr(m_globalParas.lightAmbients[0]->get()));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, glm::value_ptr(m_globalParas.lightDiffuses[0]->get()));
    glLightfv(GL_LIGHT0, GL_SPECULAR, glm::value_ptr(m_globalParas.lightSpeculars[0]->get()));
    glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(m_globalParas.lightPositions[0]->get()));
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, glm::value_ptr(m_globalParas.lightSpotDirection[0]->get()));
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, m_globalParas.lightSpotExponent[0]->get());
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, m_globalParas.lightSpotCutoff[0]->get());
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, m_globalParas.lightAttenuations[0]->get().x);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, m_globalParas.lightAttenuations[0]->get().y);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, m_globalParas.lightAttenuations[0]->get().z);

    if (m_globalParas.lightCount.get() > 1) {
      glEnable(GL_LIGHT1);
      glLightfv(GL_LIGHT1, GL_AMBIENT, glm::value_ptr(m_globalParas.lightAmbients[1]->get()));
      glLightfv(GL_LIGHT1, GL_DIFFUSE, glm::value_ptr(m_globalParas.lightDiffuses[1]->get()));
      glLightfv(GL_LIGHT1, GL_SPECULAR, glm::value_ptr(m_globalParas.lightSpeculars[1]->get()));
      glLightfv(GL_LIGHT1, GL_POSITION, glm::value_ptr(m_globalParas.lightPositions[1]->get()));
      glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, glm::value_ptr(m_globalParas.lightSpotDirection[1]->get()));
      glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, m_globalParas.lightSpotExponent[1]->get());
      glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, m_globalParas.lightSpotCutoff[1]->get());
      glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, m_globalParas.lightAttenuations[1]->get().x);
      glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, m_globalParas.lightAttenuations[1]->get().y);
      glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, m_globalParas.lightAttenuations[1]->get().z);
    }

    if (m_globalParas.lightCount.get() > 2) {
      glEnable(GL_LIGHT2);
      glLightfv(GL_LIGHT2, GL_AMBIENT, glm::value_ptr(m_globalParas.lightAmbients[2]->get()));
      glLightfv(GL_LIGHT2, GL_DIFFUSE, glm::value_ptr(m_globalParas.lightDiffuses[2]->get()));
      glLightfv(GL_LIGHT2, GL_SPECULAR, glm::value_ptr(m_globalParas.lightSpeculars[2]->get()));
      glLightfv(GL_LIGHT2, GL_POSITION, glm::value_ptr(m_globalParas.lightPositions[2]->get()));
      glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, glm::value_ptr(m_globalParas.lightSpotDirection[2]->get()));
      glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, m_globalParas.lightSpotExponent[2]->get());
      glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, m_globalParas.lightSpotCutoff[2]->get());
      glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, m_globalParas.lightAttenuations[2]->get().x);
      glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, m_globalParas.lightAttenuations[2]->get().y);
      glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, m_globalParas.lightAttenuations[2]->get().z);
    }

    if (m_globalParas.lightCount.get() > 3) {
      glEnable(GL_LIGHT3);
      glLightfv(GL_LIGHT3, GL_AMBIENT, glm::value_ptr(m_globalParas.lightAmbients[3]->get()));
      glLightfv(GL_LIGHT3, GL_DIFFUSE, glm::value_ptr(m_globalParas.lightDiffuses[3]->get()));
      glLightfv(GL_LIGHT3, GL_SPECULAR, glm::value_ptr(m_globalParas.lightSpeculars[3]->get()));
      glLightfv(GL_LIGHT3, GL_POSITION, glm::value_ptr(m_globalParas.lightPositions[3]->get()));
      glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, glm::value_ptr(m_globalParas.lightSpotDirection[3]->get()));
      glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, m_globalParas.lightSpotExponent[3]->get());
      glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, m_globalParas.lightSpotCutoff[3]->get());
      glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, m_globalParas.lightAttenuations[3]->get().x);
      glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, m_globalParas.lightAttenuations[3]->get().y);
      glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, m_globalParas.lightAttenuations[3]->get().z);
    }

    if (m_globalParas.lightCount.get() > 4) {
      glEnable(GL_LIGHT4);
      glLightfv(GL_LIGHT4, GL_AMBIENT, glm::value_ptr(m_globalParas.lightAmbients[4]->get()));
      glLightfv(GL_LIGHT4, GL_DIFFUSE, glm::value_ptr(m_globalParas.lightDiffuses[4]->get()));
      glLightfv(GL_LIGHT4, GL_SPECULAR, glm::value_ptr(m_globalParas.lightSpeculars[4]->get()));
      glLightfv(GL_LIGHT4, GL_POSITION, glm::value_ptr(m_globalParas.lightPositions[4]->get()));
      glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, glm::value_ptr(m_globalParas.lightSpotDirection[4]->get()));
      glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, m_globalParas.lightSpotExponent[4]->get());
      glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, m_globalParas.lightSpotCutoff[4]->get());
      glLightf(GL_LIGHT4, GL_CONSTANT_ATTENUATION, m_globalParas.lightAttenuations[4]->get().x);
      glLightf(GL_LIGHT4, GL_LINEAR_ATTENUATION, m_globalParas.lightAttenuations[4]->get().y);
      glLightf(GL_LIGHT4, GL_QUADRATIC_ATTENUATION, m_globalParas.lightAttenuations[4]->get().z);
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(m_materialAmbient.get()));
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, std::min(m_materialShininess.get(), 128.f));
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glm::value_ptr(m_materialSpecular.get()));
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
    glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    glPopMatrix();
  }

  activateClipPlanesOpenGL();
  for (size_t i=0; i<renderers.size(); ++i) {
    renderers[i]->renderUsingOpengl();
  }
  deactivateClipPlanesOpenGL();

  glPopAttrib();
}

void Z3DRendererBase::renderPickingInstant(const std::vector<Z3DPrimitiveRenderer *> &renderers)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  activateClipPlanesOpenGL();
  for (size_t i=0; i<renderers.size(); ++i) {
    renderers[i]->renderPickingUsingOpengl();
  }
  deactivateClipPlanesOpenGL();

  glPopAttrib();
}
#endif

void Z3DRendererBase::renderUsingGLSL(Z3DEye eye, const std::vector<Z3DPrimitiveRenderer*>& renderers)
{
  activateClipPlanesGLSL();
  for (size_t i = 0; i < renderers.size(); ++i) {
    renderers[i]->render(eye);
  }
  deactivateClipPlanesGLSL();
}

void Z3DRendererBase::renderPickingUsingGLSL(Z3DEye eye, const std::vector<Z3DPrimitiveRenderer*>& renderers)
{
  activateClipPlanesGLSL();
  for (size_t i = 0; i < renderers.size(); ++i) {
    renderers[i]->renderPicking(eye);
  }
  deactivateClipPlanesGLSL();
}

bool Z3DRendererBase::needLighting(const std::vector<Z3DPrimitiveRenderer*>& renderers) const
{
  bool needLighting = false;
  for (size_t i = 0; i < renderers.size(); ++i) {
    needLighting = needLighting || renderers[i]->needLighting();
  }
  return needLighting;
}

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
bool Z3DRendererBase::useDisplayList(const std::vector<Z3DPrimitiveRenderer*> &renderers) const
{
  bool useDisplayList = false;
  for (size_t i=0; i<renderers.size(); ++i) {
    useDisplayList = useDisplayList || renderers[i]->useDisplayList();
  }
  return useDisplayList;
}
#endif

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
void Z3DRendererBase::activateClipPlanesOpenGL()
{
  if (!m_clipEnabled)
    return;
  for (size_t i=0; i<m_clipPlanes.size(); ++i) {
    glClipPlane(GL_CLIP_PLANE0+i, glm::value_ptr(m_doubleClipPlanes[i]));
    glEnable(GL_CLIP_PLANE0+i);
  }
}

void Z3DRendererBase::deactivateClipPlanesOpenGL()
{
  if (!m_clipEnabled)
    return;
  for (size_t i=0; i<m_clipPlanes.size(); ++i) {
    glDisable(GL_CLIP_PLANE0+i);
  }
}
#endif

void Z3DRendererBase::activateClipPlanesGLSL()
{
  if (!m_clipEnabled)
    return;
  for (size_t i = 0; i < m_clipPlanes.size(); ++i) {
    if (GLVersionGE(3, 0)) {
      glEnable(GL_CLIP_DISTANCE0 + i);
    } else {
      glClipPlane(GL_CLIP_PLANE0 + i, glm::value_ptr(m_doubleClipPlanes[i]));
      glEnable(GL_CLIP_PLANE0 + i);
    }
  }
}

void Z3DRendererBase::deactivateClipPlanesGLSL()
{
  if (!m_clipEnabled)
    return;
  for (size_t i = 0; i < m_clipPlanes.size(); ++i) {
    if (GLVersionGE(3, 0)) {
      glDisable(GL_CLIP_DISTANCE0 + i);
    } else {
      glDisable(GL_CLIP_PLANE0 + i);
    }
  }
}

void Z3DRendererBase::makeViewportMatrix()
{
#if 0
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  float l = viewport[0];
  float b = viewport[1];
  float r = viewport[2] + l;
  float t = viewport[3] + b;
  GLfloat depthrange[2];
  glGetFloatv(GL_DEPTH_RANGE, depthrange);
  float n = depthrange[0];
  float f = depthrange[1];
#else
  float l = m_viewport[0];
  float b = m_viewport[1];
  float r = m_viewport[2] + l;
  float t = m_viewport[3] + b;
  float n = 0;
  float f = 1;
#endif
  m_viewportMatrix = glm::mat4(
    glm::vec4((r - l) / 2.f, 0.0f, 0.0f, 0.0f),
    glm::vec4(0.0f, (t - b) / 2.f, 0.0f, 0.0f),
    glm::vec4(0.0f, 0.0f, (f - n) / 2.f, 0.0f),
    glm::vec4((r + l) / 2.f, (t + b) / 2.f, (f + n) / 2.f, 1.0f)
  );
  m_inverseViewportMatrix = glm::inverse(m_viewportMatrix);
}

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
void Z3DRendererBase::invalidateDisplayList()
{
  if ((bool)glIsList(m_displayList)) {
    glDeleteLists(m_displayList, 1);
  }
  m_displayList = 0;
}

void Z3DRendererBase::invalidatePickingDisplayList()
{
  if ((bool)glIsList(m_pickingDisplayList)) {
    glDeleteLists(m_pickingDisplayList, 1);
  }
  m_pickingDisplayList = 0;
}
#endif

void Z3DRendererBase::compile()
{
  for (auto renderer : m_renderers) {
    renderer->compile();
  }
}

void Z3DRendererBase::makeCoordTransformNormalMatrix()
{
  m_coordTransformNormalMatrices[enumToUnderlyingType(Z3DEye::Left)] = glm::transpose(
    glm::inverse(glm::mat3(camera().viewMatrix(Z3DEye::Left) * m_coordTransform.get())));
  m_coordTransformNormalMatrices[enumToUnderlyingType(Z3DEye::Mono)] = glm::transpose(
    glm::inverse(glm::mat3(camera().viewMatrix(Z3DEye::Mono) * m_coordTransform.get())));
  m_coordTransformNormalMatrices[enumToUnderlyingType(Z3DEye::Right)] = glm::transpose(
    glm::inverse(glm::mat3(camera().viewMatrix(Z3DEye::Right) * m_coordTransform.get())));
}


