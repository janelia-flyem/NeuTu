#include "z3dcanvaspainter.h"

#include "z3dgl.h"
#include "z3dcanvas.h"
#include "z3dcompositor.h"
#include "z3dgpuinfo.h"
#include "z3dtexture.h"
#include "zexception.h"
#include "zimgformat.h"
#include "QsLog.h"
#include <QImage>
#include <QImageWriter>
#include <QPainter>
#include <memory>

Z3DCanvasPainter::Z3DCanvasPainter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DBoundedFilter(globalParas, parent)
  , m_textureCopyRenderer(m_rendererBase, Z3DTextureCopyRenderer::OutputColorOption::DivideByAlpha)
  , m_canvas(nullptr)
  , m_inport("Image", false, this, State::MonoViewResultInvalid)
  , m_leftEyeInport("LeftEyeImage", false, this, State::LeftEyeResultInvalid)
  , m_rightEyeInport("RightEyeImage", false, this, State::RightEyeResultInvalid)
  , m_renderToImage(false)
{
  addPort(m_inport);
  addPort(m_leftEyeInport);
  addPort(m_rightEyeInport);
}

Z3DCanvasPainter::~Z3DCanvasPainter()
{
  setCanvas(nullptr);
}

void Z3DCanvasPainter::process(Z3DEye eye)
{
  if (!m_canvas)
    return;

  Z3DRenderInputPort& currentInport = (eye == Z3DEye::Mono) ?
                                      m_inport : (eye == Z3DEye::Left) ? m_leftEyeInport : m_rightEyeInport;

  // render to image
  if (m_renderToImage) {
    if (currentInport.isReady() && eye != Z3DEye::Left) {
      renderInportToImage(eye);
    }
    return;
  }

  // render to screen
  m_canvas->getGLFocus();
  glViewport(0, 0, m_canvas->physicalSize().x, m_canvas->physicalSize().y);
  if (eye == Z3DEye::Left) {
    glDrawBuffer(GL_BACK_LEFT);
  } else if (eye == Z3DEye::Right) {
    glDrawBuffer(GL_BACK_RIGHT);
  }

  if (currentInport.isReady()) {
    m_rendererBase.setViewport(m_canvas->physicalSize());
    m_textureCopyRenderer.setColorTexture(currentInport.colorTexture());
    m_textureCopyRenderer.setDepthTexture(currentInport.depthTexture());
    m_rendererBase.render(eye, m_textureCopyRenderer);
    CHECK_GL_ERROR
  } else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR
  }
}

bool Z3DCanvasPainter::isReady(Z3DEye /*eye*/) const
{
  return true;
}

bool Z3DCanvasPainter::isValid(Z3DEye /*eye*/) const
{
  return false;
}

void Z3DCanvasPainter::updateSize()
{
  setOutputSize(m_canvas->physicalSize());
  emit requestUpstreamSizeChange(this);
}

void Z3DCanvasPainter::onCanvasResized(size_t w, size_t h)
{
  setOutputSize(glm::uvec2(w, h));
  emit requestUpstreamSizeChange(this);
}

void Z3DCanvasPainter::invalidate(State inv)
{
  if (!m_locked) {
    m_locked = true;
    m_state |= inv;
    if (m_canvas) {
      m_canvas->updateAll();
    }
    m_locked = false;
  }
}

void Z3DCanvasPainter::setCanvas(Z3DCanvas* canvas)
{
  if (canvas == m_canvas)
    return;
  if (m_canvas)
    m_canvas->disconnect(this);

  m_canvas = canvas;
  //register at new canvas:
  if (m_canvas) {
    setOutputSize(m_canvas->physicalSize());
    emit requestUpstreamSizeChange(this);
    connect(m_canvas, &Z3DCanvas::canvasSizeChanged, this, &Z3DCanvasPainter::onCanvasResized);
  }

  invalidate();
}

Z3DCanvas* Z3DCanvasPainter::canvas() const
{
  return m_canvas;
}

