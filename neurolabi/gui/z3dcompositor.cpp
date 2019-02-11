#include "z3dcompositor.h"

#include "z3dgl.h"
#include "z3dgpuinfo.h"
#include "z3drendertarget.h"
#include "z3dtexture.h"
#include "logging/zbenchtimer.h"
#include "z3dpickingmanager.h"

//#define USE_RECT_TEX

Z3DCompositor::Z3DCompositor(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DBoundedFilter(globalParas, parent)
  , m_alphaBlendRenderer(m_rendererBase, "DepthTestBlending")
  , m_firstOnTopBlendRenderer(m_rendererBase, "FirstOnTopBlending")
  , m_firstOnTopRenderer(m_rendererBase, "FirstOnTop")
  , m_textureCopyRenderer(m_rendererBase)
  , m_backgroundRenderer(m_rendererBase)
  //, m_renderGeometries("Render Geometries", true)
  , m_inport("Image", true, this, State::MonoViewResultInvalid)
  , m_leftEyeInport("LeftEyeImage", true, this, State::LeftEyeResultInvalid)
  , m_rightEyeInport("RightEyeImage", true, this, State::RightEyeResultInvalid)
  , m_outport("Image", this)
  , m_leftEyeOutport("LeftEyeImage", this)
  , m_rightEyeOutport("RightEyeImage", this)
  , m_tempPort("ImageTemp", this)
  , m_tempPort2("ImageTemp2", this)
  , m_tempPort3("ImageTemp3", this)
  , m_tempPort4("ImageTemp4", this)
  , m_tempPort5("ImageTemp5", this)
  , m_pickingPort("PickingTarget", this, GLint(GL_RGBA8))
  , m_gPPort("GeometryFilters", true, this)
  , m_vPPort("VolumeFilters", true, this)
  , m_ddpBlendShader()
  , m_ddpFinalShader()
  , m_waFinalShader()
  , m_wbFinalShader()
  , m_showBackground("Show Background", true)
  , m_lineRenderer(m_rendererBase)
  , m_arrowRenderer(m_rendererBase)
  , m_fontRenderer(m_rendererBase)
  , m_showAxis("Show Axis", true)
  , m_XAxisColor("X Axis Color", glm::vec4(1.f, 0.f, 0.f, 1.0f))
  , m_YAxisColor("Y Axis Color", glm::vec4(0.f, 1.f, 0.f, 1.0f))
  , m_ZAxisColor("Z Axis Color", glm::vec4(0.f, 0.f, 1.f, 1.0f))
  , m_axisRegionRatio("Axis Region Ratio", .25f, .1f, 1.f)
  , m_axisMode("Mode")
  , m_screenQuadVAO(1)
  , m_region(0, 1, 0, 1)
{
  addParameter(m_showBackground);

  addPort(m_inport);
  addPort(m_leftEyeInport);
  addPort(m_rightEyeInport);
  addPort(m_outport);
  addPort(m_leftEyeOutport);
  addPort(m_rightEyeOutport);
  addPrivateRenderPort(m_tempPort);
  addPrivateRenderPort(m_tempPort2);
  addPrivateRenderPort(m_tempPort3);
  addPrivateRenderPort(m_tempPort4);
  addPrivateRenderPort(m_tempPort5);
  addPrivateRenderPort(m_pickingPort);
  addPort(m_gPPort);
  addPort(m_vPPort);

  m_textureCopyRenderer.setDiscardTransparent(true);
  addParameter(m_backgroundRenderer.modePara());
  addParameter(m_backgroundRenderer.firstColorPara());
  addParameter(m_backgroundRenderer.secondColorPara());
  addParameter(m_backgroundRenderer.gradientOrientationPara());

  if (Z3DGpuInfo::instance().isWeightedAverageSupported()) {
    m_waFinalShader.bindFragDataLocation(0, "FragData0");
#ifdef USE_RECT_TEX
    m_waFinalShader.loadFromSourceFile("pass.vert", "wavg_final.frag",
                                       m_rendererBase.generateHeader() + "#define USE_RECT_TEX\n");
#else
    m_waFinalShader.loadFromSourceFile("pass.vert", "wavg_final.frag", m_rendererBase.generateHeader());
#endif
  }

  if (Z3DGpuInfo::instance().isWeightedBlendedSupported()) {
    m_wbFinalShader.bindFragDataLocation(0, "FragData0");
#ifdef USE_RECT_TEX
    m_wbFinalShader.loadFromSourceFile("pass.vert", "wblended_final.frag",
                                       m_rendererBase.generateHeader() + "#define USE_RECT_TEX\n");
#else
    m_wbFinalShader.loadFromSourceFile("pass.vert", "wblended_final.frag", m_rendererBase.generateHeader());
#endif
  }

  if (Z3DGpuInfo::instance().isDualDepthPeelingSupported()) {
    m_ddpBlendShader.bindFragDataLocation(0, "FragData0");
#ifdef USE_RECT_TEX
    m_ddpBlendShader.loadFromSourceFile("pass.vert", "dual_peeling_blend.frag",
                                        m_rendererBase.generateHeader() + "#define USE_RECT_TEX\n");
#else
    m_ddpBlendShader.loadFromSourceFile("pass.vert", "dual_peeling_blend.frag", m_rendererBase.generateHeader());
#endif
    m_ddpFinalShader.bindFragDataLocation(0, "FragData0");
#ifdef USE_RECT_TEX
    m_ddpFinalShader.loadFromSourceFile("pass.vert", "dual_peeling_final.frag",
                                        m_rendererBase.generateHeader() + "#define USE_RECT_TEX\n");
#else
    m_ddpFinalShader.loadFromSourceFile("pass.vert", "dual_peeling_final.frag", m_rendererBase.generateHeader());
#endif
  }

  globalParas.setPickingTarget(m_pickingPort.renderTarget());
  addInteractionHandler(globalParas.interactionHandler);

  m_XAxisColor.setStyle("COLOR");
  m_YAxisColor.setStyle("COLOR");
  m_ZAxisColor.setStyle("COLOR");
  m_axisMode.addOptions("Arrow", "Line");
  m_axisMode.select("Arrow");
  addParameter(m_showAxis);
  addParameter(m_XAxisColor);
  addParameter(m_YAxisColor);
  addParameter(m_ZAxisColor);
  addParameter(m_axisRegionRatio);
  addParameter(m_axisMode);
  addParameter(m_fontRenderer.allFontNamesPara());
  addParameter(m_fontRenderer.fontSizePara());
  addParameter(m_fontRenderer.fontSoftEdgeScalePara());
  addParameter(m_fontRenderer.showFontOutlinePara());
  addParameter(m_fontRenderer.fontOutlineModePara());
  addParameter(m_fontRenderer.fontOutlineColorPara());
  addParameter(m_fontRenderer.showFontShadowPara());
  addParameter(m_fontRenderer.fontShadowColorPara());
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  m_arrowRenderer.setUseDisplayList(false);
  m_lineRenderer.setUseDisplayList(false);
#endif
  m_fontRenderer.setFollowCoordTransform(false);
  setupAxisCamera();

//  // ROIs
//  m_wbFinalShader = new Z3DShaderProgram();
//  m_wbFinalShader->bindFragDataLocation(0, "FragData0");
//  m_wbFinalShader->bindFragDataLocation(1, "FragData1");
//  m_wbFinalShader->loadFromSourceFile("cube_wboit_compose.vert", "cube_wboit_compose.frag", m_rendererBase->generateHeader());
}

bool Z3DCompositor::isReady(Z3DEye eye) const
{
  if (eye == Z3DEye::Mono && m_outport.isReady())
    return true;
  if (eye == Z3DEye::Left && m_leftEyeOutport.isReady())
    return true;
  if (eye == Z3DEye::Right && m_rightEyeOutport.isReady())
    return true;

  return false;
}

std::shared_ptr<ZWidgetsGroup> Z3DCompositor::backgroundWidgetsGroup()
{
  if (!m_backgroundWidgetsGroup) {
    m_backgroundWidgetsGroup = std::make_shared<ZWidgetsGroup>("Background", 1);
    m_backgroundWidgetsGroup->addChild(m_showBackground, 1);
    m_backgroundWidgetsGroup->addChild(m_backgroundRenderer.modePara(), 1);
    m_backgroundWidgetsGroup->addChild(m_backgroundRenderer.firstColorPara(), 1);
    m_backgroundWidgetsGroup->addChild(m_backgroundRenderer.secondColorPara(), 1);
    m_backgroundWidgetsGroup->addChild(m_backgroundRenderer.gradientOrientationPara(), 1);
    //m_backgroundWidgetsGroup->setBasicAdvancedCutoff(4);
  }
  return m_backgroundWidgetsGroup;
}

std::shared_ptr<ZWidgetsGroup> Z3DCompositor::axisWidgetsGroup()
{
  if (!m_axisWidgetsGroup) {
    m_axisWidgetsGroup = std::make_shared<ZWidgetsGroup>("Axis", 1);
    m_axisWidgetsGroup->addChild(m_showAxis, 1);
    m_axisWidgetsGroup->addChild(m_axisMode, 1);
    m_axisWidgetsGroup->addChild(m_axisRegionRatio, 1);
    m_axisWidgetsGroup->addChild(m_XAxisColor, 1);
    m_axisWidgetsGroup->addChild(m_YAxisColor, 1);
    m_axisWidgetsGroup->addChild(m_ZAxisColor, 1);
    std::vector<ZParameter*> paras = m_rendererBase.parameters();
    for (auto para : paras) {
      if (para->name() == "Size Scale")
        m_axisWidgetsGroup->addChild(*para, 1);
      else if (para->name() == "Rendering Method")
        m_axisWidgetsGroup->addChild(*para, 3);
      else if (para->name() == "Opacity")
        m_axisWidgetsGroup->addChild(*para, 3);
    }
    m_axisWidgetsGroup->addChild(m_fontRenderer.allFontNamesPara(), 4);
    m_axisWidgetsGroup->addChild(m_fontRenderer.fontSizePara(), 4);
    m_axisWidgetsGroup->addChild(m_fontRenderer.fontSoftEdgeScalePara(), 4);
    m_axisWidgetsGroup->addChild(m_fontRenderer.showFontOutlinePara(), 4);
    m_axisWidgetsGroup->addChild(m_fontRenderer.fontOutlineModePara(), 4);
    m_axisWidgetsGroup->addChild(m_fontRenderer.fontOutlineColorPara(), 4);
    m_axisWidgetsGroup->addChild(m_fontRenderer.showFontShadowPara(), 4);
    m_axisWidgetsGroup->addChild(m_fontRenderer.fontShadowColorPara(), 4);
    //m_axisWidgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_axisWidgetsGroup;
}

void Z3DCompositor::savePickingBufferToImage(const QString& filename)
{
  const Z3DTexture* tex = pickingManager().renderTarget().attachment(GL_COLOR_ATTACHMENT0);
  tex->saveAsColorImage(filename);
}

void Z3DCompositor::setRenderingRegion(double left, double right, double bottom, double top)
{
  m_backgroundRenderer.setRenderingRegion(left, right, bottom, top);
  m_region = glm::vec4(left, right - left, bottom, top - bottom);
}

void Z3DCompositor::process(Z3DEye eye)
{
  std::vector<Z3DGeometryFilter*> filters = m_gPPort.connectedFilters();
  std::vector<Z3DVolumeFilter*> vFilters = m_vPPort.connectedFilters();
  std::vector<Z3DBoundedFilter*> onTopOpaqueFilters;
  std::vector<Z3DBoundedFilter*> onTopTransparentFilters;
  std::vector<Z3DBoundedFilter*> normalOpaqueFilters;
  std::vector<Z3DBoundedFilter*> normalTransparentFilters;
  std::vector<Z3DBoundedFilter*> selectedFilters;
//  std::vector<Z3DBoundedFilter*> showHandleFilters;
  for (auto vFilter : vFilters) {
    if (vFilter->isReady(eye) && vFilter->hasOpaque(eye)) {
      normalOpaqueFilters.push_back(vFilter);
    }
    if (vFilter->isReady(eye)) {
      selectedFilters.push_back(vFilter);
    }
  }
  //if (m_renderGeometries.get()) {
  for (auto geomFilter : filters) {
    if (geomFilter->isReady(eye) && (geomFilter->opacity() > 0.0)) {
      if (geomFilter->hasOpaque(eye)) {
        if (geomFilter->isStayOnTop())
          onTopOpaqueFilters.push_back(geomFilter);
        else
          normalOpaqueFilters.push_back(geomFilter);
      }
      if (geomFilter->hasTransparent(eye)) {
        if (geomFilter->isStayOnTop())
          onTopTransparentFilters.push_back(geomFilter);
        else
          normalTransparentFilters.push_back(geomFilter);
      }
    }
    if (geomFilter->isReady(eye)) {
      selectedFilters.push_back(geomFilter);
    }
  }
  //}
  size_t numNormalFilters = normalOpaqueFilters.size() + normalTransparentFilters.size();
  size_t numOnTopFilters = onTopOpaqueFilters.size() + onTopTransparentFilters.size();

  Z3DRenderOutputPort& currentOutport = (eye == Z3DEye::Mono) ?
        m_outport : (eye == Z3DEye::Left) ? m_leftEyeOutport : m_rightEyeOutport;
  Z3DRenderInputPort& currentInport = (eye == Z3DEye::Mono) ?
        m_inport : (eye == Z3DEye::Left) ? m_leftEyeInport : m_rightEyeInport;

  glEnable(GL_DEPTH_TEST);

  if (m_rendererBase.transparencyMethodPara().isSelected("Blend No Depth Mask") ||
      m_rendererBase.transparencyMethodPara().isSelected("Blend Delayed")) {
    if (!currentInport.isReady()) {  // no volume, only geometrys to render
      if (numNormalFilters == 0 || numOnTopFilters == 0) {
        if (m_rendererBase.geometriesMultisampleModePara().isSelected(
          "2x2")) { // render to tempport (twice larger than outport) then copy to outport
          m_tempPort.resize(currentOutport.size() * 2_u32);
        } else {  // render to tempport then copy to outport
          m_tempPort.resize(currentOutport.size());
        }

        if (numOnTopFilters == 0)
          renderGeometries(normalOpaqueFilters, normalTransparentFilters, m_tempPort, eye);
        else
          renderGeometries(onTopOpaqueFilters, onTopTransparentFilters, m_tempPort, eye);

        // copy to outport
        currentOutport.bindTarget();
        currentOutport.clearTarget();
        m_rendererBase.setViewport(currentOutport.size());

        if (m_showBackground.get()) {
          m_rendererBase.render(eye, m_backgroundRenderer);
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        glDepthFunc(GL_ALWAYS);
        m_textureCopyRenderer.setColorTexture(m_tempPort.colorTexture());
        m_textureCopyRenderer.setDepthTexture(m_tempPort.depthTexture());
        m_rendererBase.render(eye, m_textureCopyRenderer);
        glDepthFunc(GL_LESS);
        if (m_showAxis.get()) {
          if (!m_showBackground.get()) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          }
          renderAxis(eye);
        }
        if (m_showBackground.get() || m_showAxis.get()) {
          glBlendFunc(GL_ONE, GL_ZERO);
          glDisable(GL_BLEND);
        }

        currentOutport.releaseTarget();
      } else {
        if (m_rendererBase.geometriesMultisampleModePara().isSelected("2x2")) {
          m_tempPort.resize(currentOutport.size() * 2_u32);
          m_tempPort2.resize(currentOutport.size() * 2_u32);
        } else {
          m_tempPort.resize(currentOutport.size());
          m_tempPort2.resize(currentOutport.size());
        }

        // render normal geometries to tempport
        renderGeometries(normalOpaqueFilters, normalTransparentFilters, m_tempPort, eye);

        // render on top geometries to tempport2
        renderGeometries(onTopOpaqueFilters, onTopTransparentFilters, m_tempPort2, eye);

        // blend to output
        currentOutport.bindTarget();
        currentOutport.clearTarget();
        m_rendererBase.setViewport(currentOutport.size());

        if (m_showBackground.get()) {
          m_rendererBase.render(eye, m_backgroundRenderer);
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        m_firstOnTopBlendRenderer.setColorTexture1(m_tempPort2.colorTexture());
        m_firstOnTopBlendRenderer.setDepthTexture1(m_tempPort2.depthTexture());
        m_firstOnTopBlendRenderer.setColorTexture2(m_tempPort.colorTexture());
        m_firstOnTopBlendRenderer.setDepthTexture2(m_tempPort.depthTexture());
        m_rendererBase.render(eye, m_firstOnTopBlendRenderer);
        if (m_showAxis.get()) {
          if (!m_showBackground.get()) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          }
          renderAxis(eye);
        }
        if (m_showBackground.get() || m_showAxis.get()) {
          glBlendFunc(GL_ONE, GL_ZERO);
          glDisable(GL_BLEND);
        }

        currentOutport.releaseTarget();
      }
    } else {      // with volume
      if (numNormalFilters == 0 && numOnTopFilters == 0) {  // directly copy inport image to outport
        const Z3DTexture* colorTex = nullptr;
        const Z3DTexture* depthTex = nullptr;
        renderImages(currentInport, currentOutport, eye, colorTex, depthTex);

        currentOutport.bindTarget();
        currentOutport.clearTarget();
        m_rendererBase.setViewport(currentOutport.size());

        if (m_showBackground.get()) {
          m_rendererBase.render(eye, m_backgroundRenderer);
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        glDepthFunc(GL_ALWAYS);
        m_textureCopyRenderer.setColorTexture(colorTex);
        m_textureCopyRenderer.setDepthTexture(depthTex);
        m_rendererBase.render(eye, m_textureCopyRenderer);
        glDepthFunc(GL_LESS);
        if (m_showAxis.get()) {
          if (!m_showBackground.get()) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          }
          renderAxis(eye);
        }
        if (m_showBackground.get() || m_showAxis.get()) {
          glBlendFunc(GL_ONE, GL_ZERO);
          glDisable(GL_BLEND);
        }

        currentOutport.releaseTarget();
      } else if (numNormalFilters == 0 ||
                 numOnTopFilters == 0) {  // render geometries into one temp port then blend with volume
        if (m_rendererBase.geometriesMultisampleModePara().isSelected("2x2")) {
          m_tempPort.resize(currentOutport.size() * 2_u32);
        } else {
          m_tempPort.resize(currentOutport.size());
        }

        // render geometries into one temp port
        if (numOnTopFilters == 0)
          renderGeometries(normalOpaqueFilters, normalTransparentFilters, m_tempPort, eye);
        else
          renderGeometries(onTopOpaqueFilters, onTopTransparentFilters, m_tempPort, eye);

        const Z3DTexture* colorTex = nullptr;
        const Z3DTexture* depthTex = nullptr;
        renderImages(currentInport, currentOutport, eye, colorTex, depthTex);

        // blend tempPort with volume
        currentOutport.bindTarget();
        currentOutport.clearTarget();
        m_rendererBase.setViewport(currentOutport.size());

        if (m_showBackground.get()) {
          m_rendererBase.render(eye, m_backgroundRenderer);
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (numOnTopFilters == 0) {
          m_alphaBlendRenderer.setColorTexture1(m_tempPort.colorTexture());
          m_alphaBlendRenderer.setDepthTexture1(m_tempPort.depthTexture());
          m_alphaBlendRenderer.setColorTexture2(colorTex);
          m_alphaBlendRenderer.setDepthTexture2(depthTex);
          m_rendererBase.render(eye, m_alphaBlendRenderer);
        } else {
          m_firstOnTopBlendRenderer.setColorTexture1(m_tempPort.colorTexture());
          m_firstOnTopBlendRenderer.setDepthTexture1(m_tempPort.depthTexture());
          m_firstOnTopBlendRenderer.setColorTexture2(colorTex);
          m_firstOnTopBlendRenderer.setDepthTexture2(depthTex);
          m_rendererBase.render(eye, m_firstOnTopBlendRenderer);
        }
        if (m_showAxis.get()) {
          if (!m_showBackground.get()) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          }
          renderAxis(eye);
        }
        if (m_showBackground.get() || m_showAxis.get()) {
          glBlendFunc(GL_ONE, GL_ZERO);
          glDisable(GL_BLEND);
        }

        currentOutport.releaseTarget();
      } else { // render normal geometries into tempport, then blend inport and tempport into tempport2, then render on top geometries into tempport, then
        // blend temport and temport2 into outport
        if (m_rendererBase.geometriesMultisampleModePara().isSelected("2x2")) {
          m_tempPort.resize(currentOutport.size() * 2_u32);
          m_tempPort2.resize(currentOutport.size() * 2_u32);
        } else {
          m_tempPort.resize(currentOutport.size());
          m_tempPort2.resize(currentOutport.size());
        }

        // render normal geometries into tempport
        renderGeometries(normalOpaqueFilters, normalTransparentFilters, m_tempPort, eye);

        const Z3DTexture* colorTex = nullptr;
        const Z3DTexture* depthTex = nullptr;
        renderImages(currentInport, currentOutport, eye, colorTex, depthTex);

        // blend inport and tempport into tempport2
        m_tempPort2.bindTarget();

        m_rendererBase.setViewport(m_tempPort2.size());
        m_alphaBlendRenderer.setColorTexture1(m_tempPort.colorTexture());
        m_alphaBlendRenderer.setDepthTexture1(m_tempPort.depthTexture());
        m_alphaBlendRenderer.setColorTexture2(colorTex);
        m_alphaBlendRenderer.setDepthTexture2(depthTex);
        m_rendererBase.render(eye, m_alphaBlendRenderer);

        m_tempPort2.releaseTarget();

        // render on top geometries into tempport
        renderGeometries(onTopOpaqueFilters, onTopTransparentFilters, m_tempPort, eye);

        // blend temport and temport2 into outport
        currentOutport.bindTarget();
        currentOutport.clearTarget();
        m_rendererBase.setViewport(currentOutport.size());

        if (m_showBackground.get()) {
          m_rendererBase.render(eye, m_backgroundRenderer);
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        m_firstOnTopBlendRenderer.setColorTexture1(m_tempPort.colorTexture());
        m_firstOnTopBlendRenderer.setDepthTexture1(m_tempPort.depthTexture());
        m_firstOnTopBlendRenderer.setColorTexture2(m_tempPort2.colorTexture());
        m_firstOnTopBlendRenderer.setDepthTexture2(m_tempPort2.depthTexture());
        m_rendererBase.render(eye, m_firstOnTopBlendRenderer);
        if (m_showAxis.get()) {
          if (!m_showBackground.get()) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          }
          renderAxis(eye);
        }
        if (m_showBackground.get() || m_showAxis.get()) {
          glBlendFunc(GL_ONE, GL_ZERO);
          glDisable(GL_BLEND);
        }

        currentOutport.releaseTarget();
      }
    }
  } else { // OIT
    for (auto vFilter : vFilters) {
      if (vFilter->isReady(eye) && vFilter->hasTransparent(eye)) {
        normalTransparentFilters.push_back(vFilter);
      }
    }
    numNormalFilters = normalOpaqueFilters.size() + normalTransparentFilters.size();
    if (numNormalFilters == 0 || numOnTopFilters == 0) {
      if (m_rendererBase.geometriesMultisampleModePara().isSelected(
        "2x2")) { // render to tempport (twice larger than outport) then copy to outport
        m_tempPort.resize(currentOutport.size() * 2_u32);
      } else {  // render to tempport then copy to outport
        m_tempPort.resize(currentOutport.size());
      }

      if (numOnTopFilters == 0)
        renderGeometries(normalOpaqueFilters, normalTransparentFilters, m_tempPort, eye);
      else
        renderGeometries(onTopOpaqueFilters, onTopTransparentFilters, m_tempPort, eye);

      // copy to outport
      currentOutport.bindTarget();
      currentOutport.clearTarget();
      m_rendererBase.setViewport(currentOutport.size());

      if (m_showBackground.get()) {
        m_rendererBase.render(eye, m_backgroundRenderer);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      }
      glDepthFunc(GL_ALWAYS);
      m_textureCopyRenderer.setColorTexture(m_tempPort.colorTexture());
      m_textureCopyRenderer.setDepthTexture(m_tempPort.depthTexture());
      m_rendererBase.render(eye, m_textureCopyRenderer);
      glDepthFunc(GL_LESS);
      if (m_showAxis.get()) {
        if (!m_showBackground.get()) {
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        renderAxis(eye);
      }
      if (m_showBackground.get() || m_showAxis.get()) {
        glBlendFunc(GL_ONE, GL_ZERO);
        glDisable(GL_BLEND);
      }

      currentOutport.releaseTarget();
    } else {
      if (m_rendererBase.geometriesMultisampleModePara().isSelected("2x2")) {
        m_tempPort.resize(currentOutport.size() * 2_u32);
        m_tempPort2.resize(currentOutport.size() * 2_u32);
      } else {
        m_tempPort.resize(currentOutport.size());
        m_tempPort2.resize(currentOutport.size());
      }

      // render normal geometries to tempport
      renderGeometries(normalOpaqueFilters, normalTransparentFilters, m_tempPort, eye);

      // render on top geometries to tempport2
      renderGeometries(onTopOpaqueFilters, onTopTransparentFilters, m_tempPort2, eye);

      // blend to output
      currentOutport.bindTarget();
      currentOutport.clearTarget();
      m_rendererBase.setViewport(currentOutport.size());

      if (m_showBackground.get()) {
        m_rendererBase.render(eye, m_backgroundRenderer);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      }
      m_firstOnTopBlendRenderer.setColorTexture1(m_tempPort2.colorTexture());
      m_firstOnTopBlendRenderer.setDepthTexture1(m_tempPort2.depthTexture());
      m_firstOnTopBlendRenderer.setColorTexture2(m_tempPort.colorTexture());
      m_firstOnTopBlendRenderer.setDepthTexture2(m_tempPort.depthTexture());
      m_rendererBase.render(eye, m_firstOnTopBlendRenderer);
      if (m_showAxis.get()) {
        if (!m_showBackground.get()) {
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        renderAxis(eye);
      }
      if (m_showBackground.get() || m_showAxis.get()) {
        glBlendFunc(GL_ONE, GL_ZERO);
        glDisable(GL_BLEND);
      }

      currentOutport.releaseTarget();
    }
  }

  if (!selectedFilters.empty()) {
    currentOutport.bindTarget();
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    for (size_t i = 0; i < selectedFilters.size(); ++i) {
      selectedFilters[i]->setViewport(currentOutport.size());
      selectedFilters[i]->renderSelectionBox(eye);
    }
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
    currentOutport.releaseTarget();
  }

  // render picking objects
  if (!filters.empty()) {
    pickingManager().bindTarget();
    pickingManager().clearTarget();
    for (auto geomFilter : filters) {
      if (geomFilter->isReady(eye)) {
        geomFilter->setViewport(pickingManager().renderTarget().size());
        geomFilter->renderPicking(eye);
      }
    }
    pickingManager().releaseTarget();
  }

  glDisable(GL_DEPTH_TEST);
}

void Z3DCompositor::renderGeometries(const std::vector<Z3DBoundedFilter*>& opaqueFilters,
                                     const std::vector<Z3DBoundedFilter*>& transparentFilters,
                                     Z3DRenderOutputPort& port, Z3DEye eye)
{
  if (m_rendererBase.transparencyMethodPara().isSelected("Blend No Depth Mask"))
    renderGeomsBlendNoDepthMask(opaqueFilters, transparentFilters, port, eye);
  else if (m_rendererBase.transparencyMethodPara().isSelected("Blend Delayed"))
    renderGeomsBlendDelayed(opaqueFilters, transparentFilters, port, eye);
  else
    renderGeomsOIT(opaqueFilters, transparentFilters, port, eye, m_rendererBase.transparencyMethodPara().get());
}

//void Z3DCompositor::renderGeomsBlendDelayed(const std::vector<Z3DGeometryFilter *> &filters,
//                                            Z3DRenderOutputPort &port, Z3DEye eye)
//{
//    //
//    std::vector<Z3DGeometryFilter*> opaqueFilters;
//    std::vector<Z3DGeometryFilter*> transparentFilters;

//    for (size_t i=0; i<filters.size(); i++) {
//      Z3DGeometryFilter* geomFilter = filters.at(i);
//      if(geomFilter->getClassName().contains("Surface"))
//          transparentFilters.push_back(geomFilter);
//      else
//          opaqueFilters.push_back(geomFilter);
//    }

//#if 0
//    // debug ...
//    for(size_t i=0; i<transparentFilters.size(); i++)
//        qDebug()<<"transparentFilters ... ... "<<transparentFilters[i]->getClassName();
//    for(size_t i=0; i<opaqueFilters.size(); i++)
//        qDebug()<<"opaqueFilters ... ... "<<opaqueFilters[i]->getClassName();
//#endif
//    //
//    if (transparentFilters.empty()) {
//      renderOpaqueObj(opaqueFilters, port, eye);
//    }
//    else if (opaqueFilters.empty()) {
//      renderTransparentWB(transparentFilters, port, eye);
//    }
//    else {
//        m_tempPort3.resize(port.getSize());
//        renderOpaqueObj(opaqueFilters, m_tempPort3, eye);

//        // copy m_tempPort3 to port
//        //        port.bindTarget();
//        //        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//        //        m_rendererBase->setViewport(m_tempPort3.getSize());
//        //        m_textureCopyRenderer->setColorTexture(m_tempPort3.getColorTexture());
//        //        m_textureCopyRenderer->setDepthTexture(m_tempPort3.getDepthTexture());
//        //        m_rendererBase->activateRenderer(m_textureCopyRenderer);
//        //        m_rendererBase->render(eye);
//        //        port.releaseTarget();

//        m_tempPort4.resize(port.getSize());
//        renderTransparentWB(transparentFilters, m_tempPort4, eye);

//        // copy m_tempPort4 to port
//        //        port.bindTarget();
//        //        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//        //        m_rendererBase->setViewport(m_tempPort4.getSize());
//        //        m_textureCopyRenderer->setColorTexture(m_tempPort4.getColorTexture());
//        //        m_textureCopyRenderer->setDepthTexture(m_tempPort4.getDepthTexture());
//        //        m_rendererBase->activateRenderer(m_textureCopyRenderer);
//        //        m_rendererBase->render(eye);
//        //        port.releaseTarget();

//        // blend temport3 and temport4 into outport
//        port.bindTarget();
//        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//        m_rendererBase->setViewport(port.getSize());
//        m_alphaBlendRenderer->setColorTexture1(m_tempPort3.getColorTexture());
//        m_alphaBlendRenderer->setDepthTexture1(m_tempPort3.getDepthTexture());
//        m_alphaBlendRenderer->setColorTexture2(m_tempPort4.getColorTexture());
//        m_alphaBlendRenderer->setDepthTexture2(m_tempPort4.getDepthTexture());
//        m_rendererBase->activateRenderer(m_alphaBlendRenderer);
//        m_rendererBase->render(eye);
//        port.releaseTarget();
//    }

//}

void Z3DCompositor::renderGeomsBlendDelayed(const std::vector<Z3DBoundedFilter*>& opaqueFilters,
                                            const std::vector<Z3DBoundedFilter*>& transparentFilters,
                                            Z3DRenderOutputPort& port, Z3DEye eye)
{
  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (auto filter : opaqueFilters) {
    filter->setViewport(port.size());
    filter->renderOpaque(eye);
  }

  for (auto filter : transparentFilters) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    filter->setViewport(port.size());
    filter->renderTransparent(eye);
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
  }

  port.releaseTarget();
}

void Z3DCompositor::renderGeomsBlendNoDepthMask(const std::vector<Z3DBoundedFilter*>& opaqueFilters,
                                                const std::vector<Z3DBoundedFilter*>& transparentFilters,
                                                Z3DRenderOutputPort& port, Z3DEye eye)
{
  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (auto filter : opaqueFilters) {
    filter->setViewport(port.size());
    filter->renderOpaque(eye);
  }

  for (auto filter : transparentFilters) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    filter->setViewport(port.size());
    filter->renderTransparent(eye);
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
  }

  port.releaseTarget();
}

void Z3DCompositor::renderGeomsOIT(const std::vector<Z3DBoundedFilter*>& opaqueFilters,
                                   const std::vector<Z3DBoundedFilter*>& transparentFilters,
                                   Z3DRenderOutputPort& port, Z3DEye eye, const QString& method)
{
  //  std::vector<Z3DBoundedFilter*> allFilters;
  //  allFilters.insert(allFilters.end(), opaqueFilters.begin(), opaqueFilters.end());
  //  allFilters.insert(allFilters.end(), transparentFilters.begin(), transparentFilters.end());
  //  if (method == "Dual Depth Peeling") {
  //    renderTransparentDDP(allFilters, port, eye);
  //  } else if (method == "Weighted Average") {
  //    renderTransparentWA(allFilters, port, eye);
  //  }
  //  return;
  if (transparentFilters.empty()) {
    renderOpaqueFilters(opaqueFilters, port, eye);
  }
    //  else {
    //    if (method == "Dual Depth Peeling") {
    //      renderTransparentDDP(renderers, port, eye);
    //    }
    //  }
  else if (opaqueFilters.empty()) {
    if (method == "Dual Depth Peeling") {
      renderTransparentDDP(transparentFilters, port, eye);
    } else if (method == "Weighted Average") {
      renderTransparentWA(transparentFilters, port, eye);
    } else if (method == "Weighted Blended") {
      renderTransparentWB(transparentFilters, port, eye);
    }
  } else {
    m_tempPort3.resize(port.size());
    renderOpaqueFilters(opaqueFilters, m_tempPort3, eye);

    m_tempPort4.resize(port.size());
    if (method == "Dual Depth Peeling") {
      renderTransparentDDP(transparentFilters, m_tempPort4, eye, m_tempPort3.depthTexture());
    } else if (method == "Weighted Average") {
      renderTransparentWA(transparentFilters, m_tempPort4, eye, m_tempPort3.depthTexture());
    } else if (method == "Weighted Blended") {
      renderTransparentWB(transparentFilters, m_tempPort4, eye, m_tempPort3.depthTexture());
    }

    // blend temport3 and temport4 into outport
    port.bindTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_rendererBase.setViewport(port.size());
    m_alphaBlendRenderer.setColorTexture1(m_tempPort3.colorTexture());
    m_alphaBlendRenderer.setDepthTexture1(m_tempPort3.depthTexture());
    m_alphaBlendRenderer.setColorTexture2(m_tempPort4.colorTexture());
    m_alphaBlendRenderer.setDepthTexture2(m_tempPort4.depthTexture());
    m_rendererBase.render(eye, m_alphaBlendRenderer);
    port.releaseTarget();
  }
}

void Z3DCompositor::renderOpaqueFilters(const std::vector<Z3DBoundedFilter*>& filters,
                                        Z3DRenderOutputPort& port, Z3DEye eye)
{
  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  for (auto filter : filters) {
    filter->setViewport(port.size());
    filter->renderOpaque(eye);
  }
  port.releaseTarget();
}

//void Z3DCompositor::renderOpaqueObj(const std::vector<Z3DGeometryFilter *> &filters,
//                                 Z3DRenderOutputPort &port, Z3DEye eye)
//{
//  //
//  port.bindTarget();
//  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//  //
//  for (size_t i=0; i<filters.size(); i++) {
//    Z3DGeometryFilter* geomFilter = filters.at(i);
//    if (geomFilter->needBlending()) {
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
//        geomFilter->setCamera(m_camera.get());
//        geomFilter->setViewport(port.getSize());
//        geomFilter->render(eye);
//        glBlendFunc(GL_ONE,GL_ZERO);
//        glDisable(GL_BLEND);
//    }
//    else
//    {
//        geomFilter->setCamera(m_camera.get());
//        geomFilter->setViewport(port.getSize());
//        geomFilter->render(eye);
//    }
//  }

//  //
//  port.releaseTarget();
//}

void Z3DCompositor::renderTransparentDDP(const std::vector<Z3DBoundedFilter*>& filters,
                                         Z3DRenderOutputPort& port, Z3DEye eye, Z3DTexture* depthTexture)
{
  if (!m_ddpRT) if (!createDDPRenderTarget(port.size())) {
    LOG(ERROR) << "Can not create fbo for dual depth peeling rendering";
    return;
  }
  m_ddpRT->resize(port.size());
  if (depthTexture) {
    m_ddpRT->attachTextureToFBO(depthTexture, GL_DEPTH_ATTACHMENT, false);
    m_ddpRT->isFBOComplete();
  }

  const Z3DTexture* g_dualDepthTexId[2];
  g_dualDepthTexId[0] = m_ddpRT->attachment(GL_COLOR_ATTACHMENT0);
  g_dualDepthTexId[1] = m_ddpRT->attachment(GL_COLOR_ATTACHMENT3);
  const Z3DTexture* g_dualFrontBlenderTexId[2];
  g_dualFrontBlenderTexId[0] = m_ddpRT->attachment(GL_COLOR_ATTACHMENT1);
  g_dualFrontBlenderTexId[1] = m_ddpRT->attachment(GL_COLOR_ATTACHMENT4);
  const Z3DTexture* g_dualBackTempTexId[2];
  g_dualBackTempTexId[0] = m_ddpRT->attachment(GL_COLOR_ATTACHMENT2);
  g_dualBackTempTexId[1] = m_ddpRT->attachment(GL_COLOR_ATTACHMENT5);
  const Z3DTexture* g_dualBackBlenderTexId = m_ddpRT->attachment(GL_COLOR_ATTACHMENT6);
  const Z3DTexture* g_depthTex = m_ddpRT->attachment(GL_COLOR_ATTACHMENT7);
  const GLenum g_drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                  GL_COLOR_ATTACHMENT1,
                                  GL_COLOR_ATTACHMENT2,
                                  GL_COLOR_ATTACHMENT3,
                                  GL_COLOR_ATTACHMENT4,
                                  GL_COLOR_ATTACHMENT5,
                                  GL_COLOR_ATTACHMENT6
  };

  const GLenum g_db[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT7};

  bool g_useOQ = true;
  int g_numPasses = 100;

#define MAX_DEPTH 1.0

  if (depthTexture)
    glDepthMask(GL_FALSE);
  else
    glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  // ---------------------------------------------------------------------
  // 1. Initialize Min-Max Depth Buffer
  // ---------------------------------------------------------------------

  m_ddpRT->bind();

  // Render targets 1 and 2 store the front and back colors
  // Clear to 0.0 and use MAX blending to filter written color
  // At most one front color and one back color can be written every pass
  glDrawBuffers(2, &g_drawBuffers[1]);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  // Render target 0 stores (-minDepth, maxDepth, alphaMultiplier)
  glDrawBuffers(2, g_db);
  glClearColor(-MAX_DEPTH, -MAX_DEPTH, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glBlendEquation(GL_MAX);

  for (auto filter : filters) {
    filter->setViewport(m_ddpRT->size());
    filter->setShaderHookType(Z3DRendererBase::ShaderHookType::DualDepthPeelingInit);
    filter->renderTransparent(eye);
  }


  // ---------------------------------------------------------------------
  // 2. Dual Depth Peeling + Blending
  // ---------------------------------------------------------------------

  // Since we cannot blend the back colors in the geometry passes,
  // we use another render target to do the alpha blending
  glDrawBuffer(g_drawBuffers[6]);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  int currId = 0;

  for (int pass = 1; g_useOQ && pass < g_numPasses; pass++) {
    currId = pass % 2;
    int prevId = 1 - currId;
    int bufId = currId * 3;

    glDrawBuffers(2, &g_drawBuffers[bufId + 1]);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawBuffer(g_drawBuffers[bufId + 0]);
    glClearColor(-MAX_DEPTH, -MAX_DEPTH, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render target 0: RG32F MAX blending
    // Render target 1: RGBA MAX blending
    // Render target 2: RGBA MAX blending
    glDrawBuffers(3, &g_drawBuffers[bufId + 0]);
    glBlendEquation(GL_MAX);

    for (auto filter : filters) {
      filter->setViewport(m_ddpRT->size());
      filter->setShaderHookType(Z3DRendererBase::ShaderHookType::DualDepthPeelingPeel);
      filter->setShaderHookParaDDPDepthBlenderTexture(g_dualDepthTexId[prevId]);
      filter->setShaderHookParaDDPFrontBlenderTexture(g_dualFrontBlenderTexId[prevId]);
      filter->renderTransparent(eye);
    }

    // Full screen pass to alpha-blend the back color
    glDrawBuffer(g_drawBuffers[6]);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    GLuint queryId;
    if (g_useOQ) {
      glGenQueries(1, &queryId);
      glBeginQuery(GL_SAMPLES_PASSED, queryId);
    }

    m_ddpBlendShader.bind();
    m_ddpBlendShader.bindTexture("TempTex", g_dualBackTempTexId[currId]);
#if !defined(USE_RECT_TEX)
    m_rendererBase.setViewport(m_ddpRT->size());
    m_rendererBase.setGlobalShaderParameters(m_ddpBlendShader, eye);
#endif
    renderScreenQuad(m_screenQuadVAO, m_ddpBlendShader);
    m_ddpBlendShader.release();

    if (g_useOQ) {
      glEndQuery(GL_SAMPLES_PASSED);
      GLuint sample_count;
      glGetQueryObjectuiv(queryId, GL_QUERY_RESULT, &sample_count);
      glDeleteQueries(1, &queryId);
      if (sample_count == 0) {
        break;
      }
    }
  }

  if (depthTexture) {
    m_ddpRT->detach(GL_DEPTH_ATTACHMENT);
    m_ddpRT->isFBOComplete();
  }
  m_ddpRT->release();

  if (depthTexture)
    glDepthMask(GL_TRUE);
  glClearColor(0, 0, 0, 0);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ZERO);
  glDisable(GL_BLEND);

  for (auto filter : filters) {
    filter->setShaderHookType(Z3DRendererBase::ShaderHookType::Normal);
  }

  // ---------------------------------------------------------------------
  // 3. Final Pass
  // ---------------------------------------------------------------------

  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_ddpFinalShader.bind();
  m_ddpFinalShader.bindTexture("DepthTex", g_depthTex);
  m_ddpFinalShader.bindTexture("FrontBlenderTex", g_dualFrontBlenderTexId[currId]);
  m_ddpFinalShader.bindTexture("BackBlenderTex", g_dualBackBlenderTexId);
#if !defined(USE_RECT_TEX)
  m_rendererBase.setViewport(m_ddpRT->size());
  m_rendererBase.setGlobalShaderParameters(m_ddpFinalShader, eye);
#endif
  renderScreenQuad(m_screenQuadVAO, m_ddpFinalShader);
  m_ddpFinalShader.release();
  port.releaseTarget();

  glEnable(GL_DEPTH_TEST);
}

bool Z3DCompositor::createDDPRenderTarget(const glm::uvec2& size)
{
  m_ddpRT.reset(new Z3DRenderTarget(size));
  Z3DTexture* g_dualDepthTexId[2];
  Z3DTexture* g_dualFrontBlenderTexId[2];
  Z3DTexture* g_dualBackTempTexId[2];
  Z3DTexture* g_dualBackBlenderTexId;
  Z3DTexture* g_depthTex;

#ifdef USE_RECT_TEX
  for (int i = 0; i < 2; ++i)
  {
    g_dualDepthTexId[i] = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                         GL_RG, GL_RG32F, GL_FLOAT,
                                         GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
    g_dualDepthTexId[i]->uploadTexture();

    g_dualFrontBlenderTexId[i] = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                                GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT,
                                                GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
    g_dualFrontBlenderTexId[i]->uploadTexture();

    g_dualBackTempTexId[i] = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                            GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT,
                                            GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
    g_dualBackTempTexId[i]->uploadTexture();
  }

  g_dualBackBlenderTexId = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                          GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT,
                                          GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
  g_dualBackBlenderTexId->uploadTexture();

  g_depthTex = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                              GL_RED, GL_R32F, GL_FLOAT,
                              GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
  g_depthTex->uploadTexture();
#else
  for (int i = 0; i < 2; ++i) {
    g_dualDepthTexId[i] = new Z3DTexture(GLint(GL_RG32F), glm::uvec3(size, 1), GL_RG, GL_FLOAT);
    g_dualDepthTexId[i]->setFilter(GLint(GL_NEAREST), GLint(GL_NEAREST));
    g_dualDepthTexId[i]->uploadImage();

    g_dualFrontBlenderTexId[i] = new Z3DTexture(GLint(GL_RGBA16), glm::uvec3(size, 1), GL_RGBA, GL_UNSIGNED_SHORT);
    g_dualFrontBlenderTexId[i]->setFilter(GLint(GL_NEAREST), GLint(GL_NEAREST));
    g_dualFrontBlenderTexId[i]->uploadImage();

    g_dualBackTempTexId[i] = new Z3DTexture(GLint(GL_RGBA16), glm::uvec3(size, 1), GL_RGBA, GL_UNSIGNED_SHORT);
    g_dualBackTempTexId[i]->setFilter(GLint(GL_NEAREST), GLint(GL_NEAREST));
    g_dualBackTempTexId[i]->uploadImage();
  }

  g_dualBackBlenderTexId = new Z3DTexture(GLint(GL_RGBA16), glm::uvec3(size, 1), GL_RGBA, GL_UNSIGNED_SHORT);
  g_dualBackBlenderTexId->setFilter(GLint(GL_NEAREST), GLint(GL_NEAREST));
  g_dualBackBlenderTexId->uploadImage();

  g_depthTex = new Z3DTexture(GLint(GL_R32F), glm::uvec3(size, 1), GL_RED, GL_FLOAT);
  g_depthTex->setFilter(GLint(GL_NEAREST), GLint(GL_NEAREST));
  g_depthTex->uploadImage();
#endif

  int j = 0;
  m_ddpRT->attachTextureToFBO(g_dualDepthTexId[j], GL_COLOR_ATTACHMENT0);
  m_ddpRT->attachTextureToFBO(g_dualFrontBlenderTexId[j], GL_COLOR_ATTACHMENT1);
  m_ddpRT->attachTextureToFBO(g_dualBackTempTexId[j], GL_COLOR_ATTACHMENT2);

  j = 1;
  m_ddpRT->attachTextureToFBO(g_dualDepthTexId[j], GL_COLOR_ATTACHMENT3);
  m_ddpRT->attachTextureToFBO(g_dualFrontBlenderTexId[j], GL_COLOR_ATTACHMENT4);
  m_ddpRT->attachTextureToFBO(g_dualBackTempTexId[j], GL_COLOR_ATTACHMENT5);

  m_ddpRT->attachTextureToFBO(g_dualBackBlenderTexId, GL_COLOR_ATTACHMENT6);
  m_ddpRT->attachTextureToFBO(g_depthTex, GL_COLOR_ATTACHMENT7);
  bool comp = m_ddpRT->isFBOComplete();
  if (!comp) {
    m_ddpRT.reset();
  }
  return comp;
}

void Z3DCompositor::renderTransparentWA(const std::vector<Z3DBoundedFilter*>& filters,
                                        Z3DRenderOutputPort& port, Z3DEye eye, Z3DTexture* depthTexture)
{
  if (!m_waRT) {
    if (!createWARenderTarget(port.size())) {
      LOG(ERROR) << "Can not create fbo for weighted average rendering";
      return;
    }
  }
  m_waRT->resize(port.size());
  if (depthTexture) {
    m_waRT->attachTextureToFBO(depthTexture, GL_DEPTH_ATTACHMENT, false);
    m_waRT->isFBOComplete();
  }

  const Z3DTexture* g_accumulationTexId[2];
  g_accumulationTexId[0] = m_waRT->attachment(GL_COLOR_ATTACHMENT0);
  g_accumulationTexId[1] = m_waRT->attachment(GL_COLOR_ATTACHMENT1);
  const GLenum g_drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                  GL_COLOR_ATTACHMENT1
  };

  if (depthTexture)
    glDepthMask(GL_FALSE);
  else
    glDisable(GL_DEPTH_TEST);

  // ---------------------------------------------------------------------
  // 1. Accumulate Colors and Depth Complexity
  // ---------------------------------------------------------------------

  m_waRT->bind();

  glDrawBuffers(2, g_drawBuffers);

  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_BLEND);

  for (auto filter : filters) {
    filter->setViewport(m_waRT->size());
    filter->setShaderHookType(Z3DRendererBase::ShaderHookType::WeightedAverageInit);
    filter->renderTransparent(eye);
  }

  if (depthTexture) {
    m_waRT->detach(GL_DEPTH_ATTACHMENT);
    m_waRT->isFBOComplete();
  }
  m_waRT->release();

  if (depthTexture)
    glDepthMask(GL_TRUE);
  glBlendFunc(GL_ONE, GL_ZERO);
  glDisable(GL_BLEND);

  for (auto filter : filters) {
    filter->setShaderHookType(Z3DRendererBase::ShaderHookType::Normal);
  }

  // ---------------------------------------------------------------------
  // 2. Approximate Blending
  // ---------------------------------------------------------------------

  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_waFinalShader.bind();
  m_waFinalShader.bindTexture("ColorTex0", g_accumulationTexId[0]);
  m_waFinalShader.bindTexture("ColorTex1", g_accumulationTexId[1]);
#if !defined(USE_RECT_TEX)
  m_rendererBase.setViewport(m_waRT->size());
  m_rendererBase.setGlobalShaderParameters(m_waFinalShader, eye);
#endif
  renderScreenQuad(m_screenQuadVAO, m_waFinalShader);
  m_waFinalShader.release();
  port.releaseTarget();

  glEnable(GL_DEPTH_TEST);
}

