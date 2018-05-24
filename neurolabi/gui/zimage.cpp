#include <iostream>
#include <QImageWriter>

#include "tz_error.h"
#include "zimage.h"
#include "tz_stack_neighborhood.h"
#include "zstack.hxx"
#include "zfiletype.h"
#include "zobject3dscan.h"
#include "zobject3dstripe.h"
#include "neutube_def.h"
#include "tz_math.h"
#include "zjsonobject.h"

ZImage::ZImage() : QImage()
{
  init();
}

ZImage::ZImage(int width, int height, QImage::Format format) :
  QImage(width, height, format)
{
  init();
}

ZImage::ZImage(const ZImage &image) : QImage(image)
{
  m_transform = image.m_transform;
  m_usingContrastProtocal = image.m_usingContrastProtocal;
  m_contrastProtocol = image.m_contrastProtocol;
//  m_nonlinear = image.m_nonlinear;
//  m_grayScale = image.m_grayScale;
//  m_grayOffset = image.m_grayOffset;
  m_z = image.m_z;
}

void ZImage::init()
{
  if (width() > 0 && height() > 0) {
    memset(scanLine(0), 255, bytesPerLine() * height());
  }

  if (format() == Format_Indexed8) {
    for (int i = 0; i <= 255; ++i) {
      setColor(i, qRgb(i, i, i));
    }
  }

  m_usingContrastProtocal = false;
  setDefaultContrastProtocal();

  m_visible = true;
  m_z = neutube::DIM_INVALID_INDEX;
}

void ZImage::setDefaultContrastProtocal()
{
  m_contrastProtocol.setDefaultNonLinear();
//  m_nonlinear = true;
//  m_grayOffset = 0.0;
//  m_grayScale = 1.5;
}

/*
void ZImage::setContrastProtocol(double scale, double offset, bool nonlinear)
{
  m_contrastProtocol.setOffset(offset);
  m_contrastProtocol.setScale(scale);
  m_contrastProtocol.setNonlinear(nonlinear);
//  m_grayOffset = offset;
//  m_grayScale = scale;
//  m_nonlinear = nonlinear;
}
*/

void ZImage::setVisible(bool visible)
{
  m_visible = visible;
}

bool ZImage::isVisible() const
{
  return m_visible;
}

bool ZImage::isIndexed8() const
{
#ifdef _QT5_
  return format() == Format_Indexed8 || format() == Format_Grayscale8;
#else
  return format() == Format_Indexed8;
#endif
}

void ZImage::clear()
{
  uchar *line = scanLine(0);
  bzero(line, this->byteCount());
//  for (int i = 0; i < this->byteCount()
}
#if 0
void ZImage::adjustColorTable(int threshold)
{
  if (threshold >= 0) {
    for (int i = 0; i <= threshold; ++i) {
      setColor(i, qRgb(i, i, i));
    }
    for (int i = threshold + 1; i <= 255; ++i) {
      setColor(i, qRgb(255, 0, 0));
    }
  } else {
    for (int i = 0; i <= 255; ++i) {
      setColor(i, qRgb(i, i, i));
    }
  }
}
void ZImage::adjustColorTable(
    double scale, double offset, int threshold)
{
  adjustColorTable(scale, offset);
  if (threshold >= 0) {
    for (int i = 0; i <= threshold; ++i) {
      setColor(i, qRgb(i, i, i));
    }
    for (int i = threshold + 1; i <= 255; ++i) {
      setColor(i, qRgb(255, 0, 0));
    }
  }
}
#endif

void ZImage::adjustColorTable(double scale, double offset, int threshold)
{
  if (scale !=  1.0 || offset != 0.0 || m_usingContrastProtocal) {
    for (int i = 0; i <= 255; ++i) {
      int iv = m_contrastProtocol.mapGrey(i);
#if 0
      double v = i * scale + offset;
      if (m_usingContrastProtocal) {
        if (m_grayOffset != 0.0 || m_grayScale != 1.0) {
          double s = m_grayScale / 255.0;
          v = (v + m_grayOffset) * s;
          if (m_nonlinear) {
            v = sqrt(v) * i / 255.0;
          }
          v *= 255.0;
        }
      }

      int iv = iround(v);
      if (iv < 0) {
        iv = 0;
      } else if (iv > 255) {
        iv = 255;
      }
#endif
      setColor(i, qRgb(iv, iv, iv));
    }
  } else {
    for (int i = 0; i <= 255; ++i) {
      setColor(i, qRgb(i, i, i));
    }
  }

  if (threshold >= 0) {
    for (int i = threshold + 1; i <= 255; ++i) {
      setColor(i, qRgb(255, 0, 0));
    }
  }
}

void ZImage::setDataIndexed8(const uint8_t *data, int threshold)
{
  setDataIndexed8(data);
  adjustColorTable(1.0, 0.0, threshold);
}

void ZImage::setDataIndexed8(const uint8_t *data)
{
  int w = width();
  int h = height();

  for (int j = 0; j < h; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < w; i++) {
      *line++ = *data++;
    }
  }
}

void ZImage::setDataRgb32(const uint8_t *data, const uint8 *valueMap)
{
  int imageWidth = width();
  int imageHeight = height();

  for (int j = 0; j < imageHeight; j++) {
    uchar *line = scanLine(j);

    for (int i = 0; i < imageWidth; i++) {
      uint8 v = valueMap[*data++];
      *line++ = v;
      *line++ = v;
      *line++ = v;
      line++;
    }
  }
}

void ZImage::setDataRgba(const uint8_t *data, const uint8 *valueMap, int threshold)
{
  int imageWidth = width();
  int imageHeight = height();

  for (int j = 0; j < imageHeight; j++) {
    uchar *line = scanLine(j);

    for (int i = 0; i < imageWidth; i++) {
      uint8 v = *data++;
      if (v > threshold) {
        *line++ = '\0';
        *line++ = '\0';
        *line++ = '\xff';
      } else {
        v = valueMap[v];
        *line++ = v;
        *line++ = v;
        *line++ = v;
      }
      *line++ = '\xff';
    }
  }
}

void ZImage::setDataRgba(const uint8_t *data, const uint8 *valueMap)
{
  int imageWidth = width();
  int imageHeight = height();

  for (int j = 0; j < imageHeight; j++) {
    uchar *line = scanLine(j);

    for (int i = 0; i < imageWidth; i++) {
      uint8 v = valueMap[*data++];
      *line++ = v;
      *line++ = v;
      *line++ = v;
      *line++ = 255;
    }
  }
}

void ZImage::setDataRgba(const uint8_t *data)
{
  int w = width();
  int h = height();

  for (int j = 0; j < h; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < w; i++) {
      memset(line, *data++, 3);
      line += 3;
      *line++ = '\xff';
    }
  }
}

