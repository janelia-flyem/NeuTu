#include "z3drendertarget.h"

#include <QImage>
#include <QImageWriter>

#include "z3dtexture.h"
#include "logging/zqslog.h"
#include "zbenchtimer.h"
#include "z3dgpuinfo.h"


Z3DRenderTarget::Z3DRenderTarget(GLint internalColorFormat, GLint internalDepthFormat, glm::uvec2 size,
                                 bool multisample, int sample)
  : m_multisample(multisample)
  , m_samples(sample)
  , m_size(size)
{
  generateId();

  createColorAttachment(internalColorFormat);
  createDepthAttachment(internalDepthFormat);
  isFBOComplete();
}

Z3DRenderTarget::Z3DRenderTarget(glm::uvec2 size)
  : m_size(size)
{
  generateId();
}

Z3DRenderTarget::~Z3DRenderTarget()
{
#ifdef CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
  CHECK(m_context == Z3DContext());
#endif
  //  if (m_multisample) {
  //    glDeleteRenderbuffers(1, &m_colorBufferID);
  //    glDeleteRenderbuffers(1, &m_depthBufferID);
  //    glDeleteFramebuffers(1, &m_multisampleFBOID);
  //  }
  glDeleteFramebuffers(1, &m_fboID);
}

void Z3DRenderTarget::createColorAttachment(GLint internalColorFormat, GLenum attachment)
{
  std::unique_ptr<Z3DTexture> colorTex;

  glm::uvec3 size3(m_size, 1);
  colorTex.reset(new Z3DTexture(internalColorFormat, size3, GL_RGBA, GL_FLOAT));
  colorTex->uploadImage();

  attachTextureToFBO(colorTex.release(), attachment);

  //  if (m_multisample) {
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_colorBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_colorTex->getInternalFormat(),
  //                                     m_colorTex->getWidth(), m_colorTex->getHeight());
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_depthTex->getInternalFormat(),
  //                                     m_depthTex->getWidth(), m_depthTex->getHeight());
  //    //It's time to attach the RBs to the FBO
  //    glBindFramebuffer(GL_FRAMEBUFFER, m_multisampleFBOID);
  //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorBufferID);
  //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferID);
  //  }
}

void Z3DRenderTarget::createDepthAttachment(GLint internalDepthFormat)
{
  std::unique_ptr<Z3DTexture> depthTex;

  glm::uvec3 size3(m_size, 1);
  depthTex.reset(new Z3DTexture(internalDepthFormat, size3, GL_DEPTH_COMPONENT, GL_FLOAT));
  depthTex->uploadImage();

  attachTextureToFBO(depthTex.release(), GL_DEPTH_ATTACHMENT);

  //  if (m_multisample) {
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_colorBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_colorTex->getInternalFormat(),
  //                                     m_colorTex->getWidth(), m_colorTex->getHeight());
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_depthTex->getInternalFormat(),
  //                                     m_depthTex->getWidth(), m_depthTex->getHeight());
  //    //It's time to attach the RBs to the FBO
  //    glBindFramebuffer(GL_FRAMEBUFFER, m_multisampleFBOID);
  //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorBufferID);
  //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferID);
  //  }
}

void Z3DRenderTarget::bind()
{
#ifdef CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
  CHECK(m_context == Z3DContext());
#endif
  if (isBound())
    return;
  m_previousDrawFBOID = currentBoundDrawFBO();
  m_previousReadFBOID = currentBoundReadFBO();
  glGetIntegerv(GL_VIEWPORT, &m_previousViewport[0]);

  glViewport(0, 0, m_size.x, m_size.y);
  if (!m_multisample)
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
  //else
  //glBindFramebuffer(GL_FRAMEBUFFER, m_multisampleFBOID);
}

