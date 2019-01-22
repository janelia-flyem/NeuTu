#ifndef ZCOLORMAP_H
#define ZCOLORMAP_H

#include "z3dtexture.h"
#include "zglmutils.h"
#include "zutils.h"
#include "widgets/zparameter.h"
#include <QColor>
#include <limits>
#include <vector>

class ZColorMapKey
{
public:
  ZColorMapKey(double i, const glm::col4& color);

  ZColorMapKey(double i, const glm::col4& colorL, const glm::col4& colorR);

  ZColorMapKey(double i, const glm::vec4& color);

  ZColorMapKey(double i, const glm::vec4& colorL, const glm::vec4& colorR);

  ZColorMapKey(double i, const QColor& color);

  ZColorMapKey(double i, const QColor& colorL, const QColor& colorR);

  bool operator==(const ZColorMapKey& key) const;

  bool operator!=(const ZColorMapKey& key) const;

  bool operator<(const ZColorMapKey& key) const;

  void setColorL(const glm::col4& color);

  void setColorL(const glm::ivec4& color);

  void setColorL(const glm::vec4& color);

  void setColorL(const QColor& color);

  inline glm::col4 colorL() const
  { return m_colorL; }

  QColor qColorL() const;

  void setColorR(const glm::col4& color);

  void setColorR(const glm::ivec4& color);

  void setColorR(const glm::vec4& color);

  void setColorR(const QColor& color);

  inline glm::col4 colorR() const
  { return m_colorR; }

  QColor qColorR() const;

  inline bool isSplit() const
  { return m_split; }

  void setSplit(bool split, bool useLeft = true);

  void setFloatAlphaR(double a);

  void setFloatAlphaL(double a);

  void setAlphaR(uint8_t a);

  void setAlphaL(uint8_t a);

  double floatAlphaR() const;

  double floatAlphaL() const;

  uint8_t alphaR() const;

  uint8_t alphaL() const;

  inline double intensity() const
  { return m_intensity; }

  inline void setIntensity(double i)
  { m_intensity = i; }

  ZColorMapKey* clone() const;

private:
  friend class ZColorMapParameter;

  friend class Z3DTransferFunctionParameter;

  double m_intensity;
  glm::col4 m_colorL;
  glm::col4 m_colorR;
  bool m_split;
};

// class for colormap. A ColorMap class should always has at least two keys
class ZColorMap : public QObject
{
Q_OBJECT
public:
  template<class ForwardIterator>
  ZColorMap(ForwardIterator first, ForwardIterator last,
            const glm::col4& minColor = glm::col4(0, 0, 0, 255),
            const glm::col4& maxColor = glm::col4(255, 255, 255, 255), QObject* parent = nullptr);

  explicit ZColorMap(double min = 0.0, double max = 255.0, const glm::col4& minColor = glm::col4(0, 0, 0, 255),
                     const glm::col4& maxColor = glm::col4(255, 255, 255, 255), QObject* parent = nullptr);

  template<class ForwardIterator>
  ZColorMap(ForwardIterator first, ForwardIterator last,
            const glm::vec4& minColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
            const glm::vec4& maxColor = glm::vec4(1.f, 1.f, 1.f, 1.f), QObject* parent = nullptr);

  ZColorMap(double min, double max, const glm::vec4& minColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
            const glm::vec4& maxColor = glm::vec4(1.f, 1.f, 1.f, 1.f), QObject* parent = nullptr);

  template<class ForwardIterator>
  ZColorMap(ForwardIterator first, ForwardIterator last, const QColor& minColor = QColor(0, 0, 0, 255),
            const QColor& maxColor = QColor(255, 255, 255, 255), QObject* parent = nullptr);

  ZColorMap(double min, double max, const QColor& minColor, const QColor& maxColor = QColor(0, 0, 0, 255),
            QObject* parent = nullptr);

  ZColorMap(const ZColorMap& cm);

  ZColorMap(ZColorMap&& other) noexcept;

  void swap(ZColorMap& other) noexcept;

