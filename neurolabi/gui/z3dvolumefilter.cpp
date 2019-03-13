#include "z3dvolumefilter.h"

#include <QApplication>
#include <QMessageBox>

#include "z3dgpuinfo.h"
#include "zeventlistenerparameter.h"
#include "zmesh.h"
#include "QsLog.h"
#include "logging/zbenchtimer.h"
#include "zmeshutils.h"
#include "zlabelcolortable.h"
#include "zsparseobject.h"
#include "mvc/zstackdochelper.h"
#include "mvc/zstackdoc.h"
#include "misc/miscutility.h"

const size_t Z3DVolumeFilter::m_maxNumOfFullResolutionVolumeSlice = 6;

Z3DVolumeFilter::Z3DVolumeFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DBoundedFilter(globalParas, parent)
  , m_volumeRaycasterRenderer(m_rendererBase)
  , m_volumeSliceRenderer(m_rendererBase)
  , m_textureAndEyeCoordinateRenderer(m_rendererBase)
  , m_textureCopyRenderer(m_rendererBase)
  , m_imgPack(nullptr)
  , m_stayOnTop("Stay On Top", false)
  , m_isVolumeDownsampled("Volume Is Downsampled", false)
  , m_isSubVolume("Is Subvolume", false)
  , m_zoomInViewSize("Zoom In View Size", 256, 128, 1024)
  , m_numParas(0)
  , m_interactionDownsample("Interaction Downsample", 1, 1, 16)
  , m_entryTarget(glm::uvec2(32, 32))
  , m_exitTarget(glm::uvec2(32, 32))
  , m_layerTarget(glm::uvec2(32, 32))
  , m_layerColorTexture(GL_TEXTURE_2D_ARRAY, (GLint) GL_RGBA16, glm::uvec3(32, 32, 3), GL_RGBA, GL_FLOAT)
  , m_layerDepthTexture(GL_TEXTURE_2D_ARRAY, (GLint) GL_DEPTH_COMPONENT24, glm::uvec3(32, 32, 3), GL_DEPTH_COMPONENT,
                        GL_FLOAT)
  , m_outport("Image", this)
  , m_leftEyeOutport("LeftEyeImage", this)
  , m_rightEyeOutport("RightEyeImage", this)
  , m_vPPort("VolumeFilter", this)
  , m_opaqueOutport("OpaqueImage", this)
  , m_opaqueLeftEyeOutport("OpaqueLeftEyeImage", this)
  , m_opaqueRightEyeOutport("OpaqueRightEyeImage", this)
  , m_FRVolumeSlices(m_maxNumOfFullResolutionVolumeSlice)
  , m_FRVolumeSlicesValidState(m_maxNumOfFullResolutionVolumeSlice, false)
  , m_useFRVolumeSlice("Use Full Resolution Volume Slice", true)
  , m_showXSlice("Show X Slice", false)
  , m_xSlicePosition("X Slice Position", 0, 0, 1)
  , m_showYSlice("Show Y Slice", false)
  , m_ySlicePosition("Y Slice Position", 0, 0, 1)
  , m_showZSlice("Show Z Slice", false)
  , m_zSlicePosition("Z Slice Position", 0, 0, 1)
  , m_showXSlice2("Show X Slice 2", false)
  , m_xSlice2Position("X Slice 2 Position", 0, 0, 1)
  , m_showYSlice2("Show Y Slice 2", false)
  , m_ySlice2Position("Y Slice 2 Position", 0, 0, 1)
  , m_showZSlice2("Show Z Slice 2", false)
  , m_zSlice2Position("Z Slice 2 Position", 0, 0, 1)
  , m_leftMouseButtonPressEvent("Left Mouse Button Pressed", false)
  , m_2DImageQuad(GL_TRIANGLE_STRIP)
{
  m_baseBoundBoxRenderer.setEnableMultisample(false);
  m_textureCopyRenderer.setDiscardTransparent(true);

  addParameter(m_stayOnTop);
  m_isVolumeDownsampled.setEnabled(false);
  addParameter(m_isVolumeDownsampled);
  m_isSubVolume.setEnabled(false);
  addParameter(m_isSubVolume);
  m_zoomInViewSize.setTracking(false);
  m_zoomInViewSize.setSingleStep(32);
  addParameter(m_zoomInViewSize);
  connect(&m_rendererBase, &Z3DRendererBase::coordTransformChanged, this, &Z3DVolumeFilter::changeCoordTransform);
  connect(&m_zoomInViewSize, &ZIntParameter::valueChanged, this, &Z3DVolumeFilter::changeZoomInViewSize);

  addParameter(m_interactionDownsample);

  Z3DTexture* g_TexId[2];
  g_TexId[0] = new Z3DTexture((GLint) GL_RGBA32F, glm::uvec3(32, 32, 1), GL_RGBA, GL_FLOAT);
  g_TexId[0]->setFilter((GLint) GL_NEAREST, (GLint) GL_NEAREST);
  g_TexId[0]->uploadImage();
  g_TexId[1] = new Z3DTexture((GLint) GL_RGBA32F, glm::uvec3(32, 32, 1), GL_RGBA, GL_FLOAT);
  g_TexId[1]->setFilter((GLint) GL_NEAREST, (GLint) GL_NEAREST);
  g_TexId[1]->uploadImage();
  m_entryTarget.attachTextureToFBO(g_TexId[0], GL_COLOR_ATTACHMENT0);
  m_entryTarget.attachTextureToFBO(g_TexId[1], GL_COLOR_ATTACHMENT1);
  m_entryTarget.isFBOComplete();
  g_TexId[0] = new Z3DTexture((GLint) GL_RGBA32F, glm::uvec3(32, 32, 1), GL_RGBA, GL_FLOAT);
  g_TexId[0]->setFilter((GLint) GL_NEAREST, (GLint) GL_NEAREST);
  g_TexId[0]->uploadImage();
  g_TexId[1] = new Z3DTexture((GLint) GL_RGBA32F, glm::uvec3(32, 32, 1), GL_RGBA, GL_FLOAT);
  g_TexId[1]->setFilter((GLint) GL_NEAREST, (GLint) GL_NEAREST);
  g_TexId[1]->uploadImage();
  m_exitTarget.attachTextureToFBO(g_TexId[0], GL_COLOR_ATTACHMENT0);
  m_exitTarget.attachTextureToFBO(g_TexId[1], GL_COLOR_ATTACHMENT1);
  m_exitTarget.isFBOComplete();
  m_layerColorTexture.uploadImage();
  m_layerDepthTexture.uploadImage();
  m_layerTarget.attachTextureToFBO(&m_layerColorTexture, GL_COLOR_ATTACHMENT0, false);
  m_layerTarget.attachTextureToFBO(&m_layerDepthTexture, GL_DEPTH_ATTACHMENT, false);
  m_layerTarget.isFBOComplete();

  // ports
  addPrivateRenderTarget(m_entryTarget);
  addPrivateRenderTarget(m_exitTarget);
  addPrivateRenderTarget(m_layerTarget);
  addPort(m_outport);
  addPort(m_leftEyeOutport);
  addPort(m_rightEyeOutport);
  addPort(m_vPPort);
  addPrivateRenderPort(m_opaqueOutport);
  addPrivateRenderPort(m_opaqueLeftEyeOutport);
  addPrivateRenderPort(m_opaqueRightEyeOutport);

  addParameter(m_useFRVolumeSlice);
  addParameter(m_showXSlice);
  addParameter(m_xSlicePosition);
  addParameter(m_showYSlice);
  addParameter(m_ySlicePosition);
  addParameter(m_showZSlice);
  addParameter(m_zSlicePosition);
  addParameter(m_showXSlice2);
  addParameter(m_xSlice2Position);
  addParameter(m_showYSlice2);
  addParameter(m_ySlice2Position);
  addParameter(m_showZSlice2);
  addParameter(m_zSlice2Position);

  connect(&m_showXSlice, &ZBoolParameter::valueChanged, this, &Z3DVolumeFilter::adjustWidget);
  connect(&m_showYSlice, &ZBoolParameter::valueChanged, this, &Z3DVolumeFilter::adjustWidget);
  connect(&m_showZSlice, &ZBoolParameter::valueChanged, this, &Z3DVolumeFilter::adjustWidget);
  connect(&m_showXSlice2, &ZBoolParameter::valueChanged, this, &Z3DVolumeFilter::adjustWidget);
  connect(&m_showYSlice2, &ZBoolParameter::valueChanged, this, &Z3DVolumeFilter::adjustWidget);
  connect(&m_showZSlice2, &ZBoolParameter::valueChanged, this, &Z3DVolumeFilter::adjustWidget);

  connect(&m_xSlicePosition, &ZIntParameter::valueChanged, this, &Z3DVolumeFilter::invalidateFRVolumeXSlice);
  connect(&m_ySlicePosition, &ZIntParameter::valueChanged, this, &Z3DVolumeFilter::invalidateFRVolumeYSlice);
  connect(&m_zSlicePosition, &ZIntParameter::valueChanged, this, &Z3DVolumeFilter::invalidateFRVolumeZSlice);
  connect(&m_xSlice2Position, &ZIntParameter::valueChanged, this, &Z3DVolumeFilter::invalidateFRVolumeXSlice2);
  connect(&m_ySlice2Position, &ZIntParameter::valueChanged, this, &Z3DVolumeFilter::invalidateFRVolumeYSlice2);
  connect(&m_zSlice2Position, &ZIntParameter::valueChanged, this, &Z3DVolumeFilter::invalidateFRVolumeZSlice2);

  m_leftMouseButtonPressEvent.listenTo("trace", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_leftMouseButtonPressEvent.listenTo("trace", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  connect(&m_leftMouseButtonPressEvent, &ZEventListenerParameter::mouseEventTriggered, this, &Z3DVolumeFilter::leftMouseButtonPressed);
  addEventListener(m_leftMouseButtonPressEvent);

  m_volumeRaycasterRenderer.setLayerTarget(&m_layerTarget);
  m_volumeSliceRenderer.setLayerTarget(&m_layerTarget);
  for (size_t i = 0; i < m_maxNumOfFullResolutionVolumeSlice; ++i) {
    m_image2DRenderers.emplace_back(std::make_unique<Z3DImage2DRenderer>(m_rendererBase));
    m_image2DRenderers[i]->setLayerTarget(&m_layerTarget);
  }
  m_boundBoxLineWidth.set(1);
  m_boundBoxMode.select("Bound Box");

  addParameter(m_volumeRaycasterRenderer.compositingModePara());
  addParameter(m_volumeRaycasterRenderer.isoValuePara());
  addParameter(m_volumeRaycasterRenderer.localMIPThresholdPara());
  addParameter(m_volumeRaycasterRenderer.samplingRatePara());

  adjustWidget();

  m_numParas = m_parameters.size();
}

void Z3DVolumeFilter::setData(const ZStackDoc* doc, size_t maxVoxelNumber)
{
  if (maxVoxelNumber > 0) {
    m_maxVoxelNumber = maxVoxelNumber;
  } else {
    // directX 10 resource limit
    // 128 MB
    // directX 11 resource limit
    //min(max(128, 0.25f * (amount of dedicated VRAM)), 2048) MB
    //D3D11_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_A_TERM (128)
    //D3D11_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_B_TERM (0.25f)
    //D3D11_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_C_TERM (2048)
    size_t currentAvailableTexMem = Z3DGpuInfo::instance().dedicatedVideoMemoryMB();
    m_maxVoxelNumber =
      std::min(std::max(size_t(128), static_cast<size_t>(0.25 * currentAvailableTexMem)), size_t(2048)) * 1024 * 1024;
  }
  m_isVolumeDownsampled.set(false);
  std::vector<std::unique_ptr<Z3DVolume>> vols;
  ZStack* img = nullptr;
  if (doc) {
    img = doc->getStack();
    if (doc->hasStackData()) {
      if (doc->hasPlayer(ZStackObjectRole::ROLE_3DPAINT)) {
        readVolumesWithObject(doc, vols);
      } else {
        readVolumes(doc, vols);
      }
    } else if (doc->hasStack() && doc->hasSparseObject()) {
      if (doc->hasPlayer(ZStackObjectRole::ROLE_3DPAINT)) {
        readSparseVolumeWithObject(doc, vols);
      } else {
        readSparseVolume(doc, vols);
      }
    } else if (doc->hasStack()) {
      if (doc->hasSparseStack()) {
        readSparseStack(doc, vols);
      }
    }
  }
  LOG(INFO) << img << vols.size();
  setData(vols, img);
}

void Z3DVolumeFilter::setData(std::vector<std::unique_ptr<Z3DVolume> >& vols,
                              ZStack* img)
{
  if (m_widgetsGroup) {
    for (auto it = m_volumeRaycasterRenderer.channelVisibleParas().begin();
         it != m_volumeRaycasterRenderer.channelVisibleParas().end(); ++it) {
      m_widgetsGroup->removeChild(*it->get());
    }
    for (auto it = m_volumeRaycasterRenderer.transferFuncParas().begin();
         it != m_volumeRaycasterRenderer.transferFuncParas().end(); ++it) {
      m_widgetsGroup->removeChild(*it->get());
    }
    for (auto it = m_volumeRaycasterRenderer.texFilterModeParas().begin();
         it != m_volumeRaycasterRenderer.texFilterModeParas().end(); ++it) {
      m_widgetsGroup->removeChild(*it->get());
    }
    for (auto it = m_sliceColormaps.begin(); it != m_sliceColormaps.end(); ++it) {
      m_widgetsGroup->removeChild(*it->get());
    }
  }
  while (m_numParas < m_parameters.size()) {
    removeParameter(*m_parameters[m_numParas]);
  }

  m_imgPack = img;

  m_volumes.swap(vols);
  m_zoomInVolumes.clear();
  m_isSubVolume.set(false);

  if (m_volumes.size() > m_layerColorTexture.depth()) {
    m_layerColorTexture.setDimension(
          glm::uvec3(m_layerColorTexture.width(), m_layerColorTexture.height(), m_volumes.size()));
    m_layerColorTexture.uploadImage();
    m_layerDepthTexture.setDimension(
          glm::uvec3(m_layerDepthTexture.width(), m_layerDepthTexture.height(), m_volumes.size()));
    m_layerDepthTexture.uploadImage();
    m_layerTarget.attachTextureToFBO(&m_layerColorTexture, GL_COLOR_ATTACHMENT0, false);
    m_layerTarget.attachTextureToFBO(&m_layerDepthTexture, GL_DEPTH_ATTACHMENT, false);
    m_layerTarget.isFBOComplete();
  }

  m_sliceColormaps.clear();
  for (size_t i = 0; i < m_volumes.size(); ++i) {
    m_sliceColormaps.emplace_back(
          std::make_unique<ZColorMapParameter>(QString("Slice Channel %1 Colormap").arg(i + 1)));
    m_sliceColormaps[i]->get().create1DTexture(256);
    m_sliceColormaps[i]->get().reset(0.0, 1.0, glm::vec4(0.f), glm::vec4(m_volumes[i]->volColor(), 1.f));
  }

  volumeChanged();
  updateBoundBox();

  for (auto it = m_volumeRaycasterRenderer.channelVisibleParas().begin();
       it != m_volumeRaycasterRenderer.channelVisibleParas().end(); ++it) {
    addParameter(*it->get());
  }
  for (auto it = m_volumeRaycasterRenderer.transferFuncParas().begin();
       it != m_volumeRaycasterRenderer.transferFuncParas().end(); ++it) {
    addParameter(*it->get());
  }
  for (auto it = m_volumeRaycasterRenderer.texFilterModeParas().begin();
       it != m_volumeRaycasterRenderer.texFilterModeParas().end(); ++it) {
    addParameter(*it->get());
  }
  for (auto it = m_sliceColormaps.begin(); it != m_sliceColormaps.end(); ++it) {
    addParameter(*it->get());
  }

  if (m_widgetsGroup) {
    for (auto it = m_volumeRaycasterRenderer.channelVisibleParas().begin();
         it != m_volumeRaycasterRenderer.channelVisibleParas().end(); ++it) {
      m_widgetsGroup->addChild(*it->get(), 2);
    }
    for (auto it = m_volumeRaycasterRenderer.transferFuncParas().begin();
         it != m_volumeRaycasterRenderer.transferFuncParas().end(); ++it) {
      m_widgetsGroup->addChild(*it->get(), 3);
    }
    for (auto it = m_volumeRaycasterRenderer.texFilterModeParas().begin();
         it != m_volumeRaycasterRenderer.texFilterModeParas().end(); ++it) {
      m_widgetsGroup->addChild(*it->get(), 15);
    }
    for (auto it = m_sliceColormaps.begin(); it != m_sliceColormaps.end(); ++it) {
      m_widgetsGroup->addChild(*it->get(), 11);
    }
    m_widgetsGroup->emitWidgetsGroupChangedSignal();
  }

  invalidateResult();
}

bool Z3DVolumeFilter::openZoomInView(const glm::ivec3& volPos)
{
  if (!m_isVolumeDownsampled.get())
    return false;
  if (m_volumes.empty())
    return false;
  glm::ivec3 voldim = glm::ivec3(m_volumes[0]->cubeSize());
  if (!(volPos[0] >= 0 && volPos[0] < voldim.x && volPos[1] >= 0 && volPos[1] < voldim.y && volPos[2] >= 0 &&
        volPos[2] < voldim.z))
    return false;

  m_zoomInPos = volPos;
  if (m_zoomInViewSize.get() % 2 != 0)
    m_zoomInViewSize.set(m_zoomInViewSize.get() + 1);
  int halfsize = m_zoomInViewSize.get() / 2;
  int left = std::max(volPos[0] - halfsize + 1, 0);
  int right = std::min(volPos[0] + halfsize, int(m_imgPack->width()) - 1);
  int up = std::max(volPos[1] - halfsize + 1, 0);
  int down = std::min(volPos[1] + halfsize, int(m_imgPack->height()) - 1);
  int front = 0;
  int back = m_imgPack->depth() - 1;
  readSubVolumes(left, right, up, down, front, back);

  m_isSubVolume.set(true);
  m_isVolumeDownsampled.set(false);

  volumeChanged();
  invalidateResult();
  return true;
}

void Z3DVolumeFilter::exitZoomInView()
{
  if (m_zoomInVolumes.empty())
    return;

  m_zoomInVolumes.clear();
  m_isSubVolume.set(false);
  m_isVolumeDownsampled.set(true);

  volumeChanged();
  invalidateResult();
}

bool Z3DVolumeFilter::volumeNeedDownsample() const
{
  if (m_imgPack == NULL) {
    return false;
  }

  int maxTextureSize = 100;
  if (m_imgPack->depth() > 1)
    maxTextureSize = Z3DGpuInfo::instance().max3DTextureSize();
  else
    maxTextureSize = Z3DGpuInfo::instance().maxTextureSize();
  return m_imgPack->getVoxelNumber() * m_imgPack->channelNumber() > m_maxVoxelNumber ||
         m_imgPack->width() > maxTextureSize || m_imgPack->height() > maxTextureSize ||
         m_imgPack->depth() > maxTextureSize;
}

bool Z3DVolumeFilter::isVolumeDownsampled() const
{
  return m_isVolumeDownsampled.get();
}

std::shared_ptr<ZWidgetsGroup> Z3DVolumeFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("Volume", 1);

    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_stayOnTop, 1);
    m_widgetsGroup->addChild(m_isVolumeDownsampled, 2);
    m_widgetsGroup->addChild(m_isSubVolume, 2);
    m_widgetsGroup->addChild(m_zoomInViewSize, 2);

    for (auto it = m_volumeRaycasterRenderer.channelVisibleParas().begin();
         it != m_volumeRaycasterRenderer.channelVisibleParas().end(); ++it) {
      m_widgetsGroup->addChild(*it->get(), 2);
    }
    for (auto it = m_volumeRaycasterRenderer.transferFuncParas().begin();
         it != m_volumeRaycasterRenderer.transferFuncParas().end(); ++it) {
      m_widgetsGroup->addChild(*it->get(), 3);
    }
    m_widgetsGroup->addChild(m_volumeRaycasterRenderer.compositingModePara(), 4);
    m_widgetsGroup->addChild(m_volumeRaycasterRenderer.isoValuePara(), 4);
    m_widgetsGroup->addChild(m_volumeRaycasterRenderer.localMIPThresholdPara(), 4);
    m_widgetsGroup->addChild(m_volumeRaycasterRenderer.samplingRatePara(), 15);
    for (auto it = m_volumeRaycasterRenderer.texFilterModeParas().begin();
         it != m_volumeRaycasterRenderer.texFilterModeParas().end(); ++it) {
      m_widgetsGroup->addChild(*it->get(), 15);
    }

    m_widgetsGroup->addChild(m_xCut, 12);
    m_widgetsGroup->addChild(m_yCut, 12);
    m_widgetsGroup->addChild(m_zCut, 12);
    m_widgetsGroup->addChild(m_boundBoxMode, 13);
    m_widgetsGroup->addChild(m_boundBoxLineWidth, 13);
    m_widgetsGroup->addChild(m_boundBoxLineColor, 13);
    m_widgetsGroup->addChild(m_selectionLineWidth, 17);
    m_widgetsGroup->addChild(m_selectionLineColor, 17);
    m_widgetsGroup->addChild(m_interactionDownsample, 19);
    m_widgetsGroup->addChild(m_rendererBase.coordTransformPara(), 1);

    const std::vector<ZParameter*>& paras = parameters();
    for (size_t i = 0; i < paras.size(); ++i) {
      ZParameter* para = paras[i];
      if (para->name().contains("Slice") && !para->name().endsWith("2") && !para->name().endsWith("2 Position"))
        m_widgetsGroup->addChild(*para, 11);
      else if (para->name().contains("Slice"))
        m_widgetsGroup->addChild(*para, 19);
    }
    //m_widgetsGroup->setBasicAdvancedCutoff(14);
  }
  return m_widgetsGroup;
}

