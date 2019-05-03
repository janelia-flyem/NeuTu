#include "z3dtexture.h"

#include <QImage>
#include <QImageWriter>

#include "logging/zqslog.h"
#include "z3dgpuinfo.h"

Z3DTexture::Z3DTexture(GLenum textureTarget, GLint internalFormat, const glm::uvec3& dimension,
                       GLenum dataFormat, GLenum dataType)
  : m_textureTarget(textureTarget)
  , m_dimension(dimension)
  , m_internalFormat(internalFormat)
  , m_dataFormat(dataFormat)
  , m_dataType(dataType)
{
  CHECK(m_dimension.x > 0 && m_dimension.y > 0 && m_dimension.z > 0);
  getType();
  glGenTextures(1, &m_id);
  setFilter();
  setWrap();
}

Z3DTexture::Z3DTexture(
    GLint internalFormat, const glm::uvec3& dimension, GLenum dataFormat, GLenum dataType)
  : m_dimension(dimension)
  , m_internalFormat(internalFormat)
  , m_dataFormat(dataFormat)
  , m_dataType(dataType)
{
  CHECK(m_dimension.x > 0 && m_dimension.y > 0 && m_dimension.z > 0);
  if (m_dimension.z > 1) {
    m_textureTarget = GL_TEXTURE_3D;
  } else if (m_dimension.y > 1) {
    m_textureTarget = GL_TEXTURE_2D;
  } else {
    m_textureTarget = GL_TEXTURE_1D;
  }
  getType();
  glGenTextures(1, &m_id);
  setFilter();
  setWrap();
}

Z3DTexture::~Z3DTexture()
{
  if (m_id) {
    glDeleteTextures(1, &m_id);
  }
}

void Z3DTexture::setFilter(GLint minFilter, GLint magFilter) const
{
  bind();
  glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, magFilter);
  glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, minFilter);
}

void Z3DTexture::setWrap(GLint wrap) const
{
  bind();
  glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, wrap);
  glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, wrap);
  glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_R, wrap);
}

void Z3DTexture::generateMipmap() const
{
  bind();
  glGenerateMipmap(m_textureTarget);
}

void Z3DTexture::uploadImage(const GLvoid* data) const
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  bind();

  switch (m_type) {
    case 3:
      glTexImage3D(m_textureTarget, 0, m_internalFormat,
                   m_dimension.x, m_dimension.y, m_dimension.z, 0,
                   m_dataFormat, m_dataType, data);
      break;
    case 2:
      glTexImage2D(m_textureTarget, 0, m_internalFormat,
                   m_dimension.x, m_dimension.y, 0,
                   m_dataFormat, m_dataType, data);
      break;
    case 1:
      glTexImage1D(m_textureTarget, 0, m_internalFormat,
                   m_dimension.x, 0,
                   m_dataFormat, m_dataType, data);
      break;
    default:
      break;
  }
}

void Z3DTexture::uploadSubImage(const glm::uvec3& offset, const glm::uvec3& size, const GLvoid* data) const
{
  CHECK(data);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  bind();

  switch (m_type) {
    case 3:
      glTexSubImage3D(m_textureTarget, 0, offset.x, offset.y, offset.z, size.x, size.y, size.z,
                      m_dataFormat, m_dataType, data);
      break;
    case 2:
      glTexSubImage2D(m_textureTarget, 0, offset.x, offset.y, size.x, size.y,
                      m_dataFormat, m_dataType, data);
      break;
    case 1:
      glTexSubImage1D(m_textureTarget, 0, offset.x, size.x,
                      m_dataFormat, m_dataType, data);
      break;
    default:
      break;
  }
}