  ZColorMap& operator=(ZColorMap other) noexcept
  {
    swap(other);
    return *this;
  }

  void reset(double min = 0.0, double max = 255.0, const glm::col4& minColor = glm::col4(0, 0, 0, 255),
             const glm::col4& maxColor = glm::col4(255, 255, 255, 255));

  template<class ForwardIterator>
  void
  reset(ForwardIterator first, ForwardIterator last, const glm::col4& minColor = glm::col4(0, 0, 0, 255),
        const glm::col4& maxColor = glm::col4(255, 255, 255, 255));

  template<class ForwardIterator>
  void reset(ForwardIterator first, ForwardIterator last,
             const glm::vec4& minColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
             const glm::vec4& maxColor = glm::vec4(1.f, 1.f, 1.f, 1.f));

  void reset(double min, double max, const glm::vec4& minColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
             const glm::vec4& maxColor = glm::vec4(1.f, 1.f, 1.f, 1.f));

  template<class ForwardIterator>
  void reset(ForwardIterator first, ForwardIterator last, const QColor& minColor = QColor(0, 0, 0, 255),
             const QColor& maxColor = QColor(255, 255, 255, 255));

  void reset(double min, double max, const QColor& minColor = QColor(0, 0, 0, 255),
             const QColor& maxColor = QColor(255, 255, 255, 255));

  bool operator==(const ZColorMap& cm) const;

  bool operator!=(const ZColorMap& cm) const;

  double domainMin() const;

  double domainMax() const;

  glm::dvec2 domain() const
  { return glm::dvec2(domainMin(), domainMax()); }

  virtual bool isValidDomainMin(double min) const;

  virtual bool isValidDomainMax(double max) const;

  // set new domain range might fail, check the validality use isValidDomain*** function
  bool setDomainMin(double min, bool rescaleKeys = false);

  bool setDomainMax(double max, bool rescaleKeys = false);

  void setDomain(double min, double max, bool rescaleKeys = false);

  void setDomain(const glm::dvec2& domain, bool rescaleKeys = false);

  glm::col4 mappedColor(double i) const;

  glm::col4 mappedColorBGRA(double i) const;

  glm::vec4 mappedFColor(double i) const;

  QColor mappedQColor(double i) const;

  glm::col4 keyColorL(size_t index) const;

  glm::vec4 keyFColorL(size_t index) const;

  QColor keyQColorL(size_t index) const;

  glm::col4 keyColorR(size_t index) const;

  glm::vec4 keyFColorR(size_t index) const;

  QColor keyQColorR(size_t index) const;

  void setKeyColorL(size_t index, const glm::col4& color);

  void setKeyColorR(size_t index, const glm::col4& color);

  void setKeyColorL(size_t index, const glm::vec4& color);

  void setKeyColorR(size_t index, const glm::vec4& color);

  void setKeyColorL(size_t index, const QColor& color);

  void setKeyColorR(size_t index, const QColor& color);

  double keyFloatAlphaL(size_t index) const;

  uint8_t keyAlphaL(size_t index) const;

  double keyFloatAlphaR(size_t index) const;

  uint8_t keyAlphaR(size_t index) const;

  void setKeyAlphaL(size_t index, uint8_t a);

  void setKeyAlphaR(size_t index, uint8_t a);

  void setKeyFloatAlphaL(size_t index, double a);

  void setKeyFloatAlphaR(size_t index, double a);

  double keyIntensity(size_t index) const;

  // note: don't call this function in a loop because this function might
  // change the order of keys and invalidate iterator
  void setKeyIntensity(size_t index, double intensity);

  bool isKeySelected(size_t index) const;

  void setKeySelected(size_t index, bool v = true);

  void deselectAllKeys();

  std::vector<size_t> selectedKeyIndexes() const;

  bool isKeySplit(size_t index) const;

  void setKeySplit(size_t index, bool v = true, bool useLeft = true);

  // fraction range [0.0 1.0]
  glm::col4 fractionMappedColor(double fraction) const;

  glm::vec4 fractionMappedFColor(double fraction) const;

  QColor fractionMappedQColor(double fraction) const;