void Z3DVolumeFilter::enterInteractionMode()
{
  glm::uvec2 expectedSize = m_outport.expectedSize();
  if (m_interactionDownsample.get() != 1) {
    for (auto port : outputPorts()) {
      port->resize(expectedSize / uint32_t(m_interactionDownsample.get()));
    }
    for (auto port : m_privateRenderPorts) {
      port->resize(expectedSize / uint32_t(m_interactionDownsample.get()));
    }
    for (auto target : m_privateRenderTargets) {
      target->resize(expectedSize / uint32_t(m_interactionDownsample.get()));
    }

    for (auto port : inputPorts()) {
      port->setExpectedSize(expectedSize / uint32_t(m_interactionDownsample.get()));
    }
    emit requestUpstreamSizeChange(this);

    // upstream will invalidate the network, but in case there are no upstream
    // do one more invalidation
    invalidateResult();
  }
}

void Z3DVolumeFilter::exitInteractionMode()
{
  glm::uvec2 expectedSize = m_outport.expectedSize();
  if (m_interactionDownsample.get() != 1) {
    for (auto port : outputPorts()) {
      port->resize(expectedSize);
    }
    for (auto port : m_privateRenderPorts) {
      port->resize(expectedSize);
    }
    for (auto target : m_privateRenderTargets) {
      target->resize(expectedSize);
    }

    for (auto port : inputPorts()) {
      port->setExpectedSize(expectedSize);
    }
    emit requestUpstreamSizeChange(this);

    // upstream will invalidate the network, but in case there are no upstream
    // do one more invalidation
    invalidateResult();
  }
}