void ZImage::setDataRgba(const uint8_t *data, int threshold)
{
  int w = width();
  int h = height();

  for (int j = 0; j < h; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < w; i++) {
      if (*data > threshold) {
        *line++ = '\0';
        *line++ = '\0';
        *line++ = '\xff';
        ++data;
      } else {
        *line++ = *data;
        *line++ = *data;
        *line++ = *data++;
      }
      *line++ = '\xff';
    }
  }
}

void ZImage::setDataRgb32(const uint8_t *data)
{
  int w = width();
  int h = height();

  for (int j = 0; j < h; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < w; i++) {
      memset(line, *data++, 3);
      line += 4;
    }
  }
}

void ZImage::setDataRgb32(const uint8_t *data, int threshold)
{
  int w = width();
  int h = height();

  for (int j = 0; j < h; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < w; i++) {
      if (*data > threshold) {
        *line++ = '\0';
        *line++ = '\0';
        *line++ = '\xff';
        ++data;
      } else {
        *line++ = *data;
        *line++ = *data;
        *line++ = *data++;
      }
      line++;
    }
  }
}

bool ZImage::isArgb32() const
{
  return format() == QImage::Format_ARGB32 ||
      format() == QImage::Format_ARGB32_Premultiplied;
}

void ZImage::setData(const uint8 *data, int threshold)
{
  if (format() == QImage::Format_Indexed8) {
    setDataIndexed8(data);
    adjustColorTable(1.0, 0.0, threshold);
  } else if (isArgb32()) {
    if (threshold < 0) {
      setDataRgba(data);
    } else {
      setDataRgba(data, threshold);
    }
  } else if (format() == QImage::Format_RGB32) {
    setDataRgb32(data, threshold);
  }
}

void ZImage::setData(
    const uint8 *data, int stackWidth, int stackHeight, int /*stackDepth*/,
    int slice, neutube::EAxis sliceAxis)
{
  int imageWidth = width();
  int imageHeight = height();
  int area = stackWidth * stackHeight;

  switch (sliceAxis) {
  case neutube::Z_AXIS:
  case neutube::A_AXIS:
  {
    data += (size_t) area * slice;
    if (isIndexed8()) {
      setDataIndexed8(data);
    } else if (isArgb32()) {
      setDataRgba(data);
    } else if (format() == Format_RGB32) {
      setDataRgb32(data);
    }
  }
    break;
  case neutube::Y_AXIS:
  {
    const uint8 *dataOrigin = data + slice * stackWidth;

    if (format() == Format_Indexed8) {
      for (int j = 0; j < imageHeight; j++) {
        uchar *line = scanLine(j);
        data = dataOrigin + j * area;
        for (int i = 0; i < imageWidth; i++) {
          *line++ = *data++;
        }
      }
    } else if (depth() == 32) {
      for (int j = 0; j < imageHeight; j++) {
        uchar *line = scanLine(j);
        data = dataOrigin + j * area;
        for (int i = 0; i < imageWidth; i++) {
          *line++ = *data;
          *line++ = *data;
          *line++ = *data;
          data++;
          *line++ = 255;
        }
      }
    }
  }
    break;
  case neutube::X_AXIS:
  {
    const uint8 *dataOrigin = data + slice;

//    data += slice;
    if (format() == Format_Indexed8) {
      for (int j = 0; j < imageHeight; j++) {
        uchar *line = scanLine(j);
        data = dataOrigin + j * stackWidth;
        for (int i = 0; i < imageWidth; i++) {
          *line++ = *data;
          data += area;
        }
      }
    } else if (depth() == 32) {
      for (int j = 0; j < imageHeight; j++) {
        uchar *line = scanLine(j);
        data = dataOrigin + j * stackWidth;
        for (int i = 0; i < imageWidth; i++) {
          *line++ = *data;
          *line++ = *data;
          *line++ = *data;
          data += area;
          *line++ = 255;
        }
      }
    }
  }
    break;
  }
}

void ZImage::MakeValueMap(double scale, double offset, uint8 *valueMap)
{
  for (int i = 0; i < 256; ++i) {
    int value = iround(scale * i + offset);
    if (value <= 0) {
      valueMap[i] = '\0';
    } else if (value >= 255) {
      valueMap[i] = '\xff';
    } else {
      valueMap[i] = value;
    }
  }
}

void ZImage::setData(
    const uint8 *data, int stackWidth, int stackHeight, int /*stackDepth*/,
    int slice, double scale, double offset, neutube::EAxis sliceAxis)
{
  int imageWidth = width();
  int imageHeight = height();
  int area = stackWidth * stackHeight;

  uint8 valueMap[256];
  if (format() != Format_Indexed8) {
    MakeValueMap(scale, offset, valueMap);
  } else {
    adjustColorTable(scale, offset, -1);
  }

  switch (sliceAxis) {
  case neutube::Z_AXIS:
  case neutube::A_AXIS:
  {
    data += (size_t) area * slice;
    if (format() == Format_Indexed8) {
      setDataIndexed8(data);
    } else if (isArgb32()) {
      setDataRgba(data, valueMap);
    }
  }
    break;
  case neutube::Y_AXIS:
  {
    const uint8 *dataOrigin = data + slice * stackWidth;

    if (format() == Format_Indexed8) {
      for (int j = 0; j < imageHeight; j++) {
        uchar *line = scanLine(j);
        data = dataOrigin + j * area;
        for (int i = 0; i < imageWidth; i++) {
          *line++ = *data++;
        }
      }
    } else if (depth() == 32) {
      for (int j = 0; j < imageHeight; j++) {
        uchar *line = scanLine(j);
        data = dataOrigin + j * area;
        for (int i = 0; i < imageWidth; i++) {
          uint8 v = valueMap[*data++];
          *line++ = v;
          *line++ = v;
          *line++ = v;
          //        data++;
          *line++ = 255;
        }
      }
    }
  }
    break;
  case neutube::X_AXIS:
  {
    const uint8 *dataOrigin = data + slice;
//    data += slice;
    if (format() == Format_Indexed8) {
      for (int j = 0; j < imageHeight; j++) {
        uchar *line = scanLine(j);
        data = dataOrigin + j * stackWidth;
        for (int i = 0; i < imageWidth; i++) {
          *line++ = *data;
          data += area;
        }
      }
    } else if (depth() == 32) {
      for (int j = 0; j < imageHeight; j++) {
        uchar *line = scanLine(j);
        data = dataOrigin + j * stackWidth;
        for (int i = 0; i < imageWidth; i++) {
          uint8 v = valueMap[*data];
          *line++ = v;
          *line++ = v;
          *line++ = v;
          data += area;
          *line++ = 255;
        }
      }
    }
  }
    break;
  }
}