  inline size_t numKeys() const
  { return m_keys.size(); }

  inline ZColorMapKey& key(size_t index)
  { return m_keys[index].first; }

  inline bool hasDataRange() const
  { return m_hasDataRange; }

  inline double dataMin() const
  { return m_dataMin; }

  inline double dataMax() const
  { return m_dataMax; }

  inline void setDataRange(double min, double max)
  {
    m_hasDataRange = true;
    m_dataMin = min;
    m_dataMax = max;
  }

  inline void removeDataRange()
  { m_hasDataRange = false; }

  bool setKey(size_t index, const ZColorMapKey& key, bool select = false);   //might change domain
  inline bool setKeys(const std::vector<ZColorMapKey>& keys);  //might change domain
  ZColorMapKey& addKey(const ZColorMapKey& key, bool select = false);

  void addKeyAtIntensity(double intensity, bool select = false);

  void addKeyAtIntensity(double intensity, const glm::col4& color, bool select = false);

  void addKeyAtIntensity(double intensity, uint8_t alpha, bool select = false);

  void addKeyAtIntensity(double intensity, double alpha, bool select = false);

  void addKeyAtFraction(double fraction, const glm::col4& color, bool select = false);

  void addKeyAtFraction(double fraction, bool select = false);

  void addKeyAtFraction(double fraction, uint8_t alpha, bool select = false);

  void addKeyAtFraction(double fraction, double alpha, bool select = false);

  void updateKeys();

  bool removeDuplicatedKeys();

  bool removeSelectedKeys();

  void removeKey(const ZColorMapKey& key);

  void removeKey(size_t index);

  inline bool isEmpty() const
  { return m_keys.empty(); }

  Z3DTexture* texture1D() const;

  void create1DTexture(size_t width = 256) const;

signals:

  void changed();

protected:
  void invalidateTexture()
  { m_textureIsInvalid = true; }

  void update1DTexture() const;

  inline void clearKeys()
  { m_keys.clear(); }

  virtual bool equalTo(const ZColorMap& cm) const;

protected:
  mutable std::unique_ptr<Z3DTexture> m_texture;
  mutable bool m_textureIsInvalid = true;

  friend class ZColorMapParameter;

  friend class Z3DTransferFunctionParameter;

  std::vector<std::pair<ZColorMapKey, bool>> m_keys;
  using KeyIterType = std::vector<std::pair<ZColorMapKey, bool>>::iterator;
  bool m_hasDataRange;
  double m_dataMin;
  double m_dataMax;
};

template<class ForwardIterator>
ZColorMap::ZColorMap(const ForwardIterator first, const ForwardIterator last, const glm::col4& minColor,
                     const glm::col4& maxColor, QObject* parent)
  : QObject(parent), m_hasDataRange(true)
{
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
  connect(this, &ZColorMap::changed, this, &ZColorMap::invalidateTexture);
}

template<class ForwardIterator>
ZColorMap::ZColorMap(const ForwardIterator first, const ForwardIterator last, const glm::vec4& minColor,
                     const glm::vec4& maxColor, QObject* parent)
  : QObject(parent), m_hasDataRange(true)
{
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
  connect(this, &ZColorMap::changed, this, &ZColorMap::invalidateTexture);
}

template<class ForwardIterator>
ZColorMap::ZColorMap(const ForwardIterator first, const ForwardIterator last, const QColor& minColor,
                     const QColor& maxColor, QObject* parent)
  : QObject(parent), m_hasDataRange(true)
{
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
  connect(this, &ZColorMap::changed, this, &ZColorMap::invalidateTexture);
}

template<class ForwardIterator>
void ZColorMap::reset(const ForwardIterator first, const ForwardIterator last, const glm::col4& minColor,
                      const glm::col4& maxColor)
{
  m_hasDataRange = true;
  blockSignals(true);
  clearKeys();
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
  blockSignals(false);
  emit changed();
}