bool Z3DVolumeFilter::isReady(Z3DEye eye) const
{
  return Z3DBoundedFilter::isReady(eye) && isVisible() && m_imgPack;
}

glm::vec3 Z3DVolumeFilter::get3DPosition(int x, int y, int width, int height, bool& success)
{
  if (m_volumeRaycasterRenderer.compositeMode() == "Direct Volume Rendering") {
    return getMaxInten3DPositionUnderScreenPoint(x, y, width, height, success);
  } else {
    return getFirstHit3DPosition(x, y, width, height, success);
  }
}

bool Z3DVolumeFilter::hasOpaque(Z3DEye) const
{
  return hasSlices();
}

void Z3DVolumeFilter::renderOpaque(Z3DEye eye)
{
  Z3DRenderOutputPort& currentOutport = (eye == Z3DEye::Mono) ?
                                        m_opaqueOutport : (eye == Z3DEye::Left) ? m_opaqueLeftEyeOutport
                                                                                : m_opaqueRightEyeOutport;
  m_textureCopyRenderer.setColorTexture(currentOutport.colorTexture());
  m_textureCopyRenderer.setDepthTexture(currentOutport.depthTexture());
  m_rendererBase.render(eye, m_textureCopyRenderer);
}

bool Z3DVolumeFilter::hasTransparent(Z3DEye eye) const
{
  const Z3DRenderOutputPort& currentOutport = (eye == Z3DEye::Mono) ?
                                              m_outport : (eye == Z3DEye::Left) ? m_leftEyeOutport : m_rightEyeOutport;
  return currentOutport.hasValidData();
}

void Z3DVolumeFilter::renderTransparent(Z3DEye eye)
{
  Z3DRenderOutputPort& currentOutport = (eye == Z3DEye::Mono) ?
                                        m_outport : (eye == Z3DEye::Left) ? m_leftEyeOutport : m_rightEyeOutport;
  m_textureCopyRenderer.setColorTexture(currentOutport.colorTexture());
  m_textureCopyRenderer.setDepthTexture(currentOutport.depthTexture());
  m_rendererBase.render(eye, m_textureCopyRenderer);
}

ZLineSegment Z3DVolumeFilter::getScreenRay(int x, int y, int width, int height)
{
  ZLineSegment seg;

  glm::vec3 res(-1);
  glm::vec3 des(-1);
  //success = true;
  //ZStack *stack = m_stackInputPort.getFirstValidData();
  if ((m_outport.hasValidData() || m_rightEyeOutport.hasValidData())) {
    glm::ivec2 pos2D = glm::ivec2(x, height - y);
    // ??
//    Z3DRenderOutputPort &port =
//        m_outport.hasValidData() ? m_outport : m_rightEyeOutport;
//    if (port.size() == port.expectedSize() / m_interactionDownsample.get()) {
//      pos2D /= m_interactionDownsample.get();
//      width /= m_interactionDownsample.get();
//      height /= m_interactionDownsample.get();
//    }
    glm::vec3 fpos3D = get3DPosition(pos2D, 0.5, width, height);
    res = glm::applyMatrix(m_volumes[0]->worldToPhysicalMatrix(), fpos3D);
#ifdef _DEBUG_2
    std::cout << m_volumes.getFirstValidData()->getScaleSpacing() << std::endl;
    std::cout << pos3D << std::endl;
    std::cout << res << std::endl;
#endif
    //LWARN() << pos3D;
    /*
      Cuboid_I box;
      stack->getBoundBox(&box);
      if (Cuboid_I_Hit(&box, res.x, res.y, res.z)) {
        success = true;
      }
      */

    seg.setStartPoint(res[0], res[1], res[2]);

    //if (success) {
    fpos3D = get3DPosition(pos2D, 1.0, width, height);
    des = glm::applyMatrix(m_volumes[0]->worldToPhysicalMatrix(), fpos3D);
    seg.setEndPoint(des[0], des[1], des[2]);
    //}
  }

  return seg;
}

void Z3DVolumeFilter::changeCoordTransform()
{
  if (m_volumes.empty())
    return;
  for (size_t i = 0; i < m_volumes.size(); ++i) {
    m_volumes[i]->setPhysicalToWorldMatrix(m_rendererBase.coordTransform());
  }
  for (size_t i = 0; i < m_zoomInVolumes.size(); ++i) {
    m_zoomInVolumes[i]->setPhysicalToWorldMatrix(m_rendererBase.coordTransform());
  }
  invalidateAllFRVolumeSlices();
}

void Z3DVolumeFilter::changeZoomInViewSize()
{
  if (m_zoomInVolumes.empty())
    return;
  exitZoomInView();
  openZoomInView(m_zoomInPos);
}

void Z3DVolumeFilter::adjustWidget()
{
  m_zSlicePosition.setVisible(m_showZSlice.get());
  m_ySlicePosition.setVisible(m_showYSlice.get());
  m_xSlicePosition.setVisible(m_showXSlice.get());
  m_zSlice2Position.setVisible(m_showZSlice2.get());
  m_ySlice2Position.setVisible(m_showYSlice2.get());
  m_xSlice2Position.setVisible(m_showXSlice2.get());
}

void Z3DVolumeFilter::leftMouseButtonPressed(QMouseEvent* e, int w, int h)
{
  e->ignore();
  if (!m_volumeRaycasterRenderer.hasVisibleRendering())
    return;
  // Mouse button pressed
  if (e->type() == QEvent::MouseButtonPress) {
    m_startCoord.x = e->x();
    m_startCoord.y = e->y();
    toggleInteractionMode(true, this);
    return;
  }

  if (e->type() == QEvent::MouseButtonRelease) {
    if (std::abs(e->x() - m_startCoord.x) < 2 && std::abs(m_startCoord.y - e->y()) < 2) {
      bool success;
#ifndef _QT4_
      glm::vec3 pos3D = get3DPosition(e->x() * qApp->devicePixelRatio(),
                                              e->y() * qApp->devicePixelRatio(),
                                              w * qApp->devicePixelRatio(),
                                              h * qApp->devicePixelRatio(),
                                              success);
#else
      glm::vec3 pos3D = get3DPosition(e->x(), e->y(), w, h, success);
#endif
      if (success) {
        emit pointInVolumeLeftClicked(e->pos(), glm::ivec3(pos3D), e->modifiers());
        e->accept();
      }
    }
    toggleInteractionMode(false, this);
  }
}

void Z3DVolumeFilter::invalidateFRVolumeZSlice()
{
  m_FRVolumeSlicesValidState[0] = false;
}

void Z3DVolumeFilter::invalidateFRVolumeYSlice()
{
  m_FRVolumeSlicesValidState[1] = false;
}

void Z3DVolumeFilter::invalidateFRVolumeXSlice()
{
  m_FRVolumeSlicesValidState[2] = false;
}

void Z3DVolumeFilter::invalidateFRVolumeZSlice2()
{
  m_FRVolumeSlicesValidState[3] = false;
}

void Z3DVolumeFilter::invalidateFRVolumeYSlice2()
{
  m_FRVolumeSlicesValidState[4] = false;
}

void Z3DVolumeFilter::invalidateFRVolumeXSlice2()
{
  m_FRVolumeSlicesValidState[5] = false;
}

void Z3DVolumeFilter::updateCubeSerieSlices()
{
  m_cubeSerieSlices.clear();
  Z3DVolume* volume = getVolumes()[0].get();

  glm::vec3 coordLuf = volume->physicalLUF();
  glm::vec3 coordRdb = volume->physicalRDB();
  glm::uvec3 volDim = volume->originalDimensions();
  glm::uvec3 dim = volume->dimensions();

  float xTexCoordStart = std::max(m_xCut.lowerValue(), m_xCut.minimum() + 1) / (m_xCut.maximum() - 1);
  float xTexCoordEnd = std::min(m_xCut.upperValue(), m_xCut.maximum() - 1) / (m_xCut.maximum() - 1);
  float xCoordStart = glm::mix(coordLuf.x, coordRdb.x, xTexCoordStart);
  float xCoordEnd = glm::mix(coordLuf.x, coordRdb.x, xTexCoordEnd);
  float yTexCoordStart = std::max(m_yCut.lowerValue(), m_yCut.minimum() + 1) / (m_yCut.maximum() - 1);
  float yTexCoordEnd = std::min(m_yCut.upperValue(), m_yCut.maximum() - 1) / (m_yCut.maximum() - 1.f);
  float yCoordStart = glm::mix(coordLuf.y, coordRdb.y, yTexCoordStart);
  float yCoordEnd = glm::mix(coordLuf.y, coordRdb.y, yTexCoordEnd);
  float zTexCoordStart = std::max(m_zCut.lowerValue(), m_zCut.minimum() + 1) / (m_zCut.maximum() - 1);
  float zTexCoordEnd = std::min(m_zCut.upperValue(), m_zCut.maximum() - 1) / (m_zCut.maximum() - 1);
  float zCoordStart = glm::mix(coordLuf.z, coordRdb.z, zTexCoordStart);
  float zCoordEnd = glm::mix(coordLuf.z, coordRdb.z, zTexCoordEnd);

  // it is no point to make more slices than actual texture dimension
  int numZSlice = std::ceil((m_zCut.upperValue() - m_zCut.lowerValue() - 1.0) / dim.z * volDim.z) * 2;
  int numYSlice = std::ceil((m_yCut.upperValue() - m_yCut.lowerValue() - 1.0) / dim.y * volDim.y) * 2;
  int numXSlice = std::ceil((m_xCut.upperValue() - m_xCut.lowerValue() - 1.0) / dim.x * volDim.x) * 2;
  // Z front to back
  m_cubeSerieSlices["ZF2B"] = ZMesh::CreateCubeSerieSlices(numZSlice, 2,
                                                           glm::vec3(xCoordStart, yCoordStart, zCoordStart),
                                                           glm::vec3(xCoordEnd, yCoordEnd, zCoordEnd),
                                                           glm::vec3(xTexCoordStart, yTexCoordStart, zTexCoordStart),
                                                           glm::vec3(xTexCoordEnd, yTexCoordEnd, zTexCoordEnd));
  // Z back to front
  m_cubeSerieSlices["ZB2F"] = ZMesh::CreateCubeSerieSlices(numZSlice, 2,
                                                           glm::vec3(xCoordStart, yCoordStart, zCoordEnd),
                                                           glm::vec3(xCoordEnd, yCoordEnd, zCoordStart),
                                                           glm::vec3(xTexCoordStart, yTexCoordStart, zTexCoordEnd),
                                                           glm::vec3(xTexCoordEnd, yTexCoordEnd, zTexCoordStart));
  // Y front to back
  m_cubeSerieSlices["YF2B"] = ZMesh::CreateCubeSerieSlices(numYSlice, 1,
                                                           glm::vec3(xCoordStart, yCoordStart, zCoordStart),
                                                           glm::vec3(xCoordEnd, yCoordEnd, zCoordEnd),
                                                           glm::vec3(xTexCoordStart, yTexCoordStart, zTexCoordStart),
                                                           glm::vec3(xTexCoordEnd, yTexCoordEnd, zTexCoordEnd));
  // Y back to front
  m_cubeSerieSlices["YB2F"] = ZMesh::CreateCubeSerieSlices(numYSlice, 1,
                                                           glm::vec3(xCoordStart, yCoordEnd, zCoordStart),
                                                           glm::vec3(xCoordEnd, yCoordStart, zCoordEnd),
                                                           glm::vec3(xTexCoordStart, yTexCoordEnd, zTexCoordStart),
                                                           glm::vec3(xTexCoordEnd, yTexCoordStart, zTexCoordEnd));
  // X front to back
  m_cubeSerieSlices["XF2B"] = ZMesh::CreateCubeSerieSlices(numXSlice, 0,
                                                           glm::vec3(xCoordStart, yCoordStart, zCoordStart),
                                                           glm::vec3(xCoordEnd, yCoordEnd, zCoordEnd),
                                                           glm::vec3(xTexCoordStart, yTexCoordStart, zTexCoordStart),
                                                           glm::vec3(xTexCoordEnd, yTexCoordEnd, zTexCoordEnd));
  // X back to front
  m_cubeSerieSlices["XB2F"] = ZMesh::CreateCubeSerieSlices(numXSlice, 0,
                                                           glm::vec3(xCoordEnd, yCoordStart, zCoordStart),
                                                           glm::vec3(xCoordStart, yCoordEnd, zCoordEnd),
                                                           glm::vec3(xTexCoordEnd, yTexCoordStart, zTexCoordStart),
                                                           glm::vec3(xTexCoordStart, yTexCoordEnd, zTexCoordEnd));
}

