#include "zglew.h"
#include "z3dcanvasrenderer.h"

#include "z3dcanvas.h"
#include "z3dtexture.h"
#include "QsLog.h"
#include "z3dgpuinfo.h"
#include "z3dtexturecopyrenderer.h"
#include "zfiletype.h"
#include "c_stack.h"
#include "zimage.h"
#include "zsharedpointer.h"

#include <QImage>
#include <QImageWriter>

Z3DCanvasRenderer::Z3DCanvasRenderer()
  : Z3DRenderProcessor()
  , m_textureCopyRenderer(NULL)
  , m_canvas(NULL)
  , m_inport("Image", false, InvalidMonoViewResult)
  , m_leftEyeInport("LeftEyeImage", false, InvalidLeftEyeResult)
  , m_rightEyeInport("RightEyeImage", false, InvalidRightEyeResult)
  , m_renderToImage(false)
  , m_renderToImageFilename("")
{
  addPort(m_inport);
  addPort(m_leftEyeInport);
  addPort(m_rightEyeInport);
}

Z3DCanvasRenderer::~Z3DCanvasRenderer() {}

void Z3DCanvasRenderer::process(Z3DEye eye)
{
  if (!m_canvas)
    return;

  Z3DRenderInputPort &currentInport = (eye == CenterEye) ?
        m_inport : (eye == LeftEye) ? m_leftEyeInport : m_rightEyeInport;

  // render to image
  if (currentInport.isReady() && m_renderToImage) {
    try {
      renderInportToImage(m_renderToImageFilename, eye);
      if (eye == CenterEye) {
        LINFO() << "Saved rendering (" << currentInport.getSize().x << "," <<
                   currentInport.getSize().y << ")" << "to file:" << m_renderToImageFilename;
      } else if (eye == RightEye) {
        if (m_renderToImageType == HalfSideBySideStereoView) {
          LINFO() << "Saved half sbs stereo rendering (" << currentInport.getSize().x << "," <<
                     currentInport.getSize().y << ")" << "to file:" << m_renderToImageFilename;
        } else {
          LINFO() << "Saved stereo rendering (" << currentInport.getSize().x << "x 2," <<
                     currentInport.getSize().y << ")" << "to file:" << m_renderToImageFilename;
        }
      }
    }
    catch (Exception const & e) {
      LERROR() << "Exception:" << e.what();
      m_renderToImageError = QString(e.what());
    }
    catch (std::exception const & e) {
      LERROR() << "std exception:" << e.what();
      m_renderToImageError = QString(e.what());
    }
    if (eye == CenterEye || eye == RightEye) {
      m_renderToImage = false;
      // now render the to screen to prevent a blank screen
      invalidate();
    }
    return;
  }

  // render to screen
  m_canvas->getGLFocus();
  glViewport(0, 0, m_canvas->getPhysicalSize().x, m_canvas->getPhysicalSize().y);
  if (eye == LeftEye)
    glDrawBuffer(GL_BACK_LEFT);
  else if (eye == RightEye)
    glDrawBuffer(GL_BACK_RIGHT);
  if (currentInport.isReady()) {
    m_rendererBase->setViewport(m_canvas->getPhysicalSize());
    m_textureCopyRenderer->setColorTexture(currentInport.getColorTexture());
    m_textureCopyRenderer->setDepthTexture(currentInport.getDepthTexture());
    m_rendererBase->render(eye);
    CHECK_GL_ERROR;
  } else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR;
  }
}

bool Z3DCanvasRenderer::isReady(Z3DEye) const
{
  return true;
}

bool Z3DCanvasRenderer::isValid(Z3DEye) const
{
  return false;
}

void Z3DCanvasRenderer::initialize()
{
  Z3DRenderProcessor::initialize();
  m_textureCopyRenderer = new Z3DTextureCopyRenderer();
  m_rendererBase->addRenderer(m_textureCopyRenderer);
  m_textureCopyRenderer->setOutputColorOption(Z3DTextureCopyRenderer::Divide_By_Alpha);
  m_rendererBase->activateRenderer(m_textureCopyRenderer);
  CHECK_GL_ERROR;
}

void Z3DCanvasRenderer::deinitialize()
{
  setCanvas(NULL);
  CHECK_GL_ERROR;

  Z3DRenderProcessor::deinitialize();
}

void Z3DCanvasRenderer::onCanvasResized(int w, int h)
{
  glm::ivec2 newsize(w, h);
  m_inport.setExpectedSize(newsize);
  m_leftEyeInport.setExpectedSize(newsize);
  m_rightEyeInport.setExpectedSize(newsize);
  emit requestUpstreamSizeChange(this);
}

void Z3DCanvasRenderer::invalidate(InvalidationState inv)
{
  m_invalidationState |= inv;
  if (m_canvas)
    m_canvas->updateAll();
}

void Z3DCanvasRenderer::setCanvas(Z3DCanvas *canvas)
{
  if (canvas == m_canvas)
    return;
  if (m_canvas)
    m_canvas->disconnect(this);

  m_canvas = canvas;
  //register at new canvas:
  if (m_canvas) {
    m_inport.setExpectedSize(m_canvas->getPhysicalSize());
    m_leftEyeInport.setExpectedSize(m_canvas->getPhysicalSize());
    m_rightEyeInport.setExpectedSize(m_canvas->getPhysicalSize());
    emit requestUpstreamSizeChange(this);
    connect(m_canvas, SIGNAL(canvasSizeChanged(int,int)), this, SLOT(onCanvasResized(int,int)));
  }

  invalidate();
}

Z3DCanvas* Z3DCanvasRenderer::getCanvas() const
{
  return m_canvas;
}