bool Z3DCanvasPainter::renderToImage(const QString& filename, Z3DScreenShotType sst)
{
  if (!m_canvas) {
    LOG(WARNING) << "no canvas assigned";
    m_renderToImageError = "No canvas assigned";
    return false;
  }
  if (m_canvas->format().stereo() && sst == Z3DScreenShotType::MonoView) {
    LOG(FATAL) << "impossible configuration";
    return false;
  }

  // enable render-to-file on next process
  m_renderToImage = true;
  m_renderToImageError.clear();
  m_renderToImageType = sst;
  CHECK(m_monoImg.isEmpty() && m_leftImg.isEmpty() && m_rightImg.isEmpty());
  m_tiledRendering = false;

  // force rendering pass
  if (!m_canvas->format().stereo() && sst != Z3DScreenShotType::MonoView) {
    m_canvas->setFakeStereoOnce();
  }
  m_canvas->forceUpdate();

  // save Image
  if (m_renderToImageError.isEmpty()) {
    try {
      if (sst == Z3DScreenShotType::MonoView) {
        m_monoImg.infoRef().lastChannelIsAlphaChannel = true;
        m_monoImg.correctPreMultipliedColor().flip(Dimension::Y).save(filename);
        LOG(INFO) << "Saved rendering (" << m_monoImg.width() << ", " <<
                  m_monoImg.height() << ") to file: " << filename;
      } else if (sst == Z3DScreenShotType::FullSideBySideStereoView) {
        m_leftImg.infoRef().lastChannelIsAlphaChannel = true;
        m_rightImg.infoRef().lastChannelIsAlphaChannel = true;
        ZImg::cat(m_leftImg, m_rightImg, Dimension::X).correctPreMultipliedColor().flip(Dimension::Y).save(filename);
        LOG(INFO) << "Saved stereo rendering (" << m_leftImg.width() << " x 2, " <<
                  m_leftImg.height() << ") to file: " << filename;
      } else {
        m_leftImg.infoRef().lastChannelIsAlphaChannel = true;
        m_rightImg.infoRef().lastChannelIsAlphaChannel = true;
        ZImg::cat(m_leftImg, m_rightImg, Dimension::X).zoom(0.5, 1).correctPreMultipliedColor().flip(Dimension::Y).save(
          filename);
        LOG(INFO) << "Saved half sbs stereo rendering (" << m_leftImg.width() << ", " <<
                  m_leftImg.height() << ") to file: " << filename;
      }
    }
    catch (ZException const& e) {
      LOG(ERROR) << "Exception: " << e.what();
      m_renderToImageError = e.what();
    }
  }

  //
  m_renderToImage = false;
  m_monoImg.clear();
  m_leftImg.clear();
  m_rightImg.clear();

  m_canvas->forceUpdate();

  return (m_renderToImageError.isEmpty());
}