void Z3DVolumeFilter::process(Z3DEye eye)
{
  glEnable(GL_DEPTH_TEST);

  Z3DVolume* volume = getVolumes()[0].get();
  if (volume->is1DData())
    return;

  bool allCliped = m_xCut.upperValue() < m_xCut.minimum() + 1 ||
                   m_yCut.upperValue() < m_yCut.minimum() + 1 ||
                   m_zCut.upperValue() < m_zCut.minimum() + 1 ||
                   m_xCut.lowerValue() > m_xCut.maximum() - 1 ||
                   m_yCut.lowerValue() > m_yCut.maximum() - 1 ||
                   m_zCut.lowerValue() > m_zCut.maximum() - 1;

  Z3DRenderOutputPort& currentOutport = (eye == Z3DEye::Mono) ?
                                        m_outport : (eye == Z3DEye::Left) ? m_leftEyeOutport : m_rightEyeOutport;

  currentOutport.bindTarget();
  currentOutport.clearTarget();
  m_rendererBase.setViewport(currentOutport.size());

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  if (m_volumeRaycasterRenderer.hasVisibleRendering() && !allCliped) {
    prepareDataForRaycaster(volume, eye);
    m_rendererBase.render(eye, m_volumeRaycasterRenderer);
  }

  renderBoundBox(eye);

  currentOutport.releaseTarget();

  glBlendFunc(GL_ONE, GL_ZERO);
  glDisable(GL_BLEND);

  if (hasSlices()) {
    renderSlices(eye);
  }

  glDisable(GL_DEPTH_TEST);
}

bool Z3DVolumeFilter::hasSlices() const
{
  return m_showZSlice.get() || m_showXSlice.get() || m_showYSlice.get()
         || m_showXSlice2.get() || m_showYSlice2.get() || m_showZSlice2.get();
}

void Z3DVolumeFilter::renderSlices(Z3DEye eye)
{
  Z3DRenderOutputPort& currentOutport = (eye == Z3DEye::Mono) ?
                                        m_opaqueOutport : (eye == Z3DEye::Left) ? m_opaqueLeftEyeOutport
                                                                                : m_opaqueRightEyeOutport;

  currentOutport.bindTarget();
  currentOutport.clearTarget();
  m_rendererBase.setViewport(currentOutport.size());

  Z3DVolume* volume = getVolumes()[0].get();
  glm::uvec3 volDim = glm::max(glm::uvec3(2,2,2), volume->originalDimensions());
  glm::vec3 coordLuf = volume->physicalLUF();
  glm::vec3 coordRdb = volume->physicalRDB();

  if (m_useFRVolumeSlice.get() && volume->isDownsampledVolume()) {
    std::vector<Z3DPrimitiveRenderer*> renderers;

    ZStack* zstack = m_imgPack;
    const std::vector<ZVec3Parameter*>& chCols = zstack->channelColors();
    int maxTextureSize = Z3DGpuInfo::instance().maxTextureSize();

    size_t sliceRendererIdx = 0;
    if (m_showZSlice.get()) {
      if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
        m_image2DRenderers[sliceRendererIdx]->clearQuads();
        m_FRVolumeSlices[sliceRendererIdx].clear();

        float zTexCoord = m_zSlicePosition.get() / static_cast<float>(volDim.z - 1);
        float zCoord = glm::mix(coordLuf.z, coordRdb.z, zTexCoord);

        for (int c=0; c<zstack->channelNumber(); ++c) {
          Stack* croped = C_Stack::crop(zstack->c_stack(c), 0, 0, m_zSlicePosition.get(),
                                        zstack->width(), zstack->height(), 1, NULL);
          if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
            Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                              std::min(maxTextureSize, croped->height), 1);
            C_Stack::kill(croped);
            croped = croped_1;
          }
          Z3DVolume *vh = new Z3DVolume(croped);
          vh->setVolColor(chCols[c]->get());
          m_FRVolumeSlices[sliceRendererIdx].emplace_back(vh);
        }
        m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx], m_sliceColormaps);

        ZMesh slice = ZMesh::CreateCubeSliceWith2DTexture(zCoord, 2, coordLuf.xy(), coordRdb.xy());
        slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
        m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
        m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
      }
      renderers.push_back(m_image2DRenderers[sliceRendererIdx].get());
    }
    sliceRendererIdx = 1;
    if (m_showYSlice.get()) {
      if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
        m_image2DRenderers[sliceRendererIdx]->clearQuads();
        m_FRVolumeSlices[sliceRendererIdx].clear();

        float yTexCoord = m_ySlicePosition.get() / static_cast<float>(volDim.y - 1);
        float yCoord = glm::mix(coordLuf.y, coordRdb.y, yTexCoord);

        for (int c=0; c<zstack->channelNumber(); ++c) {
          Stack* croped = C_Stack::crop(zstack->c_stack(c), 0, m_ySlicePosition.get(), 0,
                                        zstack->width(), 1, zstack->depth(), NULL);
          croped->height = zstack->depth();
          croped->depth = 1;
          if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
            Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                              std::min(maxTextureSize, croped->height), 1);
            C_Stack::kill(croped);
            croped = croped_1;
          }
          Z3DVolume *vh = new Z3DVolume(croped);
          vh->setVolColor(chCols[c]->get());
          m_FRVolumeSlices[sliceRendererIdx].emplace_back(vh);
        }
        m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx], m_sliceColormaps);

        ZMesh slice = ZMesh::CreateCubeSliceWith2DTexture(yCoord, 1, coordLuf.xz(), coordRdb.xz());
        slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
        m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
        m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
      }
      renderers.push_back(m_image2DRenderers[sliceRendererIdx].get());
    }
    sliceRendererIdx = 2;
    if (m_showXSlice.get()) {
      if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
        m_image2DRenderers[sliceRendererIdx]->clearQuads();
        m_FRVolumeSlices[sliceRendererIdx].clear();

        float xTexCoord = m_xSlicePosition.get() / static_cast<float>(volDim.x - 1);
        float xCoord = glm::mix(coordLuf.x, coordRdb.x, xTexCoord);

        for (int c=0; c<zstack->channelNumber(); ++c) {
          Stack* croped = C_Stack::crop(zstack->c_stack(c), m_xSlicePosition.get(), 0, 0,
                                        1, zstack->height(), zstack->depth(), NULL);
          croped->width = zstack->height();
          croped->height = zstack->depth();
          croped->depth = 1;
          if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
            Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                              std::min(maxTextureSize, croped->height), 1);
            C_Stack::kill(croped);
            croped = croped_1;
          }
          Z3DVolume *vh = new Z3DVolume(croped);
          vh->setVolColor(chCols[c]->get());
          m_FRVolumeSlices[sliceRendererIdx].emplace_back(vh);
        }
        m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx], m_sliceColormaps);

        ZMesh slice = ZMesh::CreateCubeSliceWith2DTexture(xCoord, 0, coordLuf.yz(), coordRdb.yz());
        slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
        m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
        m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
      }
      renderers.push_back(m_image2DRenderers[sliceRendererIdx].get());
    }
    sliceRendererIdx = 3;
    if (m_showZSlice2.get()) {
      if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
        m_image2DRenderers[sliceRendererIdx]->clearQuads();
        m_FRVolumeSlices[sliceRendererIdx].clear();

        float zTexCoord = m_zSlice2Position.get() / static_cast<float>(volDim.z - 1);
        float zCoord = glm::mix(coordLuf.z, coordRdb.z, zTexCoord);

        for (int c=0; c<zstack->channelNumber(); ++c) {
          Stack* croped = C_Stack::crop(zstack->c_stack(c), 0, 0, m_zSlice2Position.get(),
                                        zstack->width(), zstack->height(), 1, NULL);
          if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
            Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                              std::min(maxTextureSize, croped->height), 1);
            C_Stack::kill(croped);
            croped = croped_1;
          }
          Z3DVolume *vh = new Z3DVolume(croped);
          vh->setVolColor(chCols[c]->get());
          m_FRVolumeSlices[sliceRendererIdx].emplace_back(vh);
        }
        m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx], m_sliceColormaps);

        ZMesh slice = ZMesh::CreateCubeSliceWith2DTexture(zCoord, 2, coordLuf.xy(), coordRdb.xy());
        slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
        m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
        m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
      }
      renderers.push_back(m_image2DRenderers[sliceRendererIdx].get());
    }
    sliceRendererIdx = 4;
    if (m_showYSlice2.get()) {
      if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
        m_image2DRenderers[sliceRendererIdx]->clearQuads();
        m_FRVolumeSlices[sliceRendererIdx].clear();

        float yTexCoord = m_ySlice2Position.get() / static_cast<float>(volDim.y - 1);
        float yCoord = glm::mix(coordLuf.y, coordRdb.y, yTexCoord);

        for (int c=0; c<zstack->channelNumber(); ++c) {
          Stack* croped = C_Stack::crop(zstack->c_stack(c), 0, m_ySlice2Position.get(), 0,
                                        zstack->width(), 1, zstack->depth(), NULL);
          croped->height = zstack->depth();
          croped->depth = 1;
          if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
            Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                              std::min(maxTextureSize, croped->height), 1);
            C_Stack::kill(croped);
            croped = croped_1;
          }
          Z3DVolume *vh = new Z3DVolume(croped);
          vh->setVolColor(chCols[c]->get());
          m_FRVolumeSlices[sliceRendererIdx].emplace_back(vh);
        }
        m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx], m_sliceColormaps);

        ZMesh slice = ZMesh::CreateCubeSliceWith2DTexture(yCoord, 1, coordLuf.xz(), coordRdb.xz());
        slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
        m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
        m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
      }
      renderers.push_back(m_image2DRenderers[sliceRendererIdx].get());
    }
    sliceRendererIdx = 5;
    if (m_showXSlice2.get()) {
      if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
        m_image2DRenderers[sliceRendererIdx]->clearQuads();
        m_FRVolumeSlices[sliceRendererIdx].clear();

        float xTexCoord = m_xSlice2Position.get() / static_cast<float>(volDim.x - 1);
        float xCoord = glm::mix(coordLuf.x, coordRdb.x, xTexCoord);

        for (int c=0; c<zstack->channelNumber(); ++c) {
          Stack* croped = C_Stack::crop(zstack->c_stack(c), m_xSlice2Position.get(), 0, 0,
                                        1, zstack->height(), zstack->depth(), NULL);
          croped->width = zstack->height();
          croped->height = zstack->depth();
          croped->depth = 1;
          if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
            Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                              std::min(maxTextureSize, croped->height), 1);
            C_Stack::kill(croped);
            croped = croped_1;
          }
          Z3DVolume *vh = new Z3DVolume(croped);
          vh->setVolColor(chCols[c]->get());
          m_FRVolumeSlices[sliceRendererIdx].emplace_back(vh);
        }
        m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx], m_sliceColormaps);

        ZMesh slice = ZMesh::CreateCubeSliceWith2DTexture(xCoord, 0, coordLuf.yz(), coordRdb.yz());
        slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
        m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
        m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
      }
      renderers.push_back(m_image2DRenderers[sliceRendererIdx].get());
    }
    m_rendererBase.render(eye, renderers);

  } else {

    m_volumeSliceRenderer.clearQuads();

    if (m_showZSlice.get()) {
      float zTexCoord = m_zSlicePosition.get() / static_cast<float>(volDim.z - 1);
      float zCoord = glm::mix(coordLuf.z, coordRdb.z, zTexCoord);

      ZMesh slice = ZMesh::CreateCubeSlice(zCoord, zTexCoord, 2, coordLuf.xy(), coordRdb.xy());
      slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
      m_volumeSliceRenderer.addQuad(slice);
    }
    if (m_showYSlice.get()) {
      float yTexCoord = m_ySlicePosition.get() / static_cast<float>(volDim.y - 1);
      float yCoord = glm::mix(coordLuf.y, coordRdb.y, yTexCoord);

      ZMesh slice = ZMesh::CreateCubeSlice(yCoord, yTexCoord, 1, coordLuf.xz(), coordRdb.xz());
      slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
      m_volumeSliceRenderer.addQuad(slice);
    }
    if (m_showXSlice.get()) {
      float xTexCoord = m_xSlicePosition.get() / static_cast<float>(volDim.x - 1);
      float xCoord = glm::mix(coordLuf.x, coordRdb.x, xTexCoord);

      ZMesh slice = ZMesh::CreateCubeSlice(xCoord, xTexCoord, 0, coordLuf.yz(), coordRdb.yz());
      slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
      m_volumeSliceRenderer.addQuad(slice);
    }

    if (m_showZSlice2.get()) {
      float zTexCoord = m_zSlice2Position.get() / static_cast<float>(volDim.z - 1);
      float zCoord = glm::mix(coordLuf.z, coordRdb.z, zTexCoord);

      ZMesh slice = ZMesh::CreateCubeSlice(zCoord, zTexCoord, 2, coordLuf.xy(), coordRdb.xy());
      slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
      m_volumeSliceRenderer.addQuad(slice);
    }
    if (m_showYSlice2.get()) {
      float yTexCoord = m_ySlice2Position.get() / static_cast<float>(volDim.y - 1);
      float yCoord = glm::mix(coordLuf.y, coordRdb.y, yTexCoord);

      ZMesh slice = ZMesh::CreateCubeSlice(yCoord, yTexCoord, 1, coordLuf.xz(), coordRdb.xz());
      slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
      m_volumeSliceRenderer.addQuad(slice);
    }
    if (m_showXSlice2.get()) {
      float xTexCoord = m_xSlice2Position.get() / static_cast<float>(volDim.x - 1);
      float xCoord = glm::mix(coordLuf.x, coordRdb.x, xTexCoord);

      ZMesh slice = ZMesh::CreateCubeSlice(xCoord, xTexCoord, 0, coordLuf.yz(), coordRdb.yz());
      slice.transformVerticesByMatrix(volume->physicalToWorldMatrix());
      m_volumeSliceRenderer.addQuad(slice);
    }
    m_rendererBase.render(eye, m_volumeSliceRenderer);
  }

  currentOutport.releaseTarget();
}