bool Z3DCompositor::createWARenderTarget(const glm::uvec2& size)
{
  m_waRT.reset(new Z3DRenderTarget(size));
  Z3DTexture* g_accumulationTexId[2];

#ifdef USE_RECT_TEX
  g_accumulationTexId[0] = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                          (GLint)GL_RGBA, GL_RGBA32F, GL_FLOAT,
                                          (GLint)GL_NEAREST, (GLint)GL_NEAREST, (GLint)GL_CLAMP_TO_EDGE);
  g_accumulationTexId[0]->uploadTexture();
  g_accumulationTexId[1] = new Z3DTexture(glm::ivec3(size, 1), GL_TEXTURE_RECTANGLE,
                                          (GLint)GL_RG, GL_RG32F, GL_FLOAT,
                                          (GLint)GL_NEAREST, (GLint)GL_NEAREST, (GLint)GL_CLAMP_TO_EDGE);
  g_accumulationTexId[1]->uploadTexture();
#else
  g_accumulationTexId[0] = new Z3DTexture(GLint(GL_RGBA32F), glm::uvec3(size, 1), GL_RGBA, GL_FLOAT);
  g_accumulationTexId[0]->setFilter(GLint(GL_NEAREST), GLint(GL_NEAREST));
  g_accumulationTexId[0]->uploadImage();
  g_accumulationTexId[1] = new Z3DTexture(GLint(GL_RG32F), glm::uvec3(size, 1), GL_RG, GL_FLOAT);
  g_accumulationTexId[1]->setFilter(GLint(GL_NEAREST), GLint(GL_NEAREST));
  g_accumulationTexId[1]->uploadImage();