size_t Z3DTexture::bypePerPixel(GLenum dataFormat, GLenum dataType)
{
  int numChannels = 0;
  switch (dataFormat) {
    case GL_COLOR_INDEX:
    case GL_RED:
    case GL_RED_INTEGER:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_INTENSITY:
    case GL_LUMINANCE:
    case GL_DEPTH_COMPONENT:
    case GL_STENCIL_INDEX:
    case GL_ALPHA_INTEGER_EXT:
      numChannels = 1;
      break;

    case GL_LUMINANCE_ALPHA:
    case GL_RG:
    case GL_RG_INTEGER:
    case GL_DEPTH_STENCIL:
      numChannels = 2;
      break;

    case GL_RGB:
    case GL_BGR:
    case GL_RGB_INTEGER:
    case GL_BGR_INTEGER:
      numChannels = 3;
      break;

    case GL_RGBA:
    case GL_BGRA:
    case GL_RGBA_INTEGER:
    case GL_BGRA_INTEGER:
    case GL_RGBA16:
    case GL_RGBA16F_ARB:
      numChannels = 4;
      break;

    default:
      LOG(WARNING) << "unknown data format";
  }

  double typeSize = 0;
  switch (dataType) {
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
      typeSize = 1.0 / 3.0;
      break;
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
      typeSize = 2.0 / 3.0;
      break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
      typeSize = 0.5;
      break;

    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
      typeSize = 1.0;
      break;

    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
      typeSize = 2.0;
      break;

    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
      typeSize = 4.0;
      break;

    default:
      LOG(WARNING) << "unknown data type";
  }

  return std::round(typeSize * numChannels);
}

size_t Z3DTexture::bypePerPixel(GLint internalFormat)
{
  size_t bpp = 0;
  switch (GLenum(internalFormat)) {
    case GL_COLOR_INDEX:
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_R8:
    case GL_R8_SNORM:
    case GL_INTENSITY:
    case GL_LUMINANCE:
    case GL_DEPTH_COMPONENT:
    case GL_R3_G3_B2:
    case GL_RGBA2:
    case GL_R8I:
    case GL_R8UI:
      bpp = 1;
      break;

    case GL_LUMINANCE_ALPHA:
    case GL_INTENSITY16:
    case GL_R16:
    case GL_R16F:
    case GL_R16_SNORM:
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_DEPTH_COMPONENT16:
    case GL_RGBA4:
    case GL_R16I:
    case GL_R16UI:
    case GL_RG8I:
    case GL_RG8UI:
      bpp = 2;
      break;

    case GL_RGB:
    case GL_BGR:
    case GL_RGB8:
    case GL_RGB8I:
    case GL_RGB8UI:
    case GL_SRGB8:
    case GL_RGB8_SNORM:
    case GL_DEPTH_COMPONENT24:
      bpp = 3;
      break;

    case GL_RGBA:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_BGRA:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
    case GL_R32F:
    case GL_RG16:
    case GL_RG16F:
    case GL_RG16_SNORM:
    case GL_SRGB8_ALPHA8:
    case GL_R32I:
    case GL_R32UI:
    case GL_RG16I:
    case GL_RG16UI:
    case GL_RGBA8I:
    case GL_RGBA8UI:
      bpp = 4;
      break;

    case GL_RGB16:
    case GL_RGB16I:
    case GL_RGB16UI:
    case GL_RGB16F:
    case GL_RGB16_SNORM:
      bpp = 6;
      break;

    case GL_RGBA16:
    case GL_RGBA16F:
    case GL_RGBA16I:
    case GL_RGBA16UI:
    case GL_RG32F:
    case GL_RG32I:
    case GL_RG32UI:
      bpp = 8;
      break;

    case GL_RGB32F:
    case GL_RGB32I:
    case GL_RGB32UI:
      bpp = 12;
      break;

    case GL_RGBA32I:
    case GL_RGBA32UI:
    case GL_RGBA32F:
      bpp = 16;
      break;

    default:
      LOG(WARNING) << "unknown internal format";
      break;
  }

  return bpp;
}