const std::vector<std::unique_ptr<Z3DVolume>>& Z3DVolumeFilter::getVolumes() const
{
  if (m_isSubVolume.get())
    return m_zoomInVolumes;
  else
    return m_volumes;
}

void Z3DVolumeFilter::updateNotTransformedBoundBoxImpl()
{
  if (!m_volumes.empty()) {
    m_notTransformedBoundBox.setMinCorner(glm::dvec3(m_volumes[0]->parentVolPhysicalLUF()));
    m_notTransformedBoundBox.setMaxCorner(glm::dvec3(m_volumes[0]->parentVolPhysicalRDB()));
  }
}

void Z3DVolumeFilter::readSubVolumes(int left, int right, int up, int down, int front, int back)
{
  m_zoomInVolumes.clear();

  size_t nchannel = m_imgPack ? m_imgPack->channelNumber() : 0;
  if (nchannel > 0) {
    glm::vec3 downsampleSpacing = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 offset = glm::vec3(left, up, front) + m_volumes[0]->offset();
    for (size_t i = 0; i < nchannel; ++i) {
      Stack *stack = m_imgPack->c_stack(i);
      Stack *subStack = C_Stack::crop(
            stack, left, up, front, right-left+1, down-up+1, back-front+1, NULL);
      if (subStack->kind == GREY) {
        Z3DVolume *vh = new Z3DVolume(subStack, downsampleSpacing, offset,
                                      m_volumes[0]->physicalToWorldMatrix());
        vh->setParentVolumeDimensions(glm::uvec3(stack->width, stack->height, stack->depth));
        vh->setParentVolumeOffset(m_volumes[0]->offset());
        m_zoomInVolumes.emplace_back(vh);
      } else {
        C_Stack::translate(subStack, GREY, 1);
        Z3DVolume *vh = new Z3DVolume(subStack, downsampleSpacing, offset,
                                      m_volumes[0]->physicalToWorldMatrix());
        vh->setParentVolumeDimensions(glm::uvec3(stack->width, stack->height, stack->depth));
        vh->setParentVolumeOffset(m_volumes[0]->offset());
        m_zoomInVolumes.emplace_back(vh);
      }
    }

    std::vector<ZVec3Parameter*>& chCols = m_imgPack->channelColors();
    for (size_t i=0; i<nchannel; i++) {
      m_zoomInVolumes[i]->setVolColor(chCols[i]->get());
    }

    m_zoomInBound = m_zoomInVolumes[0]->worldBoundBox();
  }
}

glm::vec3 Z3DVolumeFilter::getFirstHit3DPosition(int x, int y, int width, int height, bool& success)
{
  glm::vec3 res(-1);
  success = false;
  if (m_volumeRaycasterRenderer.hasVisibleRendering() &&
      (m_outport.hasValidData() || m_rightEyeOutport.hasValidData())) {
    glm::ivec2 pos2D = glm::ivec2(x, height - y);
    Z3DRenderOutputPort& port = m_outport.hasValidData() ? m_outport : m_rightEyeOutport;
    if (port.size() == port.expectedSize() / uint32_t(m_interactionDownsample.get())) {
      pos2D /= m_interactionDownsample.get();
      width /= m_interactionDownsample.get();
      height /= m_interactionDownsample.get();
    }
    glm::vec3 fpos3D = get3DPosition(pos2D, width, height, port);
    res = glm::round(glm::applyMatrix(getVolumes()[0]->worldToPhysicalMatrix(), fpos3D));
    if (res.x >= 0 && res.x < m_imgPack->width() &&
        res.y >= 0 && res.y < m_imgPack->height() &&
        res.z >= 0 && res.z < m_imgPack->depth()) {
      success = true;
    }
  }
  return res;
}

glm::vec3 Z3DVolumeFilter::getMaxInten3DPositionUnderScreenPoint(int x, int y, int width, int height, bool& success)
{
  glm::vec3 res(-1);
  glm::vec3 des(-1);
  success = false;
  if (m_volumeRaycasterRenderer.hasVisibleRendering() &&
      (m_outport.hasValidData() || m_rightEyeOutport.hasValidData())) {
    glm::ivec2 pos2D = glm::ivec2(x, height - y);
    Z3DRenderOutputPort& port = m_outport.hasValidData() ? m_outport : m_rightEyeOutport;
    if (port.size() == port.expectedSize() / uint32_t(m_interactionDownsample.get())) {
      pos2D /= m_interactionDownsample.get();
      width /= m_interactionDownsample.get();
      height /= m_interactionDownsample.get();
    }
    glm::vec3 fpos3D = get3DPosition(pos2D, width, height, port);
    res = glm::round(glm::applyMatrix(getVolumes()[0]->worldToPhysicalMatrix(), fpos3D));
    Cuboid_I box;
    m_imgPack->getBoundBox(&box);
    if (Cuboid_I_Hit(&box, res.x, res.y, res.z)) {
      success = true;
    }
    //    if (res.x >= 0 && res.x < m_imgPack->width() &&
    //        res.y >= 0 && res.y < m_imgPack->height() &&
    //        res.z >= 0 && res.z < m_imgPack->depth()) {
    //      success = true;
    //    }

    if (success) {
      fpos3D = get3DPosition(pos2D, 1.0, width, height);
      des = glm::round(glm::applyMatrix(getVolumes()[0]->worldToPhysicalMatrix(), fpos3D));
      //LWARN() << "start" << res << "to" << des;
      if (glm::length(des - res) <= 1.f) {  // res is last pixel along current ray direction
        return res;
      }
    }
  }

  // find maximum intensity voxel start from res along des direction
  if (success) {
    double maxInten = m_imgPack->value(res.x - m_imgPack->getOffset().getX(),
                                       res.y - m_imgPack->getOffset().getY(),
                                       res.z - m_imgPack->getOffset().getZ());
    glm::vec3 p = res;
    glm::vec3 d = des - res;
    float N = std::max(std::max(std::abs(d.x), std::abs(d.y)), std::abs(d.z));
    glm::vec3 stepSize = d / N;
    while (true) {
      p = p + stepSize;
      glm::vec3 roundP = glm::round(p);

      if (!m_imgPack->contains(roundP.x, roundP.y, roundP.z)) {
        break;
      }

      /*
          if (roundP.x < 0 || roundP.x >= stack->width() ||
              roundP.y < 0 || roundP.y >= stack->height() ||
              roundP.z < 0 || roundP.z >= stack->depth()) {
            break;
          }
          */
      double inten = m_imgPack->value(roundP.x - m_imgPack->getOffset().getX(),
                                      roundP.y - m_imgPack->getOffset().getY(),
                                      roundP.z - m_imgPack->getOffset().getZ());
      if (inten > maxInten) {
        maxInten = inten;
        res = roundP;
      }
    }
    //LWARN() << "res" << res << "maxInten" << maxInten;
  }
  return res;
}

glm::vec3 Z3DVolumeFilter::get3DPosition(glm::ivec2 pos2D, int width, int height, Z3DRenderOutputPort& port)
{
  glm::mat4 projection = globalCamera().projectionMatrix(Z3DEye::Mono);
  glm::mat4 modelview = globalCamera().viewMatrix(Z3DEye::Mono);

  glm::ivec4 viewport;
  viewport[0] = 0;
  viewport[1] = 0;
  viewport[2] = width;
  viewport[3] = height;

  GLfloat WindowPosZ;
  port.bindTarget();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(pos2D.x, pos2D.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &WindowPosZ);
  port.releaseTarget();

  glm::vec3 pos = glm::unProject(glm::vec3(pos2D.x, pos2D.y, WindowPosZ), modelview,
                                 projection, viewport);

  return pos;
}

glm::vec3 Z3DVolumeFilter::get3DPosition(glm::ivec2 pos2D, double depth, int width, int height)
{
  glm::mat4 projection = globalCamera().projectionMatrix(Z3DEye::Mono);
  glm::mat4 modelview = globalCamera().viewMatrix(Z3DEye::Mono);

  glm::ivec4 viewport;
  viewport[0] = 0;
  viewport[1] = 0;
  viewport[2] = width;
  viewport[3] = height;

  glm::vec3 pos = glm::unProject(glm::vec3(pos2D.x, pos2D.y, depth), modelview,
                                 projection, viewport);

  return pos;
}