#endif

  m_waRT->attachTextureToFBO(g_accumulationTexId[0], GL_COLOR_ATTACHMENT0);
  m_waRT->attachTextureToFBO(g_accumulationTexId[1], GL_COLOR_ATTACHMENT1);
  bool comp = m_waRT->isFBOComplete();
  if (!comp) {
    m_waRT.reset();
  }
  return comp;
}

void Z3DCompositor::renderTransparentWB(const std::vector<Z3DBoundedFilter*>& filters,
                                        Z3DRenderOutputPort& port, Z3DEye eye, Z3DTexture* depthTexture)
{
  if (!m_wbRT) {
    if (!createWBRenderTarget(port.size())) {
      LOG(ERROR) << "Can not create fbo for weighted blended rendering";
      return;
    }
  }
  m_wbRT->resize(port.size());
  if (depthTexture) {
    m_wbRT->attachTextureToFBO(depthTexture, GL_DEPTH_ATTACHMENT, false);
    m_wbRT->isFBOComplete();
  }

  const Z3DTexture* g_accumulationTexId[2];
  g_accumulationTexId[0] = m_wbRT->attachment(GL_COLOR_ATTACHMENT0);
  g_accumulationTexId[1] = m_wbRT->attachment(GL_COLOR_ATTACHMENT1);
  const GLenum g_drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                  GL_COLOR_ATTACHMENT1
  };

  if (depthTexture)
    glDepthMask(GL_FALSE);
  else
    glDisable(GL_DEPTH_TEST);

  // ---------------------------------------------------------------------
  // 1. Geometry pass
  // ---------------------------------------------------------------------

  m_wbRT->bind();

  glDrawBuffers(2, g_drawBuffers);

  // Render target 0 stores a sum (weighted RGBA colors). Clear it to 0.f.
  // Render target 1 stores a product (transmittances). Clear it to 1.f.
  float clearColorZero[4] = { 0.f, 0.f, 0.f, 0.f };
  float clearColorOne[4]  = { 1.f, 1.f, 1.f, 1.f };
  glClearBufferfv(GL_COLOR, 0, clearColorZero);
  glClearBufferfv(GL_COLOR, 1, clearColorOne);

  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunci(0, GL_ONE, GL_ONE);
  glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

  for (auto filter : filters) {
    filter->setViewport(m_wbRT->size());
    filter->setShaderHookType(Z3DRendererBase::ShaderHookType::WeightedBlendedInit);
    filter->renderTransparent(eye);
  }

  if (depthTexture) {
    m_wbRT->detach(GL_DEPTH_ATTACHMENT);
    m_wbRT->isFBOComplete();
  }
  m_wbRT->release();

  if (depthTexture)
    glDepthMask(GL_TRUE);
  glBlendFunc(GL_ONE, GL_ZERO);
  glDisable(GL_BLEND);

  for (auto filter : filters) {
    filter->setShaderHookType(Z3DRendererBase::ShaderHookType::Normal);
  }

  // ---------------------------------------------------------------------
  // 2. Compositing pass
  // ---------------------------------------------------------------------

  port.bindTarget();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_wbFinalShader.bind();
  m_wbFinalShader.bindTexture("ColorTex0", g_accumulationTexId[0]);
  m_wbFinalShader.bindTexture("ColorTex1", g_accumulationTexId[1]);