bool Z3DCanvasRenderer::renderToImage(const QString &filename, Z3DScreenShotType sst)
{
  if (!m_canvas) {
    LWARN() << "no canvas assigned";
    m_renderToImageError = "No canvas assigned";
    return false;
  }

  // enable render-to-file on next process
  m_renderToImageFilename = filename;
  m_renderToImage = true;
  m_renderToImageError.clear();
  m_renderToImageType = sst;

  // force rendering pass
  if (m_canvas->format().stereo() && sst == MonoView) {
    LERROR() << "impossible configuration";
    assert(false);
  }
  if (!m_canvas->format().stereo() && sst != MonoView)
    m_canvas->setFakeStereoOnce();
  m_canvas->forceUpdate();

  return (m_renderToImageError.isEmpty());
}

bool Z3DCanvasRenderer::renderToImage(const QString &filename, int width, int height, Z3DScreenShotType sst)
{
  if (!m_canvas) {
    LWARN() << "no canvas assigned";
    m_renderToImageError = "No canvas assigned";
    return false;
  }

  glm::ivec2 oldDimensions = m_inport.getSize();
  // resize texture container to desired image dimensions and propagate change
  m_canvas->getGLFocus();
  m_inport.setExpectedSize(glm::ivec2(width, height));
  m_leftEyeInport.setExpectedSize(glm::ivec2(width, height));
  m_rightEyeInport.setExpectedSize(glm::ivec2(width, height));
  emit requestUpstreamSizeChange(this);

  // render with adjusted viewport size
  bool success = renderToImage(filename, sst);

  // reset texture container dimensions from canvas size
  m_inport.setExpectedSize(oldDimensions);
  m_leftEyeInport.setExpectedSize(oldDimensions);
  m_rightEyeInport.setExpectedSize(oldDimensions);
  emit requestUpstreamSizeChange(this);

  return success;
}

void Z3DCanvasRenderer::renderInportToImage(const QString &filename, Z3DEye eye)
{
  const Z3DTexture* tex = getImageColorTexture(eye);
  if (!tex) {
    throw Exception("not ready to capture image");
  }
  GLenum dataFormat = GL_BGRA;
  GLenum dataType = GL_UNSIGNED_INT_8_8_8_8_REV;

  if (eye == CenterEye) {
    ZSharedPointer<uint8_t> colorBuffer(new uint8_t[tex->getBypePerPixel(dataFormat, dataType) * tex->getNumPixels()], array_deleter<uint8_t>());
    tex->downloadTextureToBuffer(dataFormat, dataType, colorBuffer.get());
    QImage upsideDownImage(colorBuffer.get(), tex->getWidth(), tex->getHeight(),
                           QImage::Format_ARGB32_Premultiplied);
    QImage image = upsideDownImage.mirrored(false, true);

    if (!ZImage::writeImage(image, filename)) {
      throw Exception("Image writing error");
    }
  } else if (eye == RightEye) {
    const Z3DTexture* leftTex = getImageColorTexture(LeftEye);
    if (!leftTex) {
      throw Exception("not ready to capture image");
    }
    ZSharedPointer<uint8_t> colorBuffer(new uint8_t[leftTex->getBypePerPixel(dataFormat, dataType) * leftTex->getNumPixels()], array_deleter<uint8_t>());
    leftTex->downloadTextureToBuffer(dataFormat, dataType, colorBuffer.get());
    QImage sideBySideImage(tex->getWidth() * 2, tex->getHeight(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&sideBySideImage);
    painter.scale(1, -1);
    painter.translate(0, -tex->getHeight());
    QImage upsideDownImageLeft(colorBuffer.get(), tex->getWidth(), tex->getHeight(),
                               QImage::Format_ARGB32_Premultiplied);
    painter.drawImage(0, 0, upsideDownImageLeft);
    tex->downloadTextureToBuffer(dataFormat, dataType, colorBuffer.get());
    QImage upsideDownImageRight(colorBuffer.get(), tex->getWidth(), tex->getHeight(),
                                QImage::Format_ARGB32_Premultiplied);
    painter.drawImage(tex->getWidth(), 0, upsideDownImageRight);

    if (m_renderToImageType == HalfSideBySideStereoView) {
      QImage halfSideBySideImage = sideBySideImage.scaled(
            tex->getWidth(), tex->getHeight(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      QImageWriter writer(filename);
      writer.setCompression(1);
      if(!writer.write(halfSideBySideImage)) {
        throw Exception(writer.errorString().toStdString());
      }
    } else {
      QImageWriter writer(filename);
      writer.setCompression(1);
      if(!writer.write(sideBySideImage)) {
        throw Exception(writer.errorString().toStdString());
      }
    }
  }
}

const Z3DTexture* Z3DCanvasRenderer::getImageColorTexture(Z3DEye eye) const
{
  if (eye == CenterEye && m_inport.isReady())
    return m_inport.getColorTexture();
  else if (eye == LeftEye && m_leftEyeInport.isReady())
    return m_leftEyeInport.getColorTexture();
  else if (eye == RightEye && m_rightEyeInport.isReady())
    return m_rightEyeInport.getColorTexture();
  else
    return NULL;
}

const Z3DTexture* Z3DCanvasRenderer::getImageDepthTexture(Z3DEye eye) const
{
  if (eye == CenterEye && m_inport.isReady())
    return m_inport.getDepthTexture();
  else if (eye == LeftEye && m_leftEyeInport.isReady())
    return m_leftEyeInport.getDepthTexture();
  else if (eye == RightEye && m_rightEyeInport.isReady())
    return m_rightEyeInport.getDepthTexture();
  else
    return NULL;
}

QString Z3DCanvasRenderer::getRenderToImageError() const
{
  return m_renderToImageError;
}