template<class ForwardIterator>
void ZColorMap::reset(const ForwardIterator first, const ForwardIterator last, const glm::vec4& minColor,
                      const glm::vec4& maxColor)
{
  m_hasDataRange = true;
  blockSignals(true);
  clearKeys();
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
  blockSignals(false);
  emit changed();
}

template<class ForwardIterator>
void ZColorMap::reset(const ForwardIterator first, const ForwardIterator last, const QColor& minColor,
                      const QColor& maxColor)
{
  m_hasDataRange = true;
  blockSignals(true);
  clearKeys();
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
  blockSignals(false);
  emit changed();
}


// ZColormapParameter class
class ZColorMapParameter : public ZSingleValueParameter<ZColorMap>
{
Q_OBJECT
public:
  explicit ZColorMapParameter(const QString& name, QObject* parent = nullptr);

  ZColorMapParameter(const QString& name, const ZColorMap& cm, QObject* parent = nullptr);

  template<class ForwardIterator>
  ZColorMapParameter(const QString& name, ForwardIterator first, ForwardIterator last,
                     const glm::col4& minColor = glm::col4(0, 0, 0, 255),
                     const glm::col4& maxColor = glm::col4(255, 255, 255, 255), QObject* parent = nullptr);

  ZColorMapParameter(const QString& name, double min, double max, const glm::col4& minColor = glm::col4(0, 0, 0, 255),
                     const glm::col4& maxColor = glm::col4(255, 255, 255, 255), QObject* parent = nullptr);

  template<class ForwardIterator>
  ZColorMapParameter(const QString& name, ForwardIterator first, ForwardIterator last,
                     const glm::vec4& minColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
                     const glm::vec4& maxColor = glm::vec4(1.f, 1.f, 1.f, 1.f), QObject* parent = nullptr);

  ZColorMapParameter(const QString& name, double min, double max,
                     const glm::vec4& minColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
                     const glm::vec4& maxColor = glm::vec4(1.f, 1.f, 1.f, 1.f), QObject* parent = nullptr);

  template<class ForwardIterator>
  ZColorMapParameter(const QString& name, ForwardIterator first, ForwardIterator last,
                     const QColor& minColor = QColor(0, 0, 0, 255),
                     const QColor& maxColor = QColor(255, 255, 255, 255), QObject* parent = nullptr);

  ZColorMapParameter(const QString& name, double min, double max, const QColor& minColor = QColor(0, 0, 0, 255),
                     const QColor& maxColor = QColor(255, 255, 255, 255), QObject* parent = nullptr);

  // ZParameter interface
public:
  virtual void setSameAs(const ZParameter& rhs) override;

protected:
  virtual QWidget* actualCreateWidget(QWidget* parent) override;
};


template<class ForwardIterator>
ZColorMapParameter::ZColorMapParameter(const QString& name, const ForwardIterator first, const ForwardIterator last,
                                       const glm::col4& minColor, const glm::col4& maxColor, QObject* parent)
  : ZSingleValueParameter<ZColorMap>(name, parent)
{
  m_value.reset(first, last, minColor, maxColor);
  connect(&m_value, &ZColorMap::changed, this, &ZColorMapParameter::valueChanged);
}

template<class ForwardIterator>
ZColorMapParameter::ZColorMapParameter(const QString& name, const ForwardIterator first, const ForwardIterator last,
                                       const glm::vec4& minColor, const glm::vec4& maxColor, QObject* parent)
  : ZSingleValueParameter<ZColorMap>(name, parent)
{
  m_value.reset(first, last, minColor, maxColor);
  connect(&m_value, &ZColorMap::changed, this, &ZColorMapParameter::valueChanged);
}

template<class ForwardIterator>
ZColorMapParameter::ZColorMapParameter(const QString& name, const ForwardIterator first, const ForwardIterator last,
                                       const QColor& minColor, const QColor& maxColor, QObject* parent)
  : ZSingleValueParameter<ZColorMap>(name, parent)
{
  m_value.reset(first, last, minColor, maxColor);
  connect(&m_value, &ZColorMap::changed, this, &ZColorMapParameter::valueChanged);
}



#endif // ZCOLORMAP_H