void Z3DVolumeFilter::prepareDataForRaycaster(Z3DVolume* volume, Z3DEye eye)
{
  if (!m_volumeRaycasterRenderer.hasVisibleRendering())
    return;

  glm::vec3 coordLuf = volume->physicalLUF();
  glm::vec3 coordRdb = volume->physicalRDB();

  float xTexCoordStart = std::max(m_xCut.lowerValue(), m_xCut.minimum() + 1) / (m_xCut.maximum() - 1);
  float xTexCoordEnd = std::min(m_xCut.upperValue(), m_xCut.maximum() - 1) / (m_xCut.maximum() - 1);
  float xCoordStart = glm::mix(coordLuf.x, coordRdb.x, xTexCoordStart);
  float xCoordEnd = glm::mix(coordLuf.x, coordRdb.x, xTexCoordEnd);
  float yTexCoordStart = std::max(m_yCut.lowerValue(), m_yCut.minimum() + 1) / (m_yCut.maximum() - 1);
  float yTexCoordEnd = std::min(m_yCut.upperValue(), m_yCut.maximum() - 1) / (m_yCut.maximum() - 1);
  float yCoordStart = glm::mix(coordLuf.y, coordRdb.y, yTexCoordStart);
  float yCoordEnd = glm::mix(coordLuf.y, coordRdb.y, yTexCoordEnd);
  float zTexCoordStart = std::max(m_zCut.lowerValue(), m_zCut.minimum() + 1) / (m_zCut.maximum() - 1);
  float zTexCoordEnd = std::min(m_zCut.upperValue(), m_zCut.maximum() - 1) / (m_zCut.maximum() - 1);
  float zCoordStart = glm::mix(coordLuf.z, coordRdb.z, zTexCoordStart);
  float zCoordEnd = glm::mix(coordLuf.z, coordRdb.z, zTexCoordEnd);

  m_2DImageQuad.clear();

  if (volume->is2DData()) { // for 2d image
    m_2DImageQuad = ZMesh::CreateImageSlice(volume->offset().z, glm::vec2(xCoordStart, yCoordStart),
                                            glm::vec2(xCoordEnd, yCoordEnd), glm::vec2(xTexCoordStart, yTexCoordStart),
                                            glm::vec2(xTexCoordEnd, yTexCoordEnd));
    m_2DImageQuad.transformVerticesByMatrix(volume->physicalToWorldMatrix());
  } else { // 3d volume but 2d slice
    if (m_zCut.lowerValue() == m_zCut.upperValue()) {
      m_2DImageQuad = ZMesh::CreateCubeSlice(zCoordStart, zTexCoordStart, 2, glm::vec2(xCoordStart, yCoordStart),
                                             glm::vec2(xCoordEnd, yCoordEnd), glm::vec2(xTexCoordStart, yTexCoordStart),
                                             glm::vec2(xTexCoordEnd, yTexCoordEnd));
      m_2DImageQuad.transformVerticesByMatrix(volume->physicalToWorldMatrix());
    } else if (m_yCut.lowerValue() == m_yCut.upperValue()) {
      m_2DImageQuad = ZMesh::CreateCubeSlice(yCoordStart, yTexCoordStart, 1, glm::vec2(xCoordStart, zCoordStart),
                                             glm::vec2(xCoordEnd, zCoordEnd), glm::vec2(xTexCoordStart, zTexCoordStart),
                                             glm::vec2(xTexCoordEnd, zTexCoordEnd));
      m_2DImageQuad.transformVerticesByMatrix(volume->physicalToWorldMatrix());
    } else if (m_xCut.lowerValue() == m_xCut.upperValue()) {
      m_2DImageQuad = ZMesh::CreateCubeSlice(xCoordStart, xTexCoordStart, 0, glm::vec2(yCoordStart, zCoordStart),
                                             glm::vec2(yCoordEnd, zCoordEnd), glm::vec2(yTexCoordStart, zTexCoordStart),
                                             glm::vec2(yTexCoordEnd, zTexCoordEnd));
      m_2DImageQuad.transformVerticesByMatrix(volume->physicalToWorldMatrix());
    }
  }

  if (!m_2DImageQuad.empty()) {
    m_volumeRaycasterRenderer.clearQuads();
    m_volumeRaycasterRenderer.addQuad(m_2DImageQuad);
    return;
  }

  //  // 3d volume MIP
  //  if (m_volumeRaycasterRenderer->isMIPRendering()) {
  //    m_volumeRaycasterRenderer->clearQuads();
  //    float thre = 0.5;
  //    if (glm::dot(m_camera.getViewVector(), glm::vec3(0,0,1)) > thre)
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["ZB2F"]);
  //    else if (glm::dot(m_camera.getViewVector(), glm::vec3(0,0,-1)) > thre)
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["ZF2B"]);
  //    else if (glm::dot(m_camera.getViewVector(), glm::vec3(0,1,0)) > thre)
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["YB2F"]);
  //    else if (glm::dot(m_camera.getViewVector(), glm::vec3(0,-1,0)) > thre)
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["YF2B"]);
  //    else if (glm::dot(m_camera.getViewVector(), glm::vec3(1,0,0)) > thre)
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["XB2F"]);
  //    else
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["XF2B"]);
  //    return;
  //  }

  // 3d volume Raycasting
  ZMesh cube = ZMesh::CreateCube(glm::vec3(xCoordStart, yCoordStart, zCoordStart),
                                 glm::vec3(xCoordEnd, yCoordEnd, zCoordEnd),
                                 glm::vec3(xTexCoordStart, yTexCoordStart, zTexCoordStart),
                                 glm::vec3(xTexCoordEnd, yTexCoordEnd, zTexCoordEnd));
  cube.transformVerticesByMatrix(volume->physicalToWorldMatrix());

  // enable culling
  glEnable(GL_CULL_FACE);

  m_rendererBase.setViewport(m_exitTarget.size());

  // render back texture
  GLenum g_drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                            GL_COLOR_ATTACHMENT1
  };
  m_exitTarget.bind();
  glDrawBuffers(2, g_drawBuffers);
  glClear(GL_COLOR_BUFFER_BIT);
  glCullFace(GL_FRONT);

  m_textureAndEyeCoordinateRenderer.setTriangleList(&cube);
  m_rendererBase.render(eye, m_textureAndEyeCoordinateRenderer);
  m_exitTarget.release();

  // render front texture
  m_entryTarget.bind();
  glDrawBuffers(2, g_drawBuffers);
  glClear(GL_COLOR_BUFFER_BIT);
  glCullFace(GL_BACK);

  float nearPlaneDistToOrigin =
    glm::dot(globalCamera().eye(), -globalCamera().viewVector()) - globalCamera().nearDist() - 0.01f;
  std::vector<glm::vec4> planes;
  planes.emplace_back(-globalCamera().viewVector(), nearPlaneDistToOrigin);
  ZMesh clipped = ZMeshUtils::clipClosedSurface(cube, planes);
  m_textureAndEyeCoordinateRenderer.setTriangleList(&clipped);
  m_rendererBase.render(eye, m_textureAndEyeCoordinateRenderer);
  m_entryTarget.release();

  // restore OpenGL state
  glCullFace(GL_BACK);
  glDisable(GL_CULL_FACE);

  m_volumeRaycasterRenderer.setEntryExitInfo(m_entryTarget.attachment(GL_COLOR_ATTACHMENT0),
                                             m_entryTarget.attachment(GL_COLOR_ATTACHMENT1),
                                             m_exitTarget.attachment(GL_COLOR_ATTACHMENT0),
                                             m_exitTarget.attachment(GL_COLOR_ATTACHMENT1));
}

void Z3DVolumeFilter::invalidateAllFRVolumeSlices()
{
  m_FRVolumeSlicesValidState.clear();
  m_FRVolumeSlicesValidState.resize(m_maxNumOfFullResolutionVolumeSlice, false);
}

void Z3DVolumeFilter::volumeChanged()
{
  if (!m_imgPack)
    return;

  Z3DVolume* volume = getVolumes()[0].get();
  bool is2DImage = (volume->is2DData());
  glm::uvec3 volDim = volume->originalDimensions();
  m_xCut.setRange(-1, volDim.x);
  m_xCut.set(m_xCut.range());
  m_yCut.setRange(-1, volDim.y);
  m_yCut.set(m_yCut.range());
  m_zCut.setRange(-1, volDim.z);
  m_zCut.set(m_zCut.range());

  m_rendererBase.setRotationCenter(glm::vec3(volDim.x - 1, volDim.y - 1, volDim.z - 1) / 2.f);

  m_zSlicePosition.setRange(0, volDim.z - 1);
  m_ySlicePosition.setRange(0, volDim.y - 1);
  m_xSlicePosition.setRange(0, volDim.x - 1);
  m_zSlice2Position.setRange(0, volDim.z - 1);
  m_ySlice2Position.setRange(0, volDim.y - 1);
  m_xSlice2Position.setRange(0, volDim.x - 1);
  invalidateAllFRVolumeSlices();
  if (is2DImage) {
    m_useFRVolumeSlice.set(false);
    m_useFRVolumeSlice.setVisible(false);
    m_showXSlice.set(false);
    m_showYSlice.set(false);
    m_showZSlice.set(false);
    m_showXSlice2.set(false);
    m_showYSlice2.set(false);
    m_showZSlice2.set(false);
    m_showXSlice.setVisible(false);
    m_showYSlice.setVisible(false);
    m_showZSlice.setVisible(false);
    m_showXSlice2.setVisible(false);
    m_showYSlice2.setVisible(false);
    m_showZSlice2.setVisible(false);
  }

  m_volumeRaycasterRenderer.setChannels(getVolumes());
  //todo
  if (!is2DImage) {
    m_volumeSliceRenderer.setData(getVolumes(), m_sliceColormaps);
  }
}