void ZImage::setData(const color_t *data, int alpha)
{
  if (isArgb32()) {
    int i, j;
    for (j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        *line++ = (*data)[2];
        *line++ = (*data)[1];
        *line++ = (*data)[0];
        data++;
        //*line++ = '\xff';
        *line++ = alpha;
      }
    }
  }
}

void ZImage::setData(const ZObject3dScan &obj)
{
  setData(obj, obj.getColor());
}

void ZImage::setData(const ZObject3dScan &obj, const QColor &color)
{
  uint32_t colorValue = color.alpha();
  colorValue <<= 8;
  colorValue += color.red();
  colorValue <<= 8;
  colorValue += color.green();
  colorValue <<= 8;
  colorValue += color.blue();

  int stripeNumber = obj.getStripeNumber();
  if (isArgb32()) {
    for (int i = 0; i < stripeNumber; ++i) {
      const ZObject3dStripe &stripe = obj.getStripe(i);
      //    const ZObject3dScan::Segment& seg= iter.next();
      int y = m_transform.transformY(stripe.getY());
      int nseg = stripe.getSegmentNumber();
      for (int j = 0; j < nseg; ++j) {
        int x0 = m_transform.transformX(stripe.getSegmentStart(j));
        int x1 = m_transform.transformX(stripe.getSegmentEnd(j));
        if (x0 < 0) {
          x0 = 0;
        }
        if (x1 >= width()) {
          x1 = width() - 1;
        }
        if (y < height() && y >= 0) {
          uint32_t *line = ((uint32_t*) scanLine(y)) + x0;
          for (int x = x0; x <= x1; ++x) {
            *line++ = colorValue;
          }
        }
      }
    }
  }
}

void ZImage::setDataRgba(const color_t *data)
{
  int h = height();
  int w = width();
  for (int j = 0; j < h; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < w; i++) {
      *line++ = (*data)[2];
      *line++ = (*data)[1];
      *line++ = (*data)[0];
      data++;
      *line++ = '\xff';
    }
  }
}

void ZImage::setDataRgb32(const color_t *data)
{
  int h = height();
  int w = width();
  for (int j = 0; j < h; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < w; i++) {
      *line++ = (*data)[2];
      *line++ = (*data)[1];
      *line++ = (*data)[0];
      data++;
      line++;
    }
  }
}

void ZImage::setDataRgba(const color_t *data, double scale, double offset)
{
  int h = height();
  int w = width();
  for (int j = 0; j < h; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < w; i++) {
      double value = scale * (*data)[2] + offset;
      uint8 v;
      if (value <= 0.0) {
        v = '\0';
      } else if (value >= 255.0) {
        v = '\xff';
      } else {
        v = (uint8) value;
      }
      *line++ = v;
      value = scale * (*data)[1] + offset;
      if (value <= 0.0) {
        v = '\0';
      } else if (value >= 255.0) {
        v = '\xff';
      } else {
        v = (uint8) value;
      }
      *line++ = v;
      value = scale * (*data)[0] + offset;
      if (value <= 0.0) {
        v = '\0';
      } else if (value >= 255.0) {
        v = '\xff';
      } else {
        v = (uint8) value;
      }
      *line++ = v;
      data++;
      *line++ = '\xff';
    }
  }
}

void ZImage::setDataRgb32(const color_t *data, double scale, double offset)
{
  int h = height();
  int w = width();
  for (int j = 0; j < h; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < w; i++) {
      double value = scale * (*data)[2] + offset;
      uint8 v;
      if (value <= 0.0) {
        v = '\0';
      } else if (value >= 255.0) {
        v = '\xff';
      } else {
        v = (uint8) value;
      }
      *line++ = v;
      value = scale * (*data)[1] + offset;
      if (value <= 0.0) {
        v = '\0';
      } else if (value >= 255.0) {
        v = '\xff';
      } else {
        v = (uint8) value;
      }
      *line++ = v;
      value = scale * (*data)[0] + offset;
      if (value <= 0.0) {
        v = '\0';
      } else if (value >= 255.0) {
        v = '\xff';
      } else {
        v = (uint8) value;
      }
      *line++ = v;
      data++;
      line++;
    }
  }
}

void ZImage::setCData(const color_t *data, double scale, double offset)
{
  if (isArgb32()) {
    if (scale == 1.0 && offset == 0.0) {
      setDataRgba(data);
    } else {
      setDataRgba(data, scale, offset);
    }
  } else if (format() == QImage::Format_RGB32) {
    if (scale == 1.0 && offset == 0.0) {
      setDataRgb32(data);
    } else {
      setDataRgb32(data, scale, offset);
    }
  }
}

void ZImage::setDataIndexed8(const uint8 *data, double scale, double offset,
                     int threshold)
{
  setDataIndexed8(data);
  adjustColorTable(scale, offset, threshold);
#if 0
#ifdef _DEBUG_2
  tic();
#endif
  if (scale == 1.0 && offset == 0.0) {
    setData(data, threshold);
  } else {
    uint8 valueMap[256];
    for (int i = 0; i < 256; ++i) {
      double value = scale * i + offset;
      if (value <= 0.0) {
        valueMap[i] = '\0';
      } else if (value >= 255.0) {
        valueMap[i] = '\xff';
      } else {
        valueMap[i] = (uint8) value;
      }
    }

    int i, j;
    for (j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        //memset(line, valueMap[*data++], 3);
        uint8 v = valueMap[*data++];
        *line++ = v;
      }
    }

    for (int i = 0; i <= threshold; ++i) {
      setColor(i, qRgb(255, 0, 0));
    }
  }
#ifdef _DEBUG_2
  std::cout << toc() << std::endl;
#endif
#endif
}

void ZImage::setData(const uint8 *data, double scale, double offset,
                     int threshold)
{
#ifdef _DEBUG_2
  tic();
#endif
  if (format() == QImage::Format_Indexed8) {
    setDataIndexed8(data, scale, offset, threshold);
  } else if (depth() == 32) {
    if (scale == 1.0 && offset == 0.0) {
      if (threshold < 0) {
        setDataRgba(data);
      } else {
        setDataRgba(data, threshold);
      }
    } else {
      uint8 valueMap[256];
      MakeValueMap(scale, offset, valueMap);

      if (threshold < 0) {
        setDataRgba(data, valueMap);
      } else {
        setDataRgba(data, valueMap, threshold);
      }
    }
  }
#ifdef _DEBUG_2
  std::cout << toc() << std::endl;
#endif
}