void Z3DRenderTarget::release()
{
#ifdef CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
  CHECK(m_context == Z3DContext());
#endif
  //  if (m_multisample) {
  //    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_multisampleFBOID);
  //    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
  //    glBlitFramebuffer(0, 0, m_colorTex->getWidth(), m_colorTex->getHeight(), 0, 0,
  //                      m_colorTex->getWidth(), m_colorTex->getHeight(), GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT,
  //                      GL_LINEAR);
  //  }
  //LOG(INFO) << m_previousDrawFBOID << " " << m_previousReadFBOID;
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_previousReadFBOID);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_previousDrawFBOID);
  glViewport(m_previousViewport.x, m_previousViewport.y,
             m_previousViewport.z, m_previousViewport.w);
  //glGetError(); // there should be no error according to openGL doc, but some drivers report error, ignore
}

bool Z3DRenderTarget::isBound() const
{
#ifdef CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
  CHECK(m_context == Z3DContext());
#endif
  return currentBoundDrawFBO() == m_fboID;
}

void Z3DRenderTarget::clear() const
{
  if (!isBound())
    LOG(ERROR) << "RenderTarget is not bound, can not clear.";
  else
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

GLuint Z3DRenderTarget::handle() const
{
  return m_fboID;
}

glm::vec4 Z3DRenderTarget::floatColorAtPos(const glm::ivec2& pos)
{
  bind();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glm::vec4 pixel;
  glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, GL_FLOAT, &pixel[0]);
  release();
  return pixel;
}

glm::col4 Z3DRenderTarget::colorAtPos(const glm::ivec2& pos)
{  
  bind();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glm::col4 pixel;
  glReadPixels(pos.x, pos.y, 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &pixel[0]);
  std::swap(pixel.r, pixel.b);
  release();

#ifdef _DEBUG_
  std::cout << "Color at " << pos.x << " " << pos.y << ": " << int(pixel.r) << " "
            << " " <<  int(pixel.g) << " " << int(pixel.b) << " "
            << int(pixel.a) << std::endl;
#endif

  return pixel;
}

std::vector<glm::col4> Z3DRenderTarget::colorAtPos(
    const std::vector<glm::ivec2> &posArray)
{
  bind();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  std::vector<glm::col4> pixelArray;
  for (const glm::ivec2 &pos : posArray) {
    glm::col4 pixel;
    glReadPixels(pos.x, pos.y, 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &pixel[0]);
    std::swap(pixel.r, pixel.b);
    pixelArray.push_back(pixel);
  }

  release();

  return pixelArray;
}

std::vector<glm::col4> Z3DRenderTarget::colorAtPos(
    const std::vector<std::pair<int, int> > &posArray)
{
  bind();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  std::vector<glm::col4> pixelArray;
  for (const std::pair<int,int> &pos : posArray) {
    glm::col4 pixel;
    glReadPixels(pos.first, pos.second,
                 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &pixel[0]);
    std::swap(pixel.r, pixel.b);
    pixelArray.push_back(pixel);
  }

  release();

  return pixelArray;
}

std::vector<glm::col4> Z3DRenderTarget::colorsInRect(const glm::ivec2 &p0, const glm::ivec2 &p1)
{
  glm::ivec2 min(std::min(p0.x, p1.x), std::min(p0.y, p1.y));
  glm::ivec2 max(std::max(p0.x, p1.x), std::max(p0.y, p1.y));
  int width = max.x - min.x;
  int height = max.y - min.y;

  bind();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  std::vector<glm::col4> pixels(width * height);
  glReadPixels(min.x, min.y,
               width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &pixels[0]);

  for (glm::col4 &pixel : pixels) {
    std::swap(pixel.r, pixel.b);
  }
  release();

  return pixels;
}

GLfloat Z3DRenderTarget::depthAtPos(const glm::ivec2& pos)
{
  bind();
  GLfloat res;
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(pos.x, pos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &res);
  release();
  return res;
}

