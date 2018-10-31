#ifndef Z3DRENDERTARGET_H
#define Z3DRENDERTARGET_H

#include "z3dgl.h"
#include "z3dcontext.h"
#include <map>
#include <set>

class Z3DTexture;

class Z3DRenderTarget
{
public:
  // create one color and one depth attachment
  explicit Z3DRenderTarget(GLint internalColorFormat = GLint(GL_RGBA16),
                           GLint internalDepthFormat = GLint(GL_DEPTH_COMPONENT24),
                           glm::uvec2 size = glm::uvec2(32, 32),
                           bool multisample = false, int sample = 4);

  // empty fbo with no attachment
  explicit Z3DRenderTarget(glm::uvec2 size);

  virtual ~Z3DRenderTarget();

  void createColorAttachment(GLint internalColorFormat = GLint(GL_RGBA16), GLenum attachment = GL_COLOR_ATTACHMENT0);

  void createDepthAttachment(GLint internalDepthFormat = GLint(GL_DEPTH_COMPONENT24));

  void bind();

  void release();

  bool isBound() const;

  void clear() const;

  //Returns the OpenGL framebuffer object handle for this framebuffer object (returned by the glGenFrameBuffers() function).
  // returned fbo should only be used for read if multisample is used
  GLuint handle() const;

  // might crash
  const Z3DTexture* attachment(GLenum attachment) const
  { return m_attachments.at(attachment); }

  Z3DTexture* attachment(GLenum attachment)
  { return m_attachments.at(attachment); }

  //Get the color at position pos. This method will bind the RenderTarget!
  glm::vec4 floatColorAtPos(const glm::ivec2& pos);

  glm::col4 colorAtPos(const glm::ivec2& pos);
  std::vector<glm::col4> colorAtPos(const std::vector<glm::ivec2> &posArray);
  std::vector<glm::col4> colorAtPos(
      const std::vector<std::pair<int, int> > &posArray);
  std::vector<glm::col4> colorsInRect(const glm::ivec2& p0, const glm::ivec2& p1);

  GLfloat depthAtPos(const glm::ivec2& pos);

  glm::uvec2 size() const
  { return m_size; }

  bool resize(const glm::uvec2& newsize);

  void changeColorAttachmentFormat(GLint internalColorFormat, GLenum attachment = GL_COLOR_ATTACHMENT0);

  void changeDepthAttachmentFormat(GLint internalDepthFormat);

  bool isFBOComplete();

  // attach first layer miplevel 0 of texture to attachment point
  void attachTextureToFBO(Z3DTexture* texture, GLenum attachment, bool takeOwnership = true);

  void detach(GLenum attachment);

  void attachSlice(size_t zSlice);

  static GLuint currentBoundDrawFBO();

  static GLuint currentBoundReadFBO();

  void saveAsColorImage(const QString& filename);

protected:
  void generateId();

protected:
  GLuint m_fboID = 0;
  GLuint m_multisampleFBOID = 0;
  GLuint m_colorBufferID = 0;
  GLuint m_depthBufferID = 0;

  glm::ivec4 m_previousViewport;
  GLuint m_previousDrawFBOID = 0;
  GLuint m_previousReadFBOID = 0;

  std::map<GLenum, Z3DTexture*> m_attachments;
  // textures created by this rendertarget
  std::set<std::unique_ptr<Z3DTexture>> m_ownTextures;

  bool m_multisample = false;
  int m_samples = 0;
  int m_maxSamples = 0;

  glm::uvec2 m_size;

#ifdef CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS
  Z3DContext m_context;
#endif
};

#endif // Z3DRENDERTARGET_H