void ZImage::setCData(const uint16_t *data, uint8_t alpha)
{
  int i, j;

  //uint8_t *data8 = (uint8_t*) data;

  for (j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (i = 0; i < width(); i++) {
      int v = *(data++) * 127;
      *line++ = v % 255;
      v /= 255;
      *line++ = v % 255;
      *line++ = v / 255;
      *line++ = alpha;
    }
  }
}

void ZImage::setCData(const uint8_t *data, uint8_t alpha)
{
  int i, j;

  for (j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (i = 0; i < width(); i++) {
      *line++ = *data;
      *line++ = *data;
      *line++ = *data++;
      *line++ = alpha;
    }
  }
}

void ZImage::drawLabelField(
    uint64_t *data, const QVector<int> &colorTable, int bgColor, int selColor)
{
  int colorCount = colorTable.size();
  if (colorCount > 0) {
    int h = height();
    int w = width();

    for (int j = 0; j < h; j++) {
      int *line = (int*) scanLine(j);
      for (int i = 0; i < w; i++) {
        uint64_t v = *data++;
        if (v == 0) {
          *line++ = bgColor;
        } else if (v == flyem::LABEL_ID_SELECTION) {
          *line++ = selColor;
        } else {
          *line++ = colorTable[v % colorCount] ;
        }
      }
    }
  }
}

void ZImage::drawLabelFieldTranspose(
    uint64_t *data, const QVector<int> &colorTable, int bgColor, int selColor)
{
  int colorCount = colorTable.size();
  if (colorCount > 0) {
    int h = height();
    int w = width();

    uint64_t *dataLine = data;
    for (int j = 0; j < h; j++) {
      int *line = (int*) scanLine(j);
      dataLine = data + j;
      for (int i = 0; i < w; i++) {
        uint64_t v = *dataLine;
        dataLine += h;
        if (v == 0) {
          *line++ = bgColor;
        } else if (v == flyem::LABEL_ID_SELECTION) {
          *line++ = selColor;
        } else {
          *line++ = colorTable[v % colorCount] ;
        }
      }
    }
  }
}

void ZImage::drawLabelField(
    uint64_t *data, const QVector<QColor> &colorTable, uint8_t alpha)
{
  int i, j;
  QVector<int> rgbaTable(colorTable.size());
  int newAlpha = alpha;
  newAlpha <<= 24;
  for (int i = 0; i < colorTable.size(); ++i) {
    const QColor &color = colorTable[i];
    rgbaTable[i] = newAlpha + (color.red() << 16) + (color.green() << 8) +
        (color.blue());
  }

  int colorCount = colorTable.size();

//  uint16_t *colorIndex = (uint16_t*) data;
  int h = height();
  int w = width();

  for (j = 0; j < h; j++) {
//    uchar *line = scanLine(j);
    int *line = (int*) scanLine(j);
    for (i = 0; i < w; i++) {
//      const QColor &color = colorTable[*data++ % colorCount];

//      *data++ % colorCount;
//      int color = rgbaTable[colorIndex[2]];
      *line++ = rgbaTable[*data++ % colorCount] ;
//      colorIndex += 4;

//      *line++ = color.red();
//      *line++ = color.green();
//      *line++ = color.blue();
//      *line++ = alpha;
    }
  }
}

void ZImage::drawLabelField(
    uint64_t *data, const QVector<QColor> &colorTable, uint8_t alpha,
    const std::set<uint64_t> &selected)
{
  int i, j;
  QVector<int> rgbaTable(colorTable.size());
  int newAlpha = alpha;
  newAlpha <<= 24;
  for (int i = 0; i < colorTable.size(); ++i) {
    const QColor &color = colorTable[i];
    rgbaTable[i] = newAlpha + (color.red() << 16) + (color.green() << 8) +
        (color.blue());
  }

  int colorCount = colorTable.size();

//  uint16_t *colorIndex = (uint16_t*) data;
  int h = height();
  int w = width();

  for (j = 0; j < h; j++) {
//    uchar *line = scanLine(j);
    int *line = (int*) scanLine(j);
    for (i = 0; i < w; i++) {
      uint64_t label = *data++;
      if (selected.count(label) > 0) {
        *line++ = 0;
      } else {
        *line++ = rgbaTable[label % colorCount] ;
      }
    }
  }
}

void ZImage::drawRaster(const void *data, int kind, double scale,
			double offset, int threshold)
{
  Image_Array ima;
  ima.array = (uint8*) data;

  switch (kind) {
  case GREY:
    setData(ima.array8, threshold);
    break;
  case GREY16:
    setData(ima.array16, scale, offset, threshold);
    break;
  case FLOAT32:
    setData(ima.array32, scale, offset);
    break;
  case FLOAT64:
    setData(ima.array64, scale, offset);
    break;
  case COLOR:
    setData(ima.arrayc);
    break;
  default:
    PRINT_EXCEPTION("Unknown data type", "The kind of data is not recognized");
    throw std::exception();
  }
}

void ZImage::setBackground()
{
  //int w = this->bytesPerLine();
  //int h = height();
  //memset_pattern4(scanLine(0), "\xef\xef\xef\xff", w * h);
  //bzero(scanLine(0), w * h);
  //memset_pattern4(scanLine(0), "\xef\xef\xef\xff", w * h);
  //fill(Qt::transparent);
  fill(Qt::black);
}

ZImage* ZImage::createMask()
{
  ZImage *image = createMask(width(), height());
  if (image != NULL) {
    image->setTransform(image->getTransform());
  }

  return image;
}

ZImage* ZImage::createMask(const QSize &size)
{
  return createMask(size.width(), size.height());
}

ZImage* ZImage::createMask(int width, int height)
{
  ZImage *mask = NULL;
  if (width > 0 && height > 0) {
    mask = new ZImage(width, height,
                      QImage::Format_ARGB32_Premultiplied);
    mask->fill(0);
  }

  return mask;
}

ZImage* ZImage::createMask(const QRect &rect)
{
  ZImage *image = createMask(rect.width(), rect.height());
  if (image != NULL) {
    image->setOffset(-rect.left(), -rect.top());
  }

  return image;
}

bool ZImage::hasSameColor(uchar *pt1, uchar *pt2)
{
  return (pt1[0] == pt2[0]) && (pt1[1] == pt2[1]) && (pt1[2] == pt2[2]);
}

static void stack_neighbor_offset(int n_nbr, int width, int height,
                                  int neighbor[])
{
  switch (n_nbr) {
  case 4:
    neighbor[0] = -4;
    neighbor[1] = -neighbor[0];
    neighbor[2] = -width;
    neighbor[3] = -neighbor[2];
    break;
  case 8:
    stack_neighbor_offset(4, width, height, neighbor);
    neighbor[4] = -width - 4;
    neighbor[5] = -neighbor[4];
    neighbor[6] = -width + 4;
    neighbor[7] = -neighbor[6];
  default:
    break;
  }
}

