#include "z3dtransferfunction.h"

#include <QLabel>

#include "z3dgpuinfo.h"
#include "z3dshaderprogram.h"
#include "z3dtransferfunctionwidgetwitheditorwindow.h"
#include "z3dvolume.h"
#include "z3dtexture.h"
#include "QsLog.h"

Z3DTransferFunction::Z3DTransferFunction(double min, double max, const glm::col4& minColor,
                                         const glm::col4& maxColor, uint32_t width, QObject* parent)
  : ZColorMap(min, max, minColor, maxColor, parent)
  , m_dimensions(width, 1, 1)
  , m_textureFormat(GL_BGRA)
  , m_textureDataType(GL_UNSIGNED_INT_8_8_8_8_REV)
{
}

Z3DTransferFunction::Z3DTransferFunction(const Z3DTransferFunction& tf)
  : ZColorMap(tf)
  , m_dimensions(tf.m_dimensions)
  , m_textureFormat(tf.m_textureFormat)
  , m_textureDataType(tf.m_textureDataType)
{
}

Z3DTransferFunction::Z3DTransferFunction(Z3DTransferFunction&& tf) noexcept
{
  swap(tf);
}

void Z3DTransferFunction::swap(Z3DTransferFunction& other) noexcept
{
  ZColorMap::swap(other);
  std::swap(m_dimensions, other.m_dimensions);
  std::swap(m_textureFormat, other.m_textureFormat);
  std::swap(m_textureDataType, other.m_textureDataType);
}

bool Z3DTransferFunction::operator==(const Z3DTransferFunction& tf) const
{
  if (!ZColorMap::equalTo(tf))
    return false;
  if (m_dimensions != tf.m_dimensions)
    return false;
  if (m_textureDataType != tf.m_textureDataType)
    return false;
  if (m_textureFormat != tf.m_textureFormat)
    return false;

  return true;
}

bool Z3DTransferFunction::operator!=(const Z3DTransferFunction& tf) const
{
  return !(*this == tf);
}

void Z3DTransferFunction::resetToDefault()
{
  reset(0., 1., glm::col4(0, 0, 0, 0), glm::col4(255, 255, 255, 255));
  emit changed();
}

Z3DTexture* Z3DTransferFunction::texture() const
{
  if (m_textureIsInvalid)
    updateTexture();

  return m_texture.get();
}

QString Z3DTransferFunction::samplerType() const
{
  if (m_dimensions.z > 1)
    return "sampler3D";
  if (m_dimensions.y > 1)
    return "sampler2D";

  return "sampler1D";
}

void Z3DTransferFunction::resize(uint32_t width)
{
  fitDimensions(width, m_dimensions.y, m_dimensions.z);

  if (width != m_dimensions.x) {
    m_dimensions.x = width;
    emit changed();
  }
}

void Z3DTransferFunction::fitDimensions(uint32_t& width, uint32_t& height, uint32_t& depth) const
{
  uint32_t maxTexSize;
  if (depth == 1) {
    maxTexSize = static_cast<uint32_t>(Z3DGpuInfo::instance().maxTextureSize());
  } else {
    maxTexSize = static_cast<uint32_t>(Z3DGpuInfo::instance().max3DTextureSize());
  }

  if (maxTexSize < width)
    width = maxTexSize;

  if (maxTexSize < height)
    height = maxTexSize;

  if (maxTexSize < depth)
    depth = maxTexSize;
}

void Z3DTransferFunction::updateTexture() const
{
  if (!m_texture || (m_texture->dimension() != glm::uvec3(m_dimensions)))
    createTexture();
  CHECK(m_texture);

  std::vector<glm::col4> tfData(m_dimensions.x);
  for (size_t x = 0; x < tfData.size(); ++x)
    tfData[x] = mappedColorBGRA(static_cast<double>(x) / (tfData.size() - 1));
  m_texture->uploadImage(tfData.data());

  m_textureIsInvalid = false;
}

void Z3DTransferFunction::createTexture() const
{
  m_texture.reset(new Z3DTexture(GLint(GL_RGBA8), glm::uvec3(m_dimensions), m_textureFormat, m_textureDataType));
}

bool Z3DTransferFunction::isValidDomainMin(double min) const
{
  return min < domainMax() && min >= 0.0 && min < 1.0;
}

bool Z3DTransferFunction::isValidDomainMax(double max) const
{
  return max > domainMin() && max > 0.0 && max <= 1.0;
}

Z3DTransferFunctionParameter::Z3DTransferFunctionParameter(const QString& name, QObject* parent)
  : ZSingleValueParameter<Z3DTransferFunction>(name, parent)
  , m_volume(nullptr)
{
  connect(&m_value, &Z3DTransferFunction::changed, this, &Z3DTransferFunctionParameter::valueChanged);
}

Z3DTransferFunctionParameter::Z3DTransferFunctionParameter(const QString& name, double min, double max,
                                                           const glm::col4& minColor,
                                                           const glm::col4& maxColor, int width, QObject* parent)
  : ZSingleValueParameter<Z3DTransferFunction>(name, parent)
  , m_volume(nullptr)
{
  m_value = Z3DTransferFunction(min, max, minColor, maxColor, width);
  connect(&m_value, &Z3DTransferFunction::changed, this, &Z3DTransferFunctionParameter::valueChanged);
}

void Z3DTransferFunctionParameter::setVolume(Z3DVolume* volume)
{
  if (m_volume != volume) {
    m_volume = volume;
    if (m_volume) {
      // Resize texture of tf according to bitdepth of volume
      int bits = m_volume->bitsStored();
      if (bits > 16)
        bits = 16; // handle float data as if it was 16 bit to prevent overflow

      int max = 1 << bits;
      m_value.resize(max);
    }
    emit valueChanged();
  }
}

QWidget* Z3DTransferFunctionParameter::actualCreateWidget(QWidget* parent)
{
  return new Z3DTransferFunctionWidgetWithEditorWindow(this, parent);
}

void Z3DTransferFunctionParameter::setSameAs(const ZParameter& rhs)
{
  CHECK(this->isSameType(rhs));
  const Z3DTransferFunctionParameter* src = static_cast<const Z3DTransferFunctionParameter*>(&rhs);
  if (m_value != src->get()) {
    m_value = src->get();
    m_value.invalidateTexture();
    emit valueChanged();
  }
  ZParameter::setSameAs(rhs);
}