void Z3DTexture::downloadTextureToBuffer(GLenum dataFormat, GLenum dataType, GLvoid* buffer) const
{
  bind();
  glGetTexImage(m_textureTarget, 0, dataFormat, dataType, buffer);
}

void Z3DTexture::saveAsColorImage(const QString& filename) const
{
  try {
    GLenum dataFormat = GL_BGRA;
    GLenum dataType = GL_UNSIGNED_INT_8_8_8_8_REV;
    auto colorBuffer = std::make_unique<uint8_t[]>(bypePerPixel(dataFormat, dataType) * numPixels());
    downloadTextureToBuffer(dataFormat, dataType, colorBuffer.get());
    QImage upsideDownImage(colorBuffer.get(), width(), height(),
                           QImage::Format_ARGB32);
    QImage image = upsideDownImage.mirrored(false, true);
    QImageWriter writer(filename);
    writer.setCompression(1);
    if (!writer.write(image)) {
      LOG(ERROR) << writer.errorString();
    }
  }
  catch (ZException const& e) {
    LOG(ERROR) << "Exception: " << e.what();
  }
}

bool Z3DTexture::is1DTexture() const
{
  return m_textureTarget == GL_TEXTURE_1D ||
         m_textureTarget == GL_PROXY_TEXTURE_1D;
}

bool Z3DTexture::is2DTexture() const
{
  const GLenum all_2dtexture_targets[] = {
    GL_TEXTURE_2D,
    GL_TEXTURE_1D_ARRAY,
    GL_TEXTURE_RECTANGLE,
    GL_PROXY_TEXTURE_2D,
    GL_PROXY_TEXTURE_1D_ARRAY,
    GL_PROXY_TEXTURE_RECTANGLE,
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    GL_PROXY_TEXTURE_CUBE_MAP
  };

  return std::find(std::begin(all_2dtexture_targets), std::end(all_2dtexture_targets), m_textureTarget)
         != std::end(all_2dtexture_targets);
}

bool Z3DTexture::is3DTexture() const
{
  return m_textureTarget == GL_TEXTURE_3D ||
         m_textureTarget == GL_PROXY_TEXTURE_3D ||
         m_textureTarget == GL_TEXTURE_2D_ARRAY ||
         m_textureTarget == GL_PROXY_TEXTURE_2D_ARRAY ||
         m_textureTarget == GL_TEXTURE_CUBE_MAP_ARRAY ||
         m_textureTarget == GL_PROXY_TEXTURE_CUBE_MAP_ARRAY;
}

void Z3DTexture::getType()
{
  if (is3DTexture()) {
    m_type = 3;
  } else if (is2DTexture()) {
    m_type = 2;
  } else if (is1DTexture()) {
    m_type = 1;
  } else {
    CHECK(false);
  }
}

Z3DTextureUnitManager::Z3DTextureUnitManager()
  : m_maxTextureUnits(Z3DGpuInfo::instance().maxCombinedTextureImageUnits())
  , m_currentUnitNumber(-1)
{
}

void Z3DTextureUnitManager::nextAvailableUnit()
{
  ++m_currentUnitNumber;
  if (m_currentUnitNumber >= m_maxTextureUnits) {
    LOG(ERROR) << "No more avalable texture units!";
  }
}

void Z3DTextureUnitManager::activateCurrentUnit()
{
  if (m_currentUnitNumber < 0) {
    LOG(ERROR) << "Call nextAvailableUnit() to get a valid unit first!";
  } else if (m_currentUnitNumber >= m_maxTextureUnits) {
    LOG(ERROR) << "Exceed max number of texture units.";
  }
  glActiveTexture(currentUnitEnum());
}

GLenum Z3DTextureUnitManager::currentUnitEnum() const
{
  return GLenum(GLint(GL_TEXTURE0) + m_currentUnitNumber);
}

GLint Z3DTextureUnitManager::activeTextureUnit()
{
  GLint i;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &i);
  return i;
}