void ZImage::enhanceEdge()
{
  int i, j;
  int neighborOffset[26];
  int isInBound[26];
  int nnbr = 8;
  stack_neighbor_offset(nnbr, bytesPerLine(), height(), neighborOffset);

  uchar *edge = new uchar[width() * height()];
  uchar *edgeIterator = edge;

  int cwidth = width() - 1;
  int cheight = height() - 1;

  for (j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (i = 0; i < width(); i++) {
      int n = Stack_Neighbor_Bound_Test_S(nnbr, cwidth, cheight, 1, i, j, 0,
                                          isInBound);
      *edgeIterator = 0;
      if (n != nnbr) {
        for (int neighborIndex = 0; neighborIndex < nnbr; ++neighborIndex) {
          if (isInBound[neighborIndex]) {
            if (!hasSameColor(line, line + neighborOffset[neighborIndex])) {
              *edgeIterator = 1;
              break;
            }
          }
        }
      } else {
        for (int neighborIndex = 0; neighborIndex < nnbr; ++neighborIndex) {
          if (!hasSameColor(line, line + neighborOffset[neighborIndex])) {
            *edgeIterator = 1;
            break;
          }
        }
      }
      line += 4;
      edgeIterator++;
    }
  }

  edgeIterator = edge;
  for (j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (i = 0; i < width(); i++) {
      if (*edgeIterator++) {
        *line++ = 255;
        *line++ = 0;
        *line++ = 0;
        *line++ = 127;
      } else {
        *line++ = 0;
        *line++ = 0;
        *line++ = 0;
        *line++ = 0;
      }
    }
  }

  delete []edge;
}

void ZImage::setData(const ZStack *stack, int z, bool ignoringZero,
                     bool offsetAdjust)
{
  if (stack != NULL) {
    if (stack->kind() == GREY) {
      if (z >= stack->getOffset().getZ() &&
          z < stack->getOffset().getZ() + stack->depth()) {
        int targetWidth = width();
        int targetHeight = height();
        int sourceWidth = stack->width();
        int sourceHeight = stack->height();


        int tx0 = offsetAdjust ? imax2(stack->getOffset().getX(), 0) : 0;
        int ty0 = offsetAdjust ? imax2(stack->getOffset().getY(), 0) : 0;
        int tx1 = offsetAdjust ? imin2(tx0 + sourceWidth, targetWidth) : 0;
        int ty1 = offsetAdjust ? imin2(ty0 + sourceHeight,targetHeight) : 0;
        int sx = tx0;
        int sy = ty0;

        uchar *line = NULL;

        if (format() == Format_Indexed8) {
          for (int y = ty0; y < ty1; ++y) {
            if (offsetAdjust) {
              line = scanLine(y) + tx0;
            } else {
              line = scanLine(y - ty0);
            }
            const uint8_t *data = stack->getDataPointer(sx, sy, z);
            for (int x = tx0; x < tx1; ++x) {
              if (!ignoringZero || *data > 0) {
                *line++ = *data;
              } else {
                line++;
              }
              ++data;
            }
            ++sy;
          }
        } else if (depth() == 32){
          for (int y = ty0; y < ty1; ++y) {
            if (offsetAdjust) {
              line = scanLine(y) + tx0 * 4;
            } else {
              line = scanLine(y - ty0);
            }
            const uint8_t *data = stack->getDataPointer(sx, sy, z);
            for (int x = tx0; x < tx1; ++x) {
              if (!ignoringZero || *data > 0) {
                *line++ = *data;
                *line++ = *data;
                *line++ = *data;
                *line++ = 255;
              } else {
                line += 4;
              }
              ++data;
            }
            ++sy;
          }
        }
      }
    }
  }
}
/*
void ZImage::setHighContrastProtocal(
    double grayOffset, double grayScale, bool nonlinear)
{
  m_contrastProtocol.setOffset(grayOffset);
  m_contrastProtocol.setScale(grayScale);
  m_contrastProtocol.setNonlinear(nonlinear);
//  m_nonlinear = nonlinear;
//  m_grayOffset = grayOffset;
//  m_grayScale = grayScale;
}
*/
void ZImage::setContrastProtocol(const ZContrastProtocol &cp)
{
  m_contrastProtocol = cp;
}

void ZImage::updateContrast(const ZContrastProtocol &cp)
{
  setContrastProtocol(cp);
  enhanceContrast(m_usingContrastProtocal);
}

void ZImage::updateContrast(const ZJsonObject &cpObj)
{
  ZContrastProtocol cp;
  cp.load(cpObj);
  updateContrast(cp);
}

void ZImage::updateContrast(bool usingContrast)
{
  m_usingContrastProtocal = usingContrast;
  enhanceContrast(m_usingContrastProtocal);
}

void ZImage::loadHighContrastProtocal(const ZJsonObject &obj)
{
  m_contrastProtocol.load(obj);
#if 0
  if (obj.hasKey("nonlinear")) {
    m_nonlinear = ZJsonParser::booleanValue(obj["nonlinear"]);
  }

  if (obj.hasKey("offset")) {
    m_grayOffset = ZJsonParser::numberValue(obj["offset"]);
  }

  if (obj.hasKey("scale")) {
    m_grayScale = ZJsonParser::numberValue(obj["scale"]);
  }
#endif
}

void ZImage::enhanceContrast(bool highContrast)
{
  if (format() != ZImage::Format_Indexed8) {

    if (this->depth() == 32) {
      if (highContrast) {
        for (int j = 0; j < height(); j++) {
          uchar *line = scanLine(j);
          for (int i = 0; i < width(); i++) {
            if (line[0] <= 213) {
              line[0] += line[0] / 5;
            } else {
              line[0] = 255;
            }
            if (line[1] <= 213) {
              line[1] += line[1] / 5;
            } else {
              line[1] = 255;
            }
            if (line[2] <= 213) {
              line[2] += line[2] / 5;
            } else {
              line[2] = 255;
            }

            line += 4;
          }
        }
      }
    } else if (this->depth() == 8) {
      if (highContrast) {
        uchar colorTable[256];
//        double s = m_grayScale;
        for (int i = 0; i < 256; ++i) {
          int v = m_contrastProtocol.mapGrey(i);
#if 0
          double v = (i + m_grayOffset) * s;

          if (m_nonlinear) {
            if (v < 0.0) {
              v = 0.0;
            } else {
              v = sqrt(v / 255.0) * i;
            }
          }

          if (v < 0.0) {
            v = 0.0;
          } else if (v > 255.0) {
            v = 255.0;
          }
#endif
          colorTable[i] = iround(v);
        }

        for (int j = 0; j < height(); j++) {
          uchar *line = scanLine(j);
          for (int i = 0; i < width(); i++) {
            line[0] = colorTable[line[0]];
            line++;
          }
        }
      }
    }
  } else {
    if (highContrast) {
//      double s = m_grayScale / 255.0;
      for (int i = 0; i < 256; ++i) {
        QColor color;
#if 0
        double v = (i + m_grayOffset) * s;
        if (m_nonlinear) {
          v = sqrt(v) * i / 255.0;
        }
//        v *= sqrt(v);
//        v = v * 1.5;

        if (v < 0.0) {
          v = 0.0;
        } else if (v > 1.0) {
          v = 1.0;
        }
#endif
        int v = m_contrastProtocol.mapGrey(i);

        color.setRed(v);
        color.setGreen(v);
        color.setBlue(v);
        setColor(i, color.rgb());

#ifdef _DEBUG_2
        std::cout << "Gray map: " << i << " -> " << v << std::endl;
#endif
      }
    } else {
      for (int i = 0; i < 255; ++i) {
        setColor(i, qRgb(i, i, i));
      }
    }
  }
}

