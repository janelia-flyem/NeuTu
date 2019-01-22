#ifndef Z3DTRANSFERFUNCTION_H
#define Z3DTRANSFERFUNCTION_H

#include <QObject>
#include <vector>

#include "z3dgl.h"
#include "zcolormap.h"
#include "widgets/zparameter.h"

class Z3DVolume;

class Z3DTexture;

// only support 1d transfer function now
class Z3DTransferFunction : public ZColorMap
{
Q_OBJECT
public:
  explicit Z3DTransferFunction(double min = 0.0, double max = 1.0, const glm::col4& minColor = glm::col4(0, 0, 0, 0),
                               const glm::col4& maxColor = glm::col4(255, 255, 255, 255),
                               uint32_t width = 256,
                               QObject* parent = nullptr);


  Z3DTransferFunction(const Z3DTransferFunction& tf);

  Z3DTransferFunction(Z3DTransferFunction&& tf) noexcept;

  void swap(Z3DTransferFunction& other) noexcept;

  Z3DTransferFunction& operator=(Z3DTransferFunction other) noexcept
  {
    swap(other);
    return *this;
  }

  bool operator==(const Z3DTransferFunction& tf) const;

  bool operator!=(const Z3DTransferFunction& tf) const;

  void resetToDefault();

  QString samplerType() const;

  inline glm::uvec3 textureDimensions() const
  { return m_dimensions; }

  // Returns the texture of the transfer function.
  Z3DTexture* texture() const;

  void resize(uint32_t width);

  // domain should be in [0.0, 1.0] range
  virtual bool isValidDomainMin(double min) const override;

  virtual bool isValidDomainMax(double max) const override;

protected:
  void updateTexture() const;

  void createTexture() const;

private:
  // Adapts the given width and height of transfer function to graphics board capabilities.
  void fitDimensions(uint32_t& width, uint32_t& height, uint32_t& depth) const;

protected:
  glm::uvec3 m_dimensions;
  GLenum m_textureFormat;
  GLenum m_textureDataType;
};

/*!
 * \brief The class of transfer function
 *
 * A transfer function maps a pixel value into a RGBA color. It is mainly used 
 * to adjust colors in volume rendering.
 */
class Z3DTransferFunctionParameter : public ZSingleValueParameter<Z3DTransferFunction>
{
Q_OBJECT
public:
  explicit Z3DTransferFunctionParameter(const QString& name, QObject* parent = nullptr);

  Z3DTransferFunctionParameter(const QString& name, double min, double max, const glm::col4& minColor,
                               const glm::col4& maxColor, int width, QObject* parent = nullptr);

  void setVolume(Z3DVolume* volume);

  inline Z3DVolume* volume() const
  { return m_volume; }

  // ZParameter interface
public:
  virtual void setSameAs(const ZParameter& rhs) override;

protected:
  virtual QWidget* actualCreateWidget(QWidget* parent) override;

protected:
  Z3DVolume* m_volume;
};

#endif // Z3DTRANSFERFUNCTION_H
