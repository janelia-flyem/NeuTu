#include "z3d2dslicefilter.h"

#include <QApplication>
#include <QMessageBox>

#include "z3dgpuinfo.h"
#include "zmesh.h"
#include "logging/zqslog.h"
#include "zbenchtimer.h"
#include "zmeshutils.h"
#include "zstackdoc.h"

Z3D2DSliceFilter::Z3D2DSliceFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DBoundedFilter(globalParas, parent)
  , m_2dSliceRenderer(m_rendererBase)
  , m_textureCopyRenderer(m_rendererBase)
  , m_imgPack(nullptr)
  , m_stayOnTop("Stay On Top", false)
  , m_isSliceDownsampled("Volume Is Downsampled", false)
  , m_sliceColormap("Slice Colormap")
  , m_vPPort("VolumeFilter", this)
  , m_opaqueOutport("OpaqueImage", this)
  , m_opaqueLeftEyeOutport("OpaqueLeftEyeImage", this)
  , m_opaqueRightEyeOutport("OpaqueRightEyeImage", this)
{
  m_baseBoundBoxRenderer.setEnableMultisample(false);
  m_textureCopyRenderer.setDiscardTransparent(true);

  addParameter(m_stayOnTop);
  m_isSliceDownsampled.setEnabled(false);
  addParameter(m_isSliceDownsampled);
  connect(&m_rendererBase, &Z3DRendererBase::coordTransformChanged, this, &Z3D2DSliceFilter::changeCoordTransform);

  m_sliceColormap.get().create1DTexture(256);
  m_sliceColormap.get().reset(0.0, 1.0, glm::vec4(0.f), glm::vec4(1.f));

  // ports
  addPort(m_vPPort);
  addPrivateRenderPort(m_opaqueOutport);
  addPrivateRenderPort(m_opaqueLeftEyeOutport);
  addPrivateRenderPort(m_opaqueRightEyeOutport);

  m_boundBoxLineWidth.set(1);
  m_boundBoxMode.select("Bound Box");
}

void Z3D2DSliceFilter::addData(uint8_t* buffer, int width, int height,
                               ZIntPoint centerloc, ZPoint dim1vec, ZPoint dim2vec,
                               bool removeOtherData)
{
  CHECK(width > 1 && height > 1);

  if (removeOtherData) {
    m_volumes.clear();
    m_quads.clear();
    m_isSliceDownsampled.set(false);
  }

  Stack* stack = C_Stack::make(GREY, width, height, 1);
  std::memcpy(C_Stack::array8(stack), buffer, width * height);
  if (stack->width > Z3DGpuInfo::instance().maxTextureSize() ||
      stack->height > Z3DGpuInfo::instance().maxTextureSize()) {
    Stack* croped_1 = C_Stack::resize(stack,
                                      std::min(Z3DGpuInfo::instance().maxTextureSize(), stack->width),
                                      std::min(Z3DGpuInfo::instance().maxTextureSize(), stack->height),
                                      1);
    C_Stack::kill(stack);
    stack = croped_1;
    m_isSliceDownsampled.set(true);
  }
  Z3DVolume *vh = new Z3DVolume(stack);
  m_volumes.emplace_back(vh);

  glm::vec3 e1 = glm::normalize(glm::vec3(dim1vec.getX(), dim1vec.getY(), dim1vec.getZ()));
  glm::vec3 e2 = glm::normalize(glm::vec3(dim2vec.getX(), dim2vec.getY(), dim2vec.getZ()));
  glm::vec3 e3 = glm::cross(e1, e2);
  glm::mat4 transmat = glm::mat4(glm::vec4(e1, 0.f), glm::vec4(e2, 0.f), glm::vec4(e3, 0.f),
                                 glm::vec4(centerloc.getX(), centerloc.getY(), centerloc.getZ(), 1));
  ZMesh quad = ZMesh::CreateImageSlice(0.f, glm::vec2(0, 0), glm::vec2(width, height));
  quad.transformVerticesByMatrix(transmat);
  m_quads.push_back(quad);

  m_2dSliceRenderer.setData(m_volumes, m_quads, m_sliceColormap);

  updateBoundBox();

  invalidateResult();
}