void ZImage::setData8(const ZImage::DataSource<uint8_t> &source,
                     int threshold, bool useMultithread)
{
  if (source.color == glm::vec3(1.f,1.f,1.f)) {
    setData(source.data, source.scale, source.offset, threshold);
    return;
  }

  if (useMultithread) {
    int numBlock = std::min(height(), 16);
    int blockHeight = height() / numBlock;
    std::vector<QFuture<void> > res(numBlock);
    for (int i=0; i<numBlock; ++i) {
      int startLine = i * blockHeight;
      int endLine = (i+1) * blockHeight;
      if (i == numBlock - 1)
        endLine = height();
      res[i] = QtConcurrent::run(this, &ZImage::setDataBlock<uint8_t>, source, startLine,
                                 endLine, threshold);
    }
    for (int i=0; i<numBlock; ++i)
      res[i].waitForFinished();
    return;
  }

  const uint8_t* data = source.data;
  float scale = source.scale;
  float offset = source.offset;
  glm::vec3 color = glm::vec3(source.color.b, source.color.g, source.color.r);

  glm::col3 colorMap[256];
  for (int i = 0; i < 256; ++i) {
      if (scale == 1.f && offset == 0.f) {
          colorMap[i] = glm::col3(color * (float) i);
      } else {
          colorMap[i] = glm::col3(glm::clamp(color * (scale * i + offset),
                                             glm::vec3(0.f), glm::vec3(255.f)));
      }
  }

  if (format() == QImage::Format_Indexed8) {
    for (int i = 0; i < 256; ++i) {
      if (i <= threshold) {
        setColor(i, qRgb(255, 0, 0));
      } else {
        int r = iround(colorMap[i][0] * 255);
        int g = iround(colorMap[i][1] * 255);
        int b = iround(colorMap[i][2] * 255);

        setColor(i, qRgb(r, g, b));
      }
    }
    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        *line++ = *data++;
      }
    }
  } else {
    if (scale == 1.f && offset == 0.f) {
      if (threshold < 0) {
        for (int j = 0; j < height(); j++) {
          uchar *line = scanLine(j);
          for (int i = 0; i < width(); i++) {
            //glm::col3 col3 = glm::col3(color * (float)(*data++));
            glm::col3 col3 = colorMap[*data++];

            *line++ = col3[0];
            *line++ = col3[1];
            *line++ = col3[2];
            *line++ = '\xff';
          }
        }
      } else {
        for (int j = 0; j < height(); j++) {
          uchar *line = scanLine(j);
          for (int i = 0; i < width(); i++) {
            if (*data > threshold) {
              *line++ = '\0';
              *line++ = '\0';
              *line++ = '\xff';
              *line++ = '\xff';
              ++data;
            } else {
              //glm::col3 col3 = glm::col3(color * (float)(*data++));
              glm::col3 col3 = colorMap[*data++];

              *line++ = col3[0];
              *line++ = col3[1];
              *line++ = col3[2];
              *line++ = '\xff';
            }
          }
        }
      }
      return;
    }

    if (threshold < 0) {
      for (int j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (int i = 0; i < width(); i++) {
          /*
        glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                              glm::vec3(0.f), glm::vec3(255.f)));
                                              */
          glm::col3 col3 = colorMap[*data++];

          *line++ = col3[0];
          *line++ = col3[1];
          *line++ = col3[2];
          *line++ = '\xff';
        }
      }
    } else {
      for (int j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (int i = 0; i < width(); i++) {
          if (*data > threshold) {
            *line++ = '\0';
            *line++ = '\0';
            *line++ = '\xff';
            *line++ = '\xff';
            ++data;
          } else {
            glm::col3 col3 = colorMap[*data++];

            /*glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                                glm::vec3(0.f), glm::vec3(255.f)));
                                                */

            *line++ = col3[0];
            *line++ = col3[1];
            *line++ = col3[2];
            *line++ = '\xff';
          }
        }
      }
    }
  }
}

void ZImage::setDataBlockIndexed8(
    const ZImage::DataSource<uint8_t> &source, int startLine,
    int endLine, int threshold)
{
  const uint8_t* data = source.data + startLine * width();
  float scale = source.scale;
  float offset = source.offset;
  glm::vec3 color = glm::vec3(source.color.b, source.color.g, source.color.r);

  glm::col3 colorMap[256];
  for (int i = 0; i < 256; ++i) {
    if (scale == 1.f && offset == 0.f) {
      colorMap[i] = glm::col3(color * (float) i);
    } else {
      colorMap[i] = glm::col3(glm::clamp(color * (scale * i + offset),
                                         glm::vec3(0.f), glm::vec3(255.f)));
    }
  }

  int i, j;
  for (j = startLine; j < endLine; j++) {
    uchar *line = scanLine(j);
    for (i = 0; i < width(); i++) {
      *line++ = *data++;
    }
  }

  for (int i = 0; i < 256; ++i) {
    if (i <= threshold) {
      setColor(i, qRgb(255, 0, 0));
    } else {
      int r = iround(colorMap[i][0] * 255);
      int g = iround(colorMap[i][1] * 255);
      int b = iround(colorMap[i][2] * 255);

      setColor(i, qRgb(r, g, b));
    }
  }
}