#if !defined(USE_RECT_TEX)
  m_rendererBase.setViewport(m_wbRT->size());
  m_rendererBase.setGlobalShaderParameters(m_wbFinalShader, eye);
#endif
  renderScreenQuad(m_screenQuadVAO, m_wbFinalShader);
  m_wbFinalShader.release();
  port.releaseTarget();

  glEnable(GL_DEPTH_TEST);
}

bool Z3DCompositor::createWBRenderTarget(const glm::uvec2& size)
{
  m_wbRT.reset(new Z3DRenderTarget(size));
  Z3DTexture* g_accumulationTexId[2];

  g_accumulationTexId[0] = new Z3DTexture(GLint(GL_RGBA16F), glm::uvec3(size, 1), GL_RGBA, GL_FLOAT);
  g_accumulationTexId[0]->setFilter(GLint(GL_NEAREST), GLint(GL_NEAREST));
  g_accumulationTexId[0]->uploadImage();
  g_accumulationTexId[1] = new Z3DTexture(GLint(GL_R16F), glm::uvec3(size, 1), GL_RED, GL_FLOAT);
  g_accumulationTexId[1]->setFilter(GLint(GL_NEAREST), GLint(GL_NEAREST));
  g_accumulationTexId[1]->uploadImage();

  m_wbRT->attachTextureToFBO(g_accumulationTexId[0], GL_COLOR_ATTACHMENT0);
  m_wbRT->attachTextureToFBO(g_accumulationTexId[1], GL_COLOR_ATTACHMENT1);
  bool comp = m_wbRT->isFBOComplete();
  if (!comp) {
    m_wbRT.reset();
  }
  return comp;
}