bool Z3DRenderTarget::resize(const glm::uvec2& newsize)
{
  if (newsize == m_size)
    return false;
  if (newsize == glm::uvec2(0)) {
    LOG(WARNING) << "invalid size: " << newsize;
    return false;
  }
  if (newsize.x > static_cast<uint32_t>(Z3DGpuInfo::instance().maxTextureSize()) ||
      newsize.y > static_cast<uint32_t>(Z3DGpuInfo::instance().maxTextureSize())) {
    LOG(WARNING) << "size " << newsize << " exceeds texture size limit: "
                 << Z3DGpuInfo::instance().maxTextureSize();
    return false;
  }

  m_size = newsize;

  glActiveTexture(GL_TEXTURE0);
  for (const auto& enumAttach : m_attachments) {
    if (enumAttach.second) {
      enumAttach.second->setDimension(glm::uvec3(m_size.x, m_size.y, enumAttach.second->depth()));
      enumAttach.second->uploadImage();
    }
  }
  isFBOComplete();

  //  if (m_multisample) {
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_colorBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_colorTex->getInternalFormat(),
  //                                     m_colorTex->getWidth(), m_colorTex->getHeight());
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_depthTex->getInternalFormat(),
  //                                     m_depthTex->getWidth(), m_depthTex->getHeight());
  //  }
  return true;
}

void Z3DRenderTarget::changeColorAttachmentFormat(GLint internalColorFormat, GLenum attachment)
{
  if (m_attachments.find(attachment) == m_attachments.end() ||
      !m_attachments[attachment] ||
      m_attachments[attachment]->internalFormat() == internalColorFormat)
    return;

  m_attachments[attachment]->setInternalFormat(internalColorFormat);
  m_attachments[attachment]->uploadImage();

  //  if (m_multisample) {
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_colorBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_colorTex->getInternalFormat(),
  //                                     m_colorTex->getWidth(), m_colorTex->getHeight());
  //    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferID);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, std::min(m_samples, m_maxSamples),
  //                                     m_depthTex->getInternalFormat(),
  //                                     m_depthTex->getWidth(), m_depthTex->getHeight());
  //  }
}

void Z3DRenderTarget::changeDepthAttachmentFormat(GLint internalDepthFormat)
{
  if (m_attachments.find(GL_DEPTH_ATTACHMENT) == m_attachments.end() ||
      !m_attachments[GL_DEPTH_ATTACHMENT] ||
      m_attachments[GL_DEPTH_ATTACHMENT]->internalFormat() == internalDepthFormat)
    return;

  m_attachments[GL_DEPTH_ATTACHMENT]->setInternalFormat(internalDepthFormat);
  m_attachments[GL_DEPTH_ATTACHMENT]->uploadImage();
}

bool Z3DRenderTarget::isFBOComplete()
{
  bool complete = false;

  bind();

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  switch (status) {
    case GL_FRAMEBUFFER_COMPLETE:
      complete = true;
      break;
    case GL_FRAMEBUFFER_UNDEFINED:
      LOG(ERROR)
        << "GL_FRAMEBUFFER_UNDEFINED: target is the default framebuffer, but the default framebuffer does not exist.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      LOG(ERROR)
        << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: some of the framebuffer attachment points are framebuffer incomplete.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      LOG(ERROR)
        << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: framebuffer does not have at least one image attached to it.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE "
        "is GL_NONE for some color attachment point(s) named by GL_DRAWBUFFERi.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: GL_READ_BUFFER is not GL_NONE "
        "and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named "
        "by GL_READ_BUFFER.";
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      LOG(ERROR) << "GL_FRAMEBUFFER_UNSUPPORTED: the combination of internal formats of the attached images violates "
        "an implementation-dependent set of restrictions";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: the value of GL_RENDERBUFFER_SAMPLES is not the same "
        "for all attached renderbuffers; the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; "
        "or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES "
        "does not match the value of GL_TEXTURE_SAMPLES; or, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is "
        "not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, "
        "the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      LOG(ERROR)
        << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: some framebuffer attachment is layered, and some populated "
          "attachment is not layered, or all populated color attachments are not from textures of the same target.";
      break;
    default:
      LOG(ERROR) << "Unknown error!";
  }

  release();
  return complete;
}