void ZImage::setDataBlock(const ZImage::DataSource<uint8_t> &source, int startLine,
                          int endLine, int threshold)
{
  if (format() == Format_Indexed8) {
    setDataBlockIndexed8(source, startLine, endLine, threshold);
    return;
  }

  const uint8_t* data = source.data + startLine * width();
  float scale = source.scale;
  float offset = source.offset;
  glm::vec3 color = glm::vec3(source.color.b, source.color.g, source.color.r);

  glm::col3 colorMap[256];
  for (int i = 0; i < 256; ++i) {
      if (scale == 1.f && offset == 0.f) {
          colorMap[i] = glm::col3(color * (float) i);
      } else {
          colorMap[i] = glm::col3(glm::clamp(color * (scale * i + offset),
                                             glm::vec3(0.f), glm::vec3(255.f)));
      }
  }

  if (scale == 1.f && offset == 0.f) {
    if (threshold < 0) {
      int i, j;
      for (j = startLine; j < endLine; j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          //glm::col3 col3 = glm::col3(color * (float)(*data++));
          glm::col3 col3 = colorMap[*data++];

          *line++ = col3[0];
          *line++ = col3[1];
          *line++ = col3[2];
          *line++ = '\xff';
        }
      }
    } else {
      int i, j;
      for (j = startLine; j < endLine; j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          if (*data > threshold) {
            *line++ = '\0';
            *line++ = '\0';
            *line++ = '\xff';
            *line++ = '\xff';
            ++data;
          } else {
            //glm::col3 col3 = glm::col3(color * (float)(*data++));
            glm::col3 col3 = colorMap[*data++];

            *line++ = col3[0];
            *line++ = col3[1];
            *line++ = col3[2];
            *line++ = '\xff';
          }
        }
      }
    }
    return;
  }

  if (threshold < 0) {
    int i, j;
    for (j = startLine; j < endLine; j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        /*
        glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                              glm::vec3(0.f), glm::vec3(255.f)));
                                              */

        glm::col3 col3 = colorMap[*data++];

        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = '\xff';
      }
    }
  } else {
    int i, j;
    for (j = startLine; j < endLine; j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        if (*data > threshold) {
          *line++ = '\0';
          *line++ = '\0';
          *line++ = '\xff';
          *line++ = '\xff';
          ++data;
        } else {
          /*
          glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                                glm::vec3(0.f), glm::vec3(255.f)));
                                                */
          glm::col3 col3 = colorMap[*data++];

          *line++ = col3[0];
          *line++ = col3[1];
          *line++ = col3[2];
          *line++ = '\xff';
        }
      }
    }
  }
}

void ZImage::setDataIndexed8(
    const std::vector<ZImage::DataSource<uint8_t> > &sources,
    uint8_t alpha, bool useMultithread)
{
  if (useMultithread) {
    int numBlock = std::min(height(), 16);
    int blockHeight = height() / numBlock;
    std::vector<QFuture<void> > res(numBlock);
    for (int i=0; i<numBlock; ++i) {
      int startLine = i * blockHeight;
      int endLine = (i+1) * blockHeight;
      if (i == numBlock - 1)
        endLine = height();
      res[i] = QtConcurrent::run(this, &ZImage::setDataBlockMS8Indexed8,
                                 sources, startLine, endLine, alpha);
    }
    for (int i=0; i<numBlock; ++i)
      res[i].waitForFinished();
    return;
  }

  std::vector<ZImage::DataSource<uint8_t> > allSources = sources;
#ifdef _DEBUG_2
    std::cout << "Color: " << " " << sources[0].color << std::endl;
    std::cout << "Color: " << " " << sources[1].color << std::endl;
    std::cout << "Color: " << " " << sources[2].color << std::endl;
#endif
  bool needScaleAndClamp = false;
  for (size_t i=0; i<allSources.size(); ++i) {
    std::swap(allSources[i].color.r, allSources[i].color.b);
    if (allSources[i].scale != 1.f || allSources[i].offset != 0.f) {
      needScaleAndClamp = true;
    }
  }


  std::vector<std::vector<glm::col3> > colorMap(allSources.size());
  for (size_t i = 0; i < colorMap.size(); ++i) {
    colorMap[i].resize(256);
  }

#ifdef _DEBUG_2
        std::cout << "Data value" << (int) allSources[2].data[0] << std::endl;
#endif

  if (!needScaleAndClamp) {
    for (int i = 0; i < 256; ++i) {
      colorMap[0][i] = glm::col3(allSources[0].color * (float) i);
    }
  } else {
    for (int i = 0; i < 256; ++i) {
      colorMap[0][i] = glm::col3(
            glm::clamp(allSources[0].color *
            (allSources[0].scale * i + allSources[0].offset),
          glm::vec3(0.f), glm::vec3(255.f)));
    }
  }

  for (int j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < width(); i++) {
      *line++ = *(allSources[0].data)++;
    }
  }

  for (int i = 0; i < 256; ++i) {
    int r = iround(colorMap[0][i][0] * 255);
    int g = iround(colorMap[0][i][1] * 255);
    int b = iround(colorMap[0][i][2] * 255);

    setColor(i, qRgba(r, g, b, alpha));
  }
}