void Z3DCompositor::renderImages(Z3DRenderInputPort& currentInport, Z3DRenderOutputPort& currentOutport,
                                 Z3DEye eye, const Z3DTexture*& colorTex, const Z3DTexture*& depthTex)
{
  size_t numImages = currentInport.numValidInputs();
  if (numImages == 0) {
    CHECK(false);
  }
  if (numImages == 1) {
    colorTex = currentInport.colorTexture(0);
    depthTex = currentInport.depthTexture(0);
  } else {
    m_tempPort3.resize(currentOutport.size());
    m_tempPort4.resize(currentOutport.size());

    // blend inport1 and inport2 into tempport3
    m_tempPort3.bindTarget();
    m_tempPort3.clearTarget();

    m_rendererBase.setViewport(m_tempPort3.size());
    m_alphaBlendRenderer.setColorTexture1(currentInport.colorTexture(0));
    m_alphaBlendRenderer.setDepthTexture1(currentInport.depthTexture(0));
    m_alphaBlendRenderer.setColorTexture2(currentInport.colorTexture(1));
    m_alphaBlendRenderer.setDepthTexture2(currentInport.depthTexture(1));
    m_rendererBase.render(eye, m_alphaBlendRenderer);

    m_tempPort3.releaseTarget();

    Z3DRenderOutputPort* resPort = &m_tempPort3;
    Z3DRenderOutputPort* nextResPort = &m_tempPort4;
    for (size_t i = 2; i < numImages; ++i) {
      nextResPort->bindTarget();
      nextResPort->clearTarget();

      m_alphaBlendRenderer.setColorTexture1(resPort->colorTexture());
      m_alphaBlendRenderer.setDepthTexture1(resPort->depthTexture());
      m_alphaBlendRenderer.setColorTexture2(currentInport.colorTexture(i));
      m_alphaBlendRenderer.setDepthTexture2(currentInport.depthTexture(i));
      m_rendererBase.render(eye, m_alphaBlendRenderer);

      nextResPort->releaseTarget();

      std::swap(resPort, nextResPort);
    }

    colorTex = resPort->colorTexture();
    depthTex = resPort->depthTexture();
  }
}