void Z3DVolumeFilter::readVolumes(const ZStackDoc* doc, std::vector<std::unique_ptr<Z3DVolume> >& vols)
{
  int nchannel = doc->hasStackData() ? doc->getStack()->channelNumber() : 0;

  int maxPossibleChannels = Z3DGpuInfo::instance().maxArrayTextureLayers();
  if (nchannel > maxPossibleChannels) {
    QMessageBox::warning(QApplication::activeWindow(), "Too many channels",
                         QString("Due to hardware limit, only first %1 channels of this image will be shown").arg(
                           maxPossibleChannels));
    nchannel = maxPossibleChannels;
  }

  if (nchannel > 0) {
    for (int i=0; i<nchannel; i++) {
      Stack *stack = doc->getStack()->c_stack(i);

      //Under deveopment
      ZPoint offset(doc->getStack()->getOffset().getX(),
                    doc->getStack()->getOffset().getY(),
                    doc->getStack()->getOffset().getZ());
      if (doc->getStack()->getVoxelNumber() * nchannel > m_maxVoxelNumber) { //Downsample big stack
        m_isVolumeDownsampled.set(true);
        double scale = std::sqrt((m_maxVoxelNumber*1.0) /
                                 (doc->getStack()->getVoxelNumber() * nchannel));
        int height = stack->height;
        int width = stack->width;
        int depth = stack->depth;

        double widthScale = 1.0;
        double heightScale = 1.0;
        double depthScale = 1.0;
        if (width > height || width > depth) {
          width *= scale;
          widthScale = scale;
        }

        if (height > width || height > depth) {
          height *= scale;
          heightScale = scale;
        }

        if (depth > width || depth > height) {
          depth *= scale;
          depthScale = scale;
        }

        int maxTextureSize = 100;
        if (stack->depth > 1)
          maxTextureSize = Z3DGpuInfo::instance().max3DTextureSize();
        else
          maxTextureSize = Z3DGpuInfo::instance().maxTextureSize();

        if (maxTextureSize > 1024) {
          maxTextureSize = 1024;
        }

        if (height > maxTextureSize) {
          double heightScale2 = (double)maxTextureSize / height;
          height = std::floor(height * heightScale2);
          heightScale *= heightScale2;
        }
        if (width > maxTextureSize) {
          double widthScale2 = (double)maxTextureSize / width;
          width = std::floor(width * widthScale2);
          widthScale *= widthScale2;
        }
        if (depth > maxTextureSize) {
          double depthScale2 = (double)maxTextureSize / depth;
          depth = std::floor(depth * depthScale2);
          depthScale *= depthScale2;
        }

        if (width == 0) {
          width = 1;
        }

        if (height == 0) {
          height = 1;
        }

        if (depth == 0) {
          depth = 1;
        }

        Stack *stack2 = C_Stack::resize(stack, width, height, depth);
        C_Stack::translate(stack2, GREY, 1);

        if (doc->getStack()->isBinary()) {
          size_t volume = Stack_Voxel_Number(stack2);
          for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
            if (stack2->array[voxelIndex] == 1) {
              stack2->array[voxelIndex] = 255;
            }
          }
        }

        Z3DVolume *vh = new Z3DVolume(
              stack2, glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
              glm::vec3(offset.x(), offset.y(), offset.z()),
              m_rendererBase.coordTransform());

        vols.emplace_back(vh);
      } else { //small stack
        double widthScale = 1.0;
        double heightScale = 1.0;
        double depthScale = 1.0;
        int height = C_Stack::height(stack);
        int width = C_Stack::width(stack);
        int depth = C_Stack::depth(stack);
        int maxTextureSize = 100;
        if (stack->depth > 1)
          maxTextureSize = Z3DGpuInfo::instance().max3DTextureSize();
        else
          maxTextureSize = Z3DGpuInfo::instance().maxTextureSize();

        if (height > maxTextureSize) {
          heightScale = (double)maxTextureSize / height;
          height = std::floor(height * heightScale);
        }
        if (width > maxTextureSize) {
          widthScale = (double)maxTextureSize / width;
          width = std::floor(width * widthScale);
        }
        if (depth > maxTextureSize) {
          depthScale = (double)maxTextureSize / depth;
          depth = std::floor(depth * depthScale);
        }
        Stack *stack2;
        if (widthScale != 1.0 || heightScale != 1.0 || depthScale != 1.0) {
          stack2 = C_Stack::resize(stack, width, height, depth);
        } else {
          stack2 = C_Stack::clone(stack);
        }

        if (stack->kind == GREY && doc->getStack()->isBinary()) {
//          size_t volume = doc->getStack()->getVoxelNumber();
          size_t volume = C_Stack::voxelNumber(stack2);
          for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
            if (stack2->array[voxelIndex] == 1) {
              stack2->array[voxelIndex] = 255;
            }
          }
        }

        C_Stack::translate(stack2, GREY, 1);

        Z3DVolume *vh = new Z3DVolume(stack2,
                                      glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
                                      glm::vec3(offset.x(),
                                                offset.y(),
                                                offset.z()),
                                      m_rendererBase.coordTransform());

        vols.emplace_back(vh);
      }
    } //for each cannel

    std::vector<ZVec3Parameter*>& chCols = doc->getStack()->channelColors();
    for (int i=0; i<nchannel; i++) {
      vols[i]->setVolColor(chCols[i]->get());
    }
  }
}

void Z3DVolumeFilter::readVolumesWithObject(const ZStackDoc* doc, std::vector<std::unique_ptr<Z3DVolume> >& vols)
{
  std::vector<Stack*> stackArray;
  //int nchannel = 1;

  stackArray.push_back(C_Stack::clone(doc->getStack()->c_stack(0)));

#if 0
  ZStack *colorStack = new ZStack(GREY, m_doc->getStack()->width(),
                                  m_doc->getStack()->height(),
                                  m_doc->getStack()->depth(), 3);
  colorStack->setOffset(m_doc->getStackOffset());
  colorStack->initChannelColors();



  C_Stack::copyValue(m_doc->getStack()->c_stack(0),
                     colorStack->c_stack(0));
  colorStack->setChannelColor(0, 1, 1, 1);
#endif

  int offset[3];
  offset[0] = doc->getStackOffset().getX();
  offset[1] = doc->getStackOffset().getY();
  offset[2] = doc->getStackOffset().getZ();

#if 0
  C_Stack::setZero(colorStack->c_stack(1));
  QColor color = colorTable.getColor(1);
  colorStack->setChannelColor(1, color.redF(), color.greenF(), color.blueF());

  C_Stack::setZero(colorStack->c_stack(2));
  color = colorTable.getColor(2);
  colorStack->setChannelColor(2, color.redF(), color.greenF(), color.blueF());
#endif

  //C_Stack::copyValue(m_doc->getStack()->c_stack(0),
  //                   colorStack->c_stack(1));
  //C_Stack::copyValue(m_doc->getStack()->c_stack(0),
  //                   colorStack->c_stack(2));

  const auto& playerList =
      doc->getPlayerList(ZStackObjectRole::ROLE_3DPAINT);
  for (auto player : playerList) {
    //player->paintStack(colorStack);
    if (player->getLabel() > 0 && player->getLabel() < 10) {
      if (player->getLabel() >= (int) stackArray.size()) {
        stackArray.push_back(C_Stack::make(
                               GREY, doc->getStack()->width(),
                               doc->getStack()->height(),
                               doc->getStack()->depth()));
        C_Stack::setZero(stackArray.back());
        //stackArray.resize(stackArray.size() + 1);
      }
      player->labelStack(stackArray[player->getLabel()], offset, 255);

      //player->labelStack(colorStack->c_stack(player->getLabel()), offset, 255);
    }
  }

  /*
  QList<ZObject3d*> &objList = m_doc->getObj3dList();
  foreach(ZObject3d *obj, objList) {
    obj->drawStack(colorStack);
  }
  */

  size_t nchannel = stackArray.size();

  size_t maxPossibleChannels = Z3DGpuInfo::instance().maxArrayTextureLayers();
  if (nchannel > maxPossibleChannels) {
    QMessageBox::warning(QApplication::activeWindow(), "Too many channels",
                         QString("Due to hardware limit, only first %1 channels of this image will be shown").arg(
                           maxPossibleChannels));
    nchannel = maxPossibleChannels;
  }

  ZLabelColorTable colorTable;
  std::vector<QColor> colorArray(nchannel);

  if (nchannel > 0)
    colorArray[0] = QColor(255, 255, 255);
  for (size_t i = 1; i < nchannel; ++i) {
    colorArray[i] = colorTable.getColor(i);
  }

  if (nchannel > 0) {
    for (size_t i=0; i<nchannel; i++) {
      //Stack *stack = colorStack->c_stack(i);
      Stack *stack = stackArray[i];

      //Under deveopment
      ZPoint offset = ZPoint(
            doc->getStack()->getOffset().getX(),
            doc->getStack()->getOffset().getY(),
            doc->getStack()->getOffset().getZ());
      if (doc->getStack()->getVoxelNumber() * nchannel > m_maxVoxelNumber) { //Downsample big stack
        m_isVolumeDownsampled.set(true);
        double scale = std::sqrt((m_maxVoxelNumber*1.0) /
                                 (doc->getStack()->getVoxelNumber() * nchannel));
        int height = (int)(stack->height * scale);
        int width = (int)(stack->width * scale);
        int depth = stack->depth;
        double widthScale = 1.0;
        double heightScale = 1.0;
        double depthScale = 1.0;
        int maxTextureSize = 100;
        if (stack->depth > 1)
          maxTextureSize = Z3DGpuInfo::instance().max3DTextureSize();
        else
          maxTextureSize = Z3DGpuInfo::instance().maxTextureSize();

        if (height > maxTextureSize) {
          heightScale = (double)maxTextureSize / height;
          height = std::floor(height * heightScale);
        }
        if (width > maxTextureSize) {
          widthScale = (double)maxTextureSize / width;
          width = std::floor(width * widthScale);
        }
        if (depth > maxTextureSize) {
          depthScale = (double)maxTextureSize / depth;
          depth = std::floor(depth * depthScale);
        }

        widthScale *= scale;
        heightScale *= scale;

        Stack *stack2 = C_Stack::resize(stack, width, height, depth);
        C_Stack::translate(stack2, GREY, 1);

        if (doc->getStack()->isBinary()) {
          size_t volume = Stack_Voxel_Number(stack2);
          for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
            if (stack2->array[voxelIndex] == 1) {
              stack2->array[voxelIndex] = 255;
            }
          }
        }

        Z3DVolume *vh = new Z3DVolume(
              stack2, glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
              glm::vec3(offset.x(), offset.y(),
                        offset.z()),
              m_rendererBase.coordTransform()
                                      /*glm::vec3(.0)*/);

        vols.emplace_back(vh);
      } else { //small stack
        double widthScale = 1.0;
        double heightScale = 1.0;
        double depthScale = 1.0;
        int height = C_Stack::height(stack);
        int width = C_Stack::width(stack);
        int depth = C_Stack::depth(stack);
        int maxTextureSize = 100;
        if (stack->depth > 1)
          maxTextureSize = Z3DGpuInfo::instance().max3DTextureSize();
        else
          maxTextureSize = Z3DGpuInfo::instance().maxTextureSize();

        if (height > maxTextureSize) {
          heightScale = (double)maxTextureSize / height;
          height = std::floor(height * heightScale);
        }
        if (width > maxTextureSize) {
          widthScale = (double)maxTextureSize / width;
          width = std::floor(width * widthScale);
        }
        if (depth > maxTextureSize) {
          depthScale = (double)maxTextureSize / depth;
          depth = std::floor(depth * depthScale);
        }
        Stack *stack2;
        if (widthScale != 1.0 || heightScale != 1.0)
          stack2 = C_Stack::resize(stack, width, height, depth);
        else
          stack2 = C_Stack::clone(stack);

        if (stack->kind == GREY && doc->getStack()->isBinary()) {
          size_t volume = doc->getStack()->getVoxelNumber();
          for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
            if (stack2->array[voxelIndex] == 1) {
              stack2->array[voxelIndex] = 255;
            }
          }
        }

        C_Stack::translate(stack2, GREY, 1);

        Z3DVolume *vh = new Z3DVolume(stack2,
                                      glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
                                      glm::vec3(offset.x(),
                                                offset.y(),
                                                offset.z()),
                                      m_rendererBase.coordTransform());

        vols.emplace_back(vh);

      }
    } //for each cannel

    //std::vector<ZVec3Parameter*>& chCols = colorStack->channelColors();
    for (size_t i=0; i<nchannel; i++) {
      QColor &color = colorArray[i];
      vols[i]->setVolColor(glm::vec3(color.redF(), color.greenF(),
                                          color.blueF()));
      //m_volumes[i]->setVolColor(chCols[i]->get());
    }
  }

  for (std::vector<Stack*>::iterator iter = stackArray.begin();
       iter != stackArray.end(); ++iter) {
    C_Stack::kill(*iter);
  }
}