void ZImage::setData(const std::vector<ZImage::DataSource<uint8_t> > &sources,
                     uint8_t alpha, bool useMultithread)
{
  if (useMultithread) {
    int numBlock = std::min(height(), 16);
    int blockHeight = height() / numBlock;
    std::vector<QFuture<void> > res(numBlock);
    for (int i=0; i<numBlock; ++i) {
      int startLine = i * blockHeight;
      int endLine = (i+1) * blockHeight;
      if (i == numBlock - 1)
        endLine = height();
      res[i] = QtConcurrent::run(this, &ZImage::setDataBlockMS8,
                                 sources, startLine, endLine, alpha);
    }
    for (int i=0; i<numBlock; ++i)
      res[i].waitForFinished();
    return;
  }

  std::vector<ZImage::DataSource<uint8_t> > allSources = sources;
#ifdef _DEBUG_2
    std::cout << "Color: " << " " << sources[0].color << std::endl;
    std::cout << "Color: " << " " << sources[1].color << std::endl;
    std::cout << "Color: " << " " << sources[2].color << std::endl;
#endif
  bool needScaleAndClamp = false;
  for (size_t i=0; i<allSources.size(); ++i) {
    std::swap(allSources[i].color.r, allSources[i].color.b);
    if (allSources[i].scale != 1.f || allSources[i].offset != 0.f) {
      needScaleAndClamp = true;
    }
  }


  std::vector<std::vector<glm::col3> > colorMap(allSources.size());
  for (size_t i = 0; i < colorMap.size(); ++i) {
    colorMap[i].resize(256);
  }

#ifdef _DEBUG_2
        std::cout << "Data value" << (int) allSources[2].data[0] << std::endl;
#endif

  if (!needScaleAndClamp) {
    for (int i = 0; i < 256; ++i) {
      colorMap[0][i] = glm::col3(allSources[0].color *
          (float) i);
      for (size_t ch=1; ch<allSources.size(); ++ch) {
        colorMap[ch][i] = glm::col3(allSources[ch].color * (float) i);
      }
    }

#ifdef _DEBUG_2
    std::cout << "Color: " << " " << allSources[0].color << std::endl;
    std::cout << "Color: " << " " << allSources[1].color << std::endl;
    std::cout << "Color: " << " " << allSources[2].color << std::endl;
#endif

    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        glm::col3 col3 = colorMap[0][*(allSources[0].data)++];
        for (size_t ch=1; ch<allSources.size(); ++ch) {
          col3 = glm::max(col3, colorMap[ch][*(allSources[ch].data)++]);
        }

#ifdef _DEBUG_2
        std::cout << "Color map: " << " " << colorMap[0][1] << std::endl;
        std::cout << "Color map: " << " " << colorMap[1][1] << std::endl;
        std::cout << "Color map: " << " " << colorMap[2][1] << std::endl;
#endif
        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = alpha;
      }
    }
  } else {
    for (int i = 0; i < 256; ++i) {
      colorMap[0][i] = glm::col3(
            glm::clamp(allSources[0].color *
            (allSources[0].scale * i + allSources[0].offset),
          glm::vec3(0.f), glm::vec3(255.f)));
      for (size_t ch=1; ch<allSources.size(); ++ch) {
        colorMap[ch][i] = glm::col3(glm::clamp(allSources[ch].color *
                                               (allSources[ch].scale * i + allSources[ch].offset),
                                               glm::vec3(0.f), glm::vec3(255.f)));
      }
    }

    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        glm::col3 col3 = colorMap[0][*(allSources[0].data)++];
        for (size_t ch=1; ch<allSources.size(); ++ch) {
          col3 = glm::max(col3, colorMap[ch][*(allSources[ch].data)++]);
        }
#ifdef _DEBUG_2
        std::cout << "Color: " << " " << colorMap[2][10] << std::endl;
#endif

        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = alpha;
      }
    }
  }
}

void ZImage::setDataBlockMS8Indexed8(
    const std::vector<ZImage::DataSource<uint8_t> > &sources,
    int startLine, int endLine, uint8_t /*alpha*/)
{
  std::vector<ZImage::DataSource<uint8_t> > allSources = sources;
  bool needScaleAndClamp = false;
  for (size_t i=0; i<allSources.size(); ++i) {
    allSources[i].data += startLine * width();
    std::swap(allSources[i].color.r, allSources[i].color.b);
    if (allSources[i].scale != 1.f || allSources[i].offset != 0.f) {
      needScaleAndClamp = true;
    }
  }

  for (int j = startLine; j < endLine; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < width(); i++) {
      *line++ = *(allSources[0].data)++;
    }
  }
}

void ZImage::setDataBlockMS8(
    const std::vector<ZImage::DataSource<uint8_t> > &sources,
    int startLine, int endLine, uint8_t alpha)
{
  if (format() == Format_Indexed8) {
    setDataBlockMS8Indexed8(sources, startLine, endLine, alpha);
    return;
  }

  std::vector<ZImage::DataSource<uint8_t> > allSources = sources;
  bool needScaleAndClamp = false;
  for (size_t i=0; i<allSources.size(); ++i) {
    allSources[i].data += startLine * width();
    std::swap(allSources[i].color.r, allSources[i].color.b);
    if (allSources[i].scale != 1.f || allSources[i].offset != 0.f) {
      needScaleAndClamp = true;
    }
  }

  std::vector<std::vector<glm::col3> > colorMap(allSources.size());
  for (size_t i = 0; i < colorMap.size(); ++i) {
    colorMap[i].resize(256);
  }

  if (!needScaleAndClamp) {
    for (int i = 0; i < 256; ++i) {
      colorMap[0][i] = glm::col3(allSources[0].color *
          (float) i);
      for (size_t ch=1; ch<allSources.size(); ++ch) {
        colorMap[ch][i] = glm::col3(allSources[ch].color * (float) i);
      }
    }

    for (int j = startLine; j < endLine; j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        glm::col3 col3 = colorMap[0][*(allSources[0].data)++];
        for (size_t ch=1; ch<allSources.size(); ++ch) {
          col3 = glm::max(col3, colorMap[ch][*(allSources[ch].data)++]);
        }

        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = alpha;
      }
    }
    return;
  }

  for (int i = 0; i < 256; ++i) {
    colorMap[0][i] = glm::col3(
          glm::clamp(allSources[0].color *
          (allSources[0].scale * i + allSources[0].offset),
        glm::vec3(0.f), glm::vec3(255.f)));
    for (size_t ch=1; ch<allSources.size(); ++ch) {
      colorMap[ch][i] = glm::col3(glm::clamp(allSources[ch].color *
                                             (allSources[ch].scale * i + allSources[ch].offset),
                                             glm::vec3(0.f), glm::vec3(255.f)));
    }
  }

  for (int j = startLine; j < endLine; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < width(); i++) {
      glm::col3 col3 = colorMap[0][*(allSources[0].data)++];
      for (size_t ch=1; ch<allSources.size(); ++ch) {
        col3 = glm::max(col3, colorMap[ch][*(allSources[ch].data)++]);
      }

      *line++ = col3[0];
      *line++ = col3[1];
      *line++ = col3[2];
      *line++ = alpha;
    }
  }
}

bool ZImage::writeImage(const QImage &image, const QString &filename)
{
  QImageWriter writer(filename);
  writer.setCompression(1);
  if (!writer.write(image)) {
    writer.setCompression(0);
    if (!writer.write(image)) {
      if (ZFileType::FileType(filename.toStdString()) == ZFileType::FILE_TIFF) {
        Stack *stack = C_Stack::make(COLOR, image.width(), image.height(), 1);
        color_t *arrayc = (color_t*) stack->array;
        size_t index = 0;
        for (int y = 0; y < image.height(); ++y) {
          for (int x = 0; x < image.width(); ++x) {
            QRgb color = image.pixel(x, y);
            arrayc[index][0] = qRed(color);
            arrayc[index][1] = qGreen(color);
            arrayc[index][2] = qBlue(color);

            index++;
          }
        }
        C_Stack::write(filename.toStdString(), stack);
        C_Stack::kill(stack);
      } else {
        LERROR() << writer.errorString();
        return false;
      }
    }
  }

  return true;
}

const ZStTransform& ZImage::getTransform() const
{
  return m_transform;
}

void ZImage::setTransform(const ZStTransform &transform)
{
  m_transform = transform;
}

void ZImage::setScale(double sx, double sy)
{
  m_transform.setScale(sx, sy);
}

void ZImage::setOffset(double dx, double dy)
{
  m_transform.setOffset(dx, dy);
}