void Z3DCompositor::renderAxis(Z3DEye eye)
{
  prepareAxisData(eye);
  m_rendererBase.coordTransformPara().blockSignals(true);
  m_rendererBase.coordTransformPara().set(glm::mat4(globalCamera().rotateMatrix(eye)));

  glm::uvec4 viewport = m_rendererBase.viewport();

  if (m_region[0] <= 0.f && m_region[2] <= 0.f) {
    double startX = viewport.x + viewport.z / m_region[1] * m_region[0];
    double startY = viewport.y + viewport.w / m_region[3] * m_region[2];

    GLsizei size = std::min(viewport.z, viewport.w) * m_axisRegionRatio.get();
    glViewport(viewport.x - std::floor(startX), viewport.y - std::floor(startY), size, size);
    glScissor(viewport.x - std::floor(startX), viewport.y - std::floor(startY), size, size);
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    if (m_axisMode.get() == "Arrow")
      m_rendererBase.render(eye, m_arrowRenderer, m_fontRenderer);
    else
      m_rendererBase.render(eye, m_lineRenderer, m_fontRenderer);

    glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
    glScissor(viewport.x, viewport.y, viewport.z, viewport.w);
    glDisable(GL_SCISSOR_TEST);
  }
  m_rendererBase.coordTransformPara().blockSignals(false);
}

void Z3DCompositor::prepareAxisData(Z3DEye eye)
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

void Z3DCompositor::setupAxisCamera()
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
  m_fontRenderer.setDataColors(&m_textColors);
}
