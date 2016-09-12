#ifndef _ZIMAGE_H_
#define _ZIMAGE_H_

#include <QImage>
#include <set>

#include "tz_image_lib_defs.h"
#include "tz_object_3d.h"
#include "zglmutils.h"
#include "zintpoint.h"
#include "zsttransform.h"
#include "neutube.h"

class ZStack;
class ZObject3dScan;
class ZJsonObject;

/** A class to load image / stack data structure from neurolabi and to display
 *  the data. The default format is Format_ARGB32_Premultiplied.
 */
class ZImage : public QImage {
public:
  template<typename T>
  struct DataSource {
    DataSource(const T *data, double scale, double offset, glm::vec3 color)
      : data(data), scale(scale), offset(offset), color(color)
    {}
    const T *data;
    float scale;
    float offset;
    glm::vec3 color;
  };

  ZImage();
  ZImage(int width, int height,
         QImage::Format format = QImage::Format_ARGB32_Premultiplied);

  ZImage(const ZImage &image);

  void clear();

  void init();

  int getZ() const {
    return m_z;
  }

  void setZ(int z) {
    m_z = z;
  }


  /*!
   * \brief Set data function
   *
   * The data is supposed to be aligned with the image. All set-data functions
   * do not use transform for legacy code compatibility.
   */
  void setData(const uint8 *data, int threshold = -1);

  void setData(
      const uint8 *data, int stackWidth, int stackHeight, int stackDepth,
      int slice, NeuTube::EAxis sliceAxis);

  void setData(
      const uint8 *data, int stackWidth, int stackHeight, int stackDepth,
      int slice, double scale, double offset, NeuTube::EAxis sliceAxis);

  void setData(const ZStack *stack, int z, bool ignoringZero = false,
               bool offsetAdjust = true);
//  void setData(const ZStack *stack, int z, NeuTube::EAxis sliceAxis,
//               bool ignoringZero = false, bool offsetAdjust = true);

  void setData(const color_t *data, int alpha = 255);
  void setCData(const color_t *data, double scale, double offset);
  void setCData(const uint16_t *data, uint8_t alpha);
  void setCData(const uint8_t *data, uint8_t alpha);
  void setData(const uint8 *data, double scale, double offset,
               int threshold = -1);
  void setDataIndexed8(const uint8 *data, double scale, double offset,
                       int threshold = -1);

  template<class T> void set2ChannelData(
      const T *data0, double scale0, double offset0,
      const T *data1, double scale1, double offset1,
      uint8_t alpha = 255);
  template<class T> void set3ChannelData(
      const T *data0, double scale0, double offset0,
      const T *data1, double scale1, double offset1,
      const T *data2, double scale2, double offset2,
      uint8_t alpha = 255);

  template<class T> void setBinaryData(const T *data, T bg = 0,
                                       int threshold = -1);
  template<class T>
  void setData(const T *data, double scale, double offset,
               int threshold = -1);

  void setData8(const DataSource<uint8_t> &source,
               int threshold = -1, bool useMultithread = true);

  void setData(const ZObject3dScan &obj);
  void setData(const ZObject3dScan &obj, const QColor &color);

  template<typename T>
  void setData(const DataSource<T> &source, int threshold = -1,
               bool useMultithread = true);

  template<typename T>
  void setData(const std::vector<DataSource<T> > &sources, uint8_t alpha = 255,
               bool useMultithread = true);

  void setDataIndexed8(const std::vector<DataSource<uint8_t> > &sources,
                       uint8_t alpha = 255, bool useMultithread = true);

  void setData(const std::vector<DataSource<uint8_t> > &sources,
               uint8_t alpha = 255, bool useMultithread = true);

  // set data to image region [startLine, endLine)
  // used by multi threaded version of setData
  template<typename T>
  void setDataBlock(const DataSource<T> &source, int startLine,
                    int endLine, int threshold = -1);

  void setDataBlock(const ZImage::DataSource<uint8_t> &source, int startLine,
                    int endLine, int threshold);

  void setDataBlockIndexed8(
      const ZImage::DataSource<uint8_t> &source, int startLine,
      int endLine, int threshold);

  template<typename T>
  void setDataBlockMS(const std::vector<DataSource<T> > &sources, int startLine,
                      int endLine, uint8_t alpha = 255);

  void setDataBlockMS8(const std::vector<DataSource<uint8_t> > &sources, int startLine,
                      int endLine, uint8_t alpha = 255);

  void setDataBlockMS8Indexed8(
      const std::vector<DataSource<uint8_t> > &sources, int startLine,
      int endLine, uint8_t alpha = 255);

  template<class T>
  void setData(const T *data, double scale, double offset,
               int lowerThreshold, int upperThreshold);

  void drawRaster(const void *data, int kind, double scale = 1.0,
                  double offset = 0.0, int threshold = -1);
  void drawLabelField(uint64_t *data, const QVector<QColor> &colorTable,
                      uint8_t alpha);
  void drawLabelField(uint64_t *data, const QVector<int> &colorTable,
                      int bgColor, int selColor);
  void drawLabelFieldTranspose(uint64_t *data, const QVector<int> &colorTable,
                               int bgColor, int selColor);
  void drawLabelField(uint64_t *data, const QVector<QColor> &colorTable,
                      uint8_t alpha, const std::set<uint64_t> &selected);

  void setBackground();

  ZImage *createMask();
  static ZImage* createMask(int width, int height);
  static ZImage* createMask(const QSize &size);
  static ZImage* createMask(const QRect &rect);

  void enhanceEdge();
  void enhanceContrast(bool highContrast);
  //void drawObject(Object_3d *obj, int z);

  static bool writeImage(const QImage &image, const QString &filePath);

  const ZStTransform& getTransform() const;

  /*!
   * \brief Set the transform
   *
   * \a transform transforms the world coordinates to image coordintes
   */
  void setTransform(const ZStTransform &transform);

  void setScale(double sx, double sy);
  void setOffset(double dx, double dy);

  void setHighContrastProtocal(
      double grayOffset, double grayScale, bool nonlinear);

  void loadHighContrastProtocal(const ZJsonObject &obj);
  void setDefaultContrastProtocal();
  void setContrastProtocol(double scale, double offset, bool nonlinear);

  void setVisible(bool visible);
  bool isVisible() const;

private:
  static bool hasSameColor(uchar *pt1, uchar *pt2);

private:
  ZStTransform m_transform; //Transformation from world coordinates to image coordinates
  ZStTransform m_projTransform; //Transform from image coordinates to screen coordinates

  //high constrast protocal
  bool m_nonlinear;
  double m_grayScale;
  double m_grayOffset;
  //ZIntPoint m_offset;

  bool m_visible;

  int m_z;
};

#include "zimage_tmpl.cpp"

#endif