bool Z3D2DSliceFilter::isSliceDownsampled() const
{
  return m_isSliceDownsampled.get();
}

std::shared_ptr<ZWidgetsGroup> Z3D2DSliceFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("2DSlices", 1);

    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_stayOnTop, 1);
    m_widgetsGroup->addChild(m_sliceColormap, 1);
    m_widgetsGroup->addChild(m_isSliceDownsampled, 2);

    m_widgetsGroup->addChild(m_boundBoxMode, 13);
    m_widgetsGroup->addChild(m_boundBoxLineWidth, 13);
    m_widgetsGroup->addChild(m_boundBoxLineColor, 13);
    m_widgetsGroup->addChild(m_selectionLineWidth, 17);
    m_widgetsGroup->addChild(m_selectionLineColor, 17);
    m_widgetsGroup->addChild(m_rendererBase.coordTransformPara(), 1);
  }
  return m_widgetsGroup;
}

bool Z3D2DSliceFilter::isReady(Z3DEye eye) const
{
  return Z3DBoundedFilter::isReady(eye) && isVisible() && !m_volumes.empty();
}

bool Z3D2DSliceFilter::hasOpaque(Z3DEye) const
{
  return true;
}

void Z3D2DSliceFilter::renderOpaque(Z3DEye eye)
{
  Z3DRenderOutputPort& currentOutport = (eye == Z3DEye::Mono) ?
                                        m_opaqueOutport : (eye == Z3DEye::Left) ? m_opaqueLeftEyeOutport
                                                                                : m_opaqueRightEyeOutport;
  m_textureCopyRenderer.setColorTexture(currentOutport.colorTexture());
  m_textureCopyRenderer.setDepthTexture(currentOutport.depthTexture());
  m_rendererBase.render(eye, m_textureCopyRenderer);
}

bool Z3D2DSliceFilter::hasTransparent(Z3DEye /*eye*/) const
{
  return false;
}

void Z3D2DSliceFilter::renderTransparent(Z3DEye /*eye*/)
{
}

void Z3D2DSliceFilter::changeCoordTransform()
{
  if (m_volumes.empty())
    return;
  for (size_t i = 0; i < m_volumes.size(); ++i) {
    m_volumes[i]->setPhysicalToWorldMatrix(m_rendererBase.coordTransform());
  }
}

void Z3D2DSliceFilter::process(Z3DEye eye)
{
  glEnable(GL_DEPTH_TEST);

  Z3DRenderOutputPort& currentOutport = (eye == Z3DEye::Mono) ?
                                        m_opaqueOutport : (eye == Z3DEye::Left) ? m_opaqueLeftEyeOutport
                                                                                : m_opaqueRightEyeOutport;

  currentOutport.bindTarget();
  currentOutport.clearTarget();
  m_rendererBase.setViewport(currentOutport.size());

  m_rendererBase.render(eye, m_2dSliceRenderer);

  renderBoundBox(eye);

  currentOutport.releaseTarget();

  glDisable(GL_DEPTH_TEST);
}

void Z3D2DSliceFilter::updateNotTransformedBoundBoxImpl()
{
  m_notTransformedBoundBox.reset();
  if (!m_volumes.empty()) {
    for (const auto& vol : m_volumes) {
      m_notTransformedBoundBox.setMinCorner(glm::min(m_notTransformedBoundBox.minCorner(), glm::dvec3(vol->parentVolPhysicalLUF())));
      m_notTransformedBoundBox.setMaxCorner(glm::max(m_notTransformedBoundBox.maxCorner(), glm::dvec3(vol->parentVolPhysicalRDB())));
    }
  }
}