bool Z3DCanvasPainter::renderToImage(const QString& filename, int width, int height, Z3DScreenShotType sst,
                                     Z3DCompositor& compositor)
{
  if (!m_canvas) {
    LOG(WARNING) << "no canvas assigned";
    m_renderToImageError = "No canvas assigned";
    return false;
  }
  if (m_canvas->format().stereo() && sst == Z3DScreenShotType::MonoView) {
    LOG(FATAL) << "impossible configuration";
    return false;
  }

  if (m_inport.numValidInputs() == 0) {
    QApplication::processEvents();
  }

  glm::uvec2 oldDimensions = m_inport.size();

  // render with adjusted viewport size
  // enable render-to-file on next process
  m_renderToImage = true;
  m_renderToImageError.clear();
  m_renderToImageType = sst;
  CHECK(m_monoImg.isEmpty() && m_leftImg.isEmpty() && m_rightImg.isEmpty());

  const int tileSize = 2048;
  const int tileBorder = 128;
  const int tileInnerSize = tileSize - 2 * tileBorder;

  if (width <= tileSize && height <= tileSize) {
    // resize texture container to desired image dimensions and propagate change
    m_canvas->getGLFocus();
    setOutputSize(glm::uvec2(width, height));
    emit requestUpstreamSizeChange(this);

    m_tiledRendering = false;

    // force rendering pass
    if (!m_canvas->format().stereo() && sst != Z3DScreenShotType::MonoView) {
      m_canvas->setFakeStereoOnce();
    }
    m_canvas->forceUpdate();
  } else {
    m_canvas->getGLFocus();
    m_inport.setExpectedSize(glm::uvec2(tileSize, tileSize));
    m_leftEyeInport.setExpectedSize(glm::uvec2(tileSize, tileSize));
    m_rightEyeInport.setExpectedSize(glm::uvec2(tileSize, tileSize));
    globalCameraPara().viewportChanged(glm::uvec2(width, height));
    emit requestUpstreamSizeChange(this);

    m_tiledRendering = true;
    if (sst == Z3DScreenShotType::MonoView) {
      m_monoImg = ZImg(ZImgInfo(width, height, 1, 4));
    } else {
      m_leftImg = ZImg(ZImgInfo(width, height, 1, 4));
      m_rightImg = ZImg(ZImgInfo(width, height, 1, 4));
    }

    int numCols = (width + tileInnerSize - 1) / tileInnerSize;
    int numRows = (height + tileInnerSize - 1) / tileInnerSize;
    for (int c = 0; c < numCols; ++c) {
      for (int r = 0; r < numRows; ++r) {
        m_tileStartX = c * tileInnerSize - tileBorder;
        m_tileStartY = r * tileInnerSize - tileBorder;
        double left = m_tileStartX / 1.0 / width;
        double right = (m_tileStartX + tileSize) / 1.0 / width;
        double bottom = m_tileStartY / 1.0 / height;
        double top = (m_tileStartY + tileSize) / 1.0 / height;

        // set camera frustum
        globalCameraPara().setTileFrustum(left, right, bottom, top);
        compositor.setRenderingRegion(left, right, bottom, top);
        //LOG(INFO) << globalCameraPara().get().left() << globalCameraPara().get().right() << globalCameraPara().get().top() << globalCameraPara().get().bottom();

        //LOG(INFO) << "1";

        // force rendering pass
        if (!m_canvas->format().stereo() && sst != Z3DScreenShotType::MonoView) {
          m_canvas->setFakeStereoOnce();
        }
        //LOG(INFO) << globalCameraPara().get().left() << globalCameraPara().get().right() << globalCameraPara().get().top() << globalCameraPara().get().bottom();
        m_canvas->forceUpdate();
        //LOG(INFO) << globalCameraPara().get().left() << globalCameraPara().get().right() << globalCameraPara().get().top() << globalCameraPara().get().bottom();
        //LOG(INFO) << "2";
      }
    }
    globalCameraPara().setTileFrustum();
    compositor.setRenderingRegion();
    m_tiledRendering = false;
  }

  // save Image
  if (m_renderToImageError.isEmpty()) {
    try {
      if (sst == Z3DScreenShotType::MonoView) {
        m_monoImg.infoRef().lastChannelIsAlphaChannel = true;
        m_monoImg.correctPreMultipliedColor().flip(Dimension::Y).save(filename);
        LOG(INFO) << "Saved rendering (" << m_monoImg.width() << ", " <<
                  m_monoImg.height() << ") to file:" << filename;
      } else if (sst == Z3DScreenShotType::FullSideBySideStereoView) {
        m_leftImg.infoRef().lastChannelIsAlphaChannel = true;
        m_rightImg.infoRef().lastChannelIsAlphaChannel = true;
        ZImg::cat(m_leftImg, m_rightImg, Dimension::X).correctPreMultipliedColor().flip(Dimension::Y).save(filename);
        LOG(INFO) << "Saved stereo rendering (" << m_leftImg.width() << " x 2, " <<
                  m_leftImg.height() << ") to file: " << filename;
      } else {
        m_leftImg.infoRef().lastChannelIsAlphaChannel = true;
        m_rightImg.infoRef().lastChannelIsAlphaChannel = true;
        ZImg::cat(m_leftImg, m_rightImg, Dimension::X).zoom(0.5, 1).correctPreMultipliedColor().flip(Dimension::Y).save(
          filename);
        LOG(INFO) << "Saved half sbs stereo rendering (" << m_leftImg.width() << ", " <<
                  m_leftImg.height() << ") to file:" << filename;
      }
    }
    catch (ZException const& e) {
      LOG(ERROR) << "Exception: " << e.what();
      m_renderToImageError = e.what();
    }
  }

  //
  m_renderToImage = false;
  m_monoImg.clear();
  m_leftImg.clear();
  m_rightImg.clear();

  // reset texture container dimensions from canvas size
  setOutputSize(oldDimensions);
  emit requestUpstreamSizeChange(this);

  return (m_renderToImageError.isEmpty());
}

