#include <QtGui>
#include <iostream>
#include "tz_error.h"
#include "zimage.h"
#include "tz_stack_neighborhood.h"
#include "zstack.hxx"
#include "zfiletype.h"
#include "zobject3dscan.h"
#include "zobject3dstripe.h"
#include "neutube_def.h"

ZImage::ZImage() : QImage()
{
}

ZImage::ZImage(int width, int height, QImage::Format format) :
  QImage(width, height, format)
{
  memset(scanLine(0), 255, this->bytesPerLine() * height);
  /*
  int i, j;
  for (j = 0; j < height; j++) {
    char *line = (char*) scanLine(j);
    for (i = 0; i < width; i++) {
      line[3] = '\xff';
      line += 4;
    }
  }*/

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


        int tx0 = imax2(stack->getOffset().getX(), 0);
        int ty0 = imax2(stack->getOffset().getY(), 0);
        int tx1 = imin2(tx0 + sourceWidth, tx0 + targetWidth);
        int ty1 = imin2(ty0 + sourceHeight, ty0 + targetHeight);
        int sx = tx0;
        int sy = ty0;

        uchar *line = NULL;
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

void ZImage::clear()
{
  uchar *line = scanLine(0);
  bzero(line, this->byteCount());
//  for (int i = 0; i < this->byteCount()
}

void ZImage::setData(const uint8 *data, int threshold)
{
  int i, j;
  int w = width();
  int h = height();

  if (threshold < 0) {
    for (j = 0; j < h; j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < w; i++) {
//        *line++ = *data;
//        *line++ = *data;
//        *line++ = *data++;
        memset(line, *data++, 3);
        line += 3;
        *line++ = '\xff';
      }
    }
  } else {
    int i, j;

    for (j = 0; j < h; j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < w; i++) {
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
}

void ZImage::setData(
    const uint8 *data, int stackWidth, int stackHeight, int /*stackDepth*/,
    int slice, NeuTube::EAxis sliceAxis)
{
  int imageWidth = width();
  int imageHeight = height();
  int area = stackWidth * stackHeight;

  switch (sliceAxis) {
  case NeuTube::Z_AXIS:
  {
    data += (size_t) area * slice;
    for (int j = 0; j < imageHeight; j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < imageWidth; i++) {
        *line++ = *data;
        *line++ = *data;
        *line++ = *data;
        data++;
        *line++ = 255;
      }
    }
  }
    break;
  case NeuTube::Y_AXIS:
  {
    const uint8 *dataOrigin = data + slice * stackWidth;

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
    break;
  case NeuTube::X_AXIS:
  {
    const uint8 *dataOrigin = data + slice;
//    data += slice;
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
    break;
  }
}

void ZImage::setData(const color_t *data, int alpha)
{
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

void ZImage::setCData(const color_t *data, double scale, double offset)
{
  int i, j;
  if (scale == 1.0 && offset == 0.0) {
    for (j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        *line++ = (*data)[2];
        *line++ = (*data)[1];
        *line++ = (*data)[0];
        data++;
        //*line++ = '\xff';
        line++;
      }
    }
  } else {
    for (j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
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
        //*line++ = '\xff';
        line++;
      }
    }
  }
}

void ZImage::setData(const uint8 *data, double scale, double offset,
                     int threshold)
{
#ifdef _DEBUG_2
  tic();
#endif
  if (scale == 1.0 && offset == 0.0) {
    if (threshold < 0) {
      int i, j;
      int w = width();
      for (j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < w; i++) {
          memset(line, *data++, 3);
          line += 3;
          *line++ = '\xff';
        }
      }
    } else {
      int i, j;
      for (j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
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

    if (threshold < 0) {
      int i, j;
      for (j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          //memset(line, valueMap[*data++], 3);
          uint8 v = valueMap[*data++];
          *line++ = v;
          *line++ = v;
          *line++ = v;
          line++;
        }
      }
    } else {
      int i, j;
      for (j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          if (*data > threshold) {
            *line++ = '\0';
            *line++ = '\0';
            *line++ = '\xff';
            ++data;
          } else {
            uint8 v = valueMap[*data++];
            *line++ = v;
            *line++ = v;
            *line++ = v;
          }

          *line++ = '\xff';
        }
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

void ZImage::enhanceContrast(bool highContrast)
{
  if (format() != ZImage::Format_Indexed8) {
    int i, j;
    if (this->depth() == 32) {
      for (j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
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
    } else if (this->depth() == 8) {
      for (j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          if (line[0] <= 213) {
            line[0] += line[0] / 5;
          } else {
            line[0] = 255;
          }

          line++;
        }
      }
    }
  } else {
    if (highContrast) {
      for (int i = 0; i < 255; ++i) {
        QColor color;
        double v = i / 255.0;
        v *= sqrt(v);
        v = v * 1.5;
        if (v > 1.0) {
          v = 1.0;
        }
        color.setRedF(v);
        color.setGreenF(v);
        color.setBlueF(v);
        setColor(i, color.rgb());
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

void ZImage::setDataBlock(const ZImage::DataSource<uint8_t> &source, int startLine,
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

void ZImage::setDataBlockMS8(
    const std::vector<ZImage::DataSource<uint8_t> > &sources,
    int startLine, int endLine, uint8_t alpha)
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
      if (ZFileType::fileType(filename.toStdString()) == ZFileType::TIFF_FILE) {
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