void Z3DVolumeFilter::readSparseVolume(const ZStackDoc* doc, std::vector<std::unique_ptr<Z3DVolume> >& vols)
{
  ZSparseObject obj = *(doc->getSparseObjectList().front());
  QColor color = obj.getColor();
  int nchannel = 1;
//  if (color.red() != color.green() || color.green() != color.blue()) {
//    if (color.green() > 0) {
//      nchannel = 2;
//    }
//    if (color.blue() > 0) {
//      nchannel = 3;
//    }
//  }


  ZIntCuboid dataBox = obj.getBoundBox();
  ZIntPoint dsIntv = misc::getDsIntvFor3DVolume(dataBox);
//      misc::getDsIntvFor3DVolume(doc->getStack()->getBoundBox());


  int xIntv = dsIntv.getX();
  int yIntv = dsIntv.getY();
  int zIntv = dsIntv.getZ();
  /*
  int xIntv = 0;
  int yIntv = 0;
  int zIntv = 0;

  if (m_doc->getStack()->getVoxelNumber() * nchannel > m_maxVoxelNumber) { //Downsample big stack
    //m_isVolumeDownsampled.set(true);
    xIntv = 1;
    yIntv = 1;
    zIntv = 1;
  } else if (m_doc->getStack()->getVoxelNumber() * nchannel > m_maxVoxelNumber * 3) {
    xIntv = 2;
    yIntv = 2;
    zIntv = 2;
  }
*/
  int height = dataBox.getWidth();
  int width = dataBox.getHeight();
  int depth = dataBox.getDepth();

  int maxTextureSize = 100;
  if (depth > 1) {
    maxTextureSize = Z3DGpuInfo::instance().max3DTextureSize();
  } else {
    maxTextureSize = Z3DGpuInfo::instance().maxTextureSize();
  }

  if (height > maxTextureSize) {
    yIntv += height / maxTextureSize;
  }
  if (width > maxTextureSize) {
    xIntv += width / maxTextureSize;
  }
  if (depth > maxTextureSize) {
    zIntv += depth / maxTextureSize;
  }


  obj.downsampleMax(xIntv, yIntv, zIntv);
  int offset[3];

//  int rgb[3];
//  rgb[0] = color.red();
//  rgb[1] = color.green();
//  rgb[2] = color.blue();

  for (int i = 0; i < nchannel; ++i) {
    Stack *stack2 = obj.toStack(offset, 255);

    ZPoint finalOffset;
    finalOffset.set(offset[0] * (xIntv + 1),
        offset[1] * (yIntv + 1),
        offset[2] * (zIntv + 1));

#ifdef _DEBUG_2
    C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack2);
#endif

    Z3DVolume *vh = new Z3DVolume(
          stack2, glm::vec3(xIntv + 1, yIntv + 1, zIntv + 1),
          glm::vec3(finalOffset.x(), finalOffset.y(), finalOffset.z()),
          m_rendererBase.coordTransform());

    vols.emplace_back(vh);
  }

  vols[0]->setVolColor(glm::vec3(color.redF(), color.greenF(), color.blueF()));

//  vols[0]->setVolColor(glm::vec3(1.f,0.f,0.f));
//  if (vols.size() > 1) {
//    vols[1]->setVolColor(glm::vec3(0.f,1.f,0.f));
//  }
//  if (vols.size() > 2) {
//    vols[2]->setVolColor(glm::vec3(0.f,0.f,1.f));
//  }
}

void Z3DVolumeFilter::readSparseVolumeWithObject(const ZStackDoc* doc, std::vector<std::unique_ptr<Z3DVolume> >& vols)
{
  ZSparseObject obj = *(doc->getSparseObjectList().front());
  QColor color = obj.getColor();
  int nchannel = 3;

  ZIntPoint dsIntv =
      misc::getDsIntvFor3DVolume(doc->getStack()->getBoundBox());


  int xIntv = dsIntv.getX();
  int yIntv = dsIntv.getY();
  int zIntv = dsIntv.getZ();
  /*
  if (m_doc->getStack()->getVoxelNumber() * nchannel > m_maxVoxelNumber) { //Downsample big stack
    //m_isVolumeDownsampled.set(true);
    xIntv = 1;
    yIntv = 1;
    zIntv = 1;
  }
  */

  int height = doc->getStack()->width();
  int width = doc->getStack()->height();
  int depth = doc->getStack()->depth();

  int maxTextureSize = 100;
  if (depth > 1) {
    maxTextureSize = Z3DGpuInfo::instance().max3DTextureSize();
  } else {
    maxTextureSize = Z3DGpuInfo::instance().maxTextureSize();
  }

  if (height > maxTextureSize) {
    yIntv += height / maxTextureSize;
  }
  if (width > maxTextureSize) {
    xIntv += width / maxTextureSize;
  }
  if (depth > maxTextureSize) {
    zIntv += depth / maxTextureSize;
  }


  obj.downsampleMax(xIntv, yIntv, zIntv);
  int offset[3];

  int rgb[3];
  rgb[0] = color.red();
  rgb[1] = color.green();
  rgb[2] = color.blue();

  std::vector<Stack*> stackArray(3);

  for (int i = 0; i < nchannel; ++i) {
    stackArray[i] = obj.toStack(offset, rgb[i]);

    ZPoint finalOffset;
    finalOffset.set(offset[0] * (xIntv + 1),
        offset[1] * (yIntv + 1),
        offset[2] * (zIntv + 1));

#ifdef _DEBUG_2
    C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack2);
#endif

    Z3DVolume *vh = new Z3DVolume(
          stackArray[i], glm::vec3(xIntv + 1, yIntv + 1, zIntv + 1),
          glm::vec3(finalOffset.x(), finalOffset.y(), finalOffset.z()),
          m_rendererBase.coordTransform());

    vols.emplace_back(vh);
  }

  int originalOffset[3];
  originalOffset[0] = offset[0] * (xIntv + 1);
  originalOffset[1] = offset[1] * (yIntv + 1);
  originalOffset[2] = offset[2] * (zIntv + 1);

  QList<ZObject3d*>objList = doc->getObj3dList();
  foreach(ZObject3d *obj, objList) {
    obj->drawStack(stackArray, originalOffset, xIntv, yIntv, zIntv);
  }


  vols[0]->setVolColor(glm::vec3(1.f,0.f,0.f));
  if (vols.size() > 1) {
    vols[1]->setVolColor(glm::vec3(0.f,1.f,0.f));
  }
  if (vols.size() > 2) {
    vols[2]->setVolColor(glm::vec3(0.f,0.f,1.f));
  }
}

void Z3DVolumeFilter::readSparseStack(const ZStackDoc* doc, std::vector<std::unique_ptr<Z3DVolume> >& vols)
{
  ZStackDocHelper docHelper;
  ZStack *stackData = docHelper.getSparseStack(doc);


#if 0
  ZSparseStack *spStack = m_doc->getSparseStack();
  if (spStack->getBoundBox().isEmpty()) {
    return;
  }

  const ZStack *stackData = spStack->getStack();
#endif

  if (stackData == NULL) {
    return;
  }

  size_t nchannel = stackData->channelNumber();

  size_t maxPossibleChannels = Z3DGpuInfo::instance().maxArrayTextureLayers();
  if (nchannel > maxPossibleChannels) {
    QMessageBox::warning(QApplication::activeWindow(), "Too many channels",
                         QString("Due to hardware limit, only first %1 channels of this image will be shown").arg(
                           maxPossibleChannels));
    nchannel = maxPossibleChannels;
  }

  ZIntPoint dsIntv = docHelper.getSparseStackDsIntv();
//  const ZIntPoint dsIntv = spStack->getDownsampleInterval();

  double widthScale = 1.0;
  double heightScale = 1.0;
  double depthScale = 1.0;
  if (dsIntv.getX() > 0 || dsIntv.getY() > 0 || dsIntv.getZ() > 0) {
    widthScale /= dsIntv.getX() + 1;
    heightScale /= dsIntv.getY() + 1;
    depthScale /= dsIntv.getZ() + 1;
  }
  int width = stackData->width();
  int height = stackData->height();
  int depth = stackData->depth();

  std::vector<Stack*> stackArray;

  if (nchannel > 0) {
    int maxTextureSize = 100;
    if (stackData->depth() > 1) {
      maxTextureSize = Z3DGpuInfo::instance().max3DTextureSize();
    } else {
      maxTextureSize = Z3DGpuInfo::instance().maxTextureSize();
    }

    if (height > maxTextureSize) {
      double alpha = (double) maxTextureSize / height;
      heightScale *= alpha;
      height = (int) (height * alpha);
    }
    if (width > maxTextureSize) {
      double alpha = (double) maxTextureSize / width;
      widthScale *= alpha;
      width = (int) (width * alpha);
    }
    if (depth > maxTextureSize) {
      double alpha = (double) maxTextureSize / depth;
      depthScale *= alpha;
      depth = (int) (depth * alpha);
    }

    ZIntPoint dsIntv2 = misc::getDsIntvFor3DVolume(
          ZIntCuboid(0, 0, 0, width - 1, height - 1, depth - 1));
    width /= dsIntv2.getX() + 1;
    height /= dsIntv2.getY() + 1;
    depth /= dsIntv2.getZ() + 1;

    for (size_t i=0; i<nchannel; i++) {
      const Stack *stack = stackData->c_stack(i);
      Stack *stack2 = NULL;
      if (C_Stack::width(stack) != width || C_Stack::height(stack) != height ||
          C_Stack::depth(stack) != depth) {
        m_isVolumeDownsampled.set(true);

        int xIntv = C_Stack::width(stack) / width;
        int yIntv = C_Stack::height(stack) / height;
        int zIntv = C_Stack::depth(stack) / depth;

        stack2 = Downsample_Stack_Max(stack, xIntv, yIntv, zIntv, NULL);

        widthScale = 1.0 / ((xIntv + 1) * (dsIntv.getX() + 1));
        heightScale = 1.0 / ((yIntv + 1) * (dsIntv.getY() + 1));
        depthScale = 1.0 / ((zIntv + 1) * (dsIntv.getZ() + 1));

//        stack2 = Resize_Stack(stack, width, height, depth);
      } else {
        stack2 = C_Stack::clone(stack);
      }

      C_Stack::translate(stack2, GREY, 1);

      if (C_Stack::isBinary(stack2)) {
        size_t volume = C_Stack::voxelNumber(stack2);
        for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
          if (stack2->array[voxelIndex] == 1) {
            stack2->array[voxelIndex] = 255;
          }
        }
      }

      stackArray.push_back(stack2);
    } //for each cannel

    /**********************/
    int offset[3];
    offset[0] = -stackData->getOffset().getX() * (dsIntv.getX() + 1);
    offset[1] = -stackData->getOffset().getY() * (dsIntv.getY() + 1);
    offset[2] = -stackData->getOffset().getZ() * (dsIntv.getZ() + 1);

    for (auto player : doc->getPlayerList(ZStackObjectRole::ROLE_3DPAINT)) {
      //player->paintStack(colorStack);
      if (player->getLabel() > 0 && player->getLabel() < 10) {
        if (player->getLabel() >= (int) stackArray.size()) {
          stackArray.push_back(C_Stack::make(
                                 GREY, stackData->width(),
                                 stackData->height(),
                                 stackData->depth()));
          C_Stack::setZero(stackArray.back());
          //stackArray.resize(stackArray.size() + 1);
        }
        player->labelStack(stackArray[player->getLabel()], offset, 255,
            dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
      }
    }

    for (size_t i = 0; i < stackArray.size(); ++i) {
      Z3DVolume *vh = new Z3DVolume(
            stackArray[i],
            glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
            glm::vec3(-offset[0],
                      -offset[1],
                      -offset[2]),
          m_rendererBase.coordTransform()
            /*glm::vec3(.0)*/);

      vols.emplace_back(vh);
    }

    vols[0]->setVolColor(glm::vec3(1.f,1.f,1.f));
    ZLabelColorTable colorTable;
    for (size_t i = 1; i < stackArray.size(); ++i) {
      QColor color = colorTable.getColor(i);
      vols[i]->setVolColor(
            glm::vec3(color.redF(), color.greenF(), color.blueF()));
    }
    /**********************/
  }
}