void Z3DCanvasPainter::renderInportToImage(Z3DEye eye)
{
  try {
    const Z3DTexture* tex = imageColorTexture(eye);
    if (!tex) {
      throw ZGLException("not ready to capture image");
    }
    GLenum dataFormat = GL_BGRA;
    GLenum dataType = GL_UNSIGNED_INT_8_8_8_8_REV;

    //    if (m_tiledRendering) {
    //      LOG(INFO) << m_tileStartX << " " << m_tileStartY;
    //    }

    if (eye == Z3DEye::Mono) {
      // get color buffer content
      std::vector<uint8_t, boost::alignment::aligned_allocator<uint8_t, 32>> colorBuffer(
        tex->bypePerPixel(dataFormat, dataType) * tex->numPixels());
      tex->downloadTextureToBuffer(dataFormat, dataType, colorBuffer.data());
      ZImg bufImg;
      bufImg.wrapData(colorBuffer.data(), ZImgInfo(tex->width(), tex->height(), 1, 4));
      if (m_tiledRendering) {
        CHECK(!m_monoImg.isEmpty());
        ZImg tmpImg = ZImg(bufImg.info());
        ZImgFormat::CXYZtoXYZC(bufImg, tmpImg, true);
        m_monoImg.pasteImg(tmpImg, ZVoxelCoordinate(m_tileStartX, m_tileStartY));
      } else {
        m_monoImg = ZImg(bufImg.info());
        ZImgFormat::CXYZtoXYZC(bufImg, m_monoImg, true);
      }
    } else if (eye == Z3DEye::Right) {
      const Z3DTexture* leftTex = imageColorTexture(Z3DEye::Left);
      if (!leftTex) {
        throw ZGLException("not ready to capture image");
      }
      std::vector<uint8_t, boost::alignment::aligned_allocator<uint8_t, 32>> colorBuffer(
        leftTex->bypePerPixel(dataFormat, dataType) * leftTex->numPixels());
      leftTex->downloadTextureToBuffer(dataFormat, dataType, colorBuffer.data());
      ZImg bufImg;
      bufImg.wrapData(colorBuffer.data(), ZImgInfo(tex->width(), tex->height(), 1, 4));
      ZImg tmpImg;
      if (m_tiledRendering) {
        CHECK(!m_leftImg.isEmpty());
        tmpImg = ZImg(bufImg.info());
        ZImgFormat::CXYZtoXYZC(bufImg, tmpImg, true);
        m_leftImg.pasteImg(tmpImg, ZVoxelCoordinate(m_tileStartX, m_tileStartY));
      } else {
        m_leftImg = ZImg(bufImg.info());
        ZImgFormat::CXYZtoXYZC(bufImg, m_leftImg, true);
      }


      tex->downloadTextureToBuffer(dataFormat, dataType, colorBuffer.data());
      bufImg.wrapData(colorBuffer.data(), ZImgInfo(tex->width(), tex->height(), 1, 4));
      if (m_tiledRendering) {
        CHECK(!m_rightImg.isEmpty());
        ZImgFormat::CXYZtoXYZC(bufImg, tmpImg, true);
        m_rightImg.pasteImg(tmpImg, ZVoxelCoordinate(m_tileStartX, m_tileStartY));
      } else {
        m_rightImg = ZImg(bufImg.info());
        ZImgFormat::CXYZtoXYZC(bufImg, m_rightImg, true);
      }
    }
  }
  catch (ZException const& e) {
    LOG(ERROR) << "Exception: " << e.what();
    m_renderToImageError = e.what();
  }
}

void Z3DCanvasPainter::setOutputSize(const glm::uvec2& size)
{
  m_inport.setExpectedSize(size);
  m_leftEyeInport.setExpectedSize(size);
  m_rightEyeInport.setExpectedSize(size);
  globalCameraPara().viewportChanged(size);
}

const Z3DTexture* Z3DCanvasPainter::imageColorTexture(Z3DEye eye) const
{
  if (eye == Z3DEye::Mono && m_inport.isReady())
    return m_inport.colorTexture();
  else if (eye == Z3DEye::Left && m_leftEyeInport.isReady())
    return m_leftEyeInport.colorTexture();
  else if (eye == Z3DEye::Right && m_rightEyeInport.isReady())
    return m_rightEyeInport.colorTexture();
  else
    return nullptr;
}

const Z3DTexture* Z3DCanvasPainter::imageDepthTexture(Z3DEye eye) const
{
  if (eye == Z3DEye::Mono && m_inport.isReady())
    return m_inport.depthTexture();
  else if (eye == Z3DEye::Left && m_leftEyeInport.isReady())
    return m_leftEyeInport.depthTexture();
  else if (eye == Z3DEye::Right && m_rightEyeInport.isReady())
    return m_rightEyeInport.depthTexture();
  else
    return nullptr;
}

QString Z3DCanvasPainter::renderToImageError() const
{
  return m_renderToImageError;
}