void Z3DRenderTarget::attachTextureToFBO(Z3DTexture* texture, GLenum attachment, bool takeOwnership)
{
  CHECK(texture);
  if (m_size != texture->dimension().xy()) {
    LOG(WARNING) << "attached texture has imcompatible size with current fbo";
  }
  bind();
  switch (texture->textureTarget()) {
    case GL_TEXTURE_1D:
      glFramebufferTexture1D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_1D, texture->id(), 0);
      break;
    case GL_TEXTURE_3D:
    case GL_TEXTURE_2D_ARRAY:
      glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, attachment, texture->id(), 0, 0);
      break;
    default: //GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, ...
      glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, texture->textureTarget(), texture->id(), 0);
      break;
  }
  //LOG(INFO) << texture->getId() << " " << texture->getTextureTarget() << " " << GL_TEXTURE_RECTANGLE << " " << texture->getDimensions();
  release();
  m_attachments[attachment] = texture;
  if (takeOwnership)
    m_ownTextures.emplace(texture);
}

void Z3DRenderTarget::detach(GLenum attachment)
{
  bind();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, 0);
  release();
}

void Z3DRenderTarget::attachSlice(size_t zSlice)
{
  bind();
  for (const auto& enumAttach : m_attachments) {
    Z3DTexture* texture = enumAttach.second;
    CHECK(texture->textureTarget() == GL_TEXTURE_2D_ARRAY || texture->textureTarget() == GL_TEXTURE_3D);
    CHECK(zSlice < texture->depth());
    glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, enumAttach.first, texture->id(), 0, zSlice);
  }
  isFBOComplete();
  release();
}

GLuint Z3DRenderTarget::currentBoundDrawFBO()
{
  GLint fbo;
  //glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo);
  return static_cast<GLuint>(fbo);
}

GLuint Z3DRenderTarget::currentBoundReadFBO()
{
  GLint fbo;
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &fbo);
  return static_cast<GLuint>(fbo);
}

void Z3DRenderTarget::saveAsColorImage(const QString& filename)
{
  CHECK(!isBound());
  bind();
  try {
    GLenum dataFormat = GL_BGRA;
    GLenum dataType = GL_UNSIGNED_INT_8_8_8_8_REV;
    auto colorBuffer = std::make_unique<uint8_t[]>(
      Z3DTexture::bypePerPixel(dataFormat, dataType) * m_size.x * m_size.y);
    glReadPixels(0, 0, m_size.x, m_size.y, dataFormat, dataType, colorBuffer.get());
    QImage upsideDownImage(colorBuffer.get(), m_size.x, m_size.y, QImage::Format_ARGB32);
    QImage image = upsideDownImage.mirrored(false, true);
    QImageWriter writer(filename);
    writer.setCompression(1);
    if (!writer.write(image)) {
      LOG(ERROR) << writer.errorString();
    }
  }
  catch (ZException const& e) {
    release();
    LOG(ERROR) << "Exception: " << e.what();
  }
  release();
}

void Z3DRenderTarget::generateId()
{
  glGenFramebuffers(1, &m_fboID);
  //  if (m_samples < 2)
  //    m_multisample = false;
  //  if (m_multisample) {
  //    glGetIntegerv(GL_MAX_SAMPLES, &m_maxSamples);
  //    if (m_maxSamples > 1) {
  //      glGenFramebuffers(1, &m_multisampleFBOID);
  //      glGenRenderbuffers(1, &m_colorBufferID);
  //      glGenRenderbuffers(1, &m_depthBufferID);
  //    } else {
  //      LOG(WARNING) << "Multisample not supported?";
  //      m_multisample = false;
  //    }
  //  }
}
