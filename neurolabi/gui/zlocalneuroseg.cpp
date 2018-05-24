#if defined(_QT_GUI_USED_)
#include <QtConcurrentRun>
#endif

#include "tz_utilities.h"
#include "tz_math.h"
#include "zlocalneuroseg.h"
#include "tz_voxel_graphics.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_attribute.h"
#include "zpainter.h"
#include "c_stack.h"

ZLocalNeuroseg::ZLocalNeuroseg(Local_Neuroseg *locseg, bool isOwner)
{
  m_locseg = locseg;
  m_zscale = 1.0;
  m_profile = NULL;
  m_filterStack = NULL;
  m_isOwner = isOwner;
  setTarget(ZStackObject::TARGET_OBJECT_CANVAS);
}

ZLocalNeuroseg::~ZLocalNeuroseg()
{
#if defined(_QT_GUI_USED_)
  if (m_future.isRunning()) {
    m_future.waitForFinished();
  }
#endif

  if (m_isOwner == true) {
    Delete_Local_Neuroseg(m_locseg);
  }

  if (m_profile != NULL) {
    delete []m_profile;
  }
  if (m_filterStack != NULL) {
    C_Stack::kill(m_filterStack);
    m_filterStack = NULL;
  }
}

void ZLocalNeuroseg::display(
    ZPainter &painter, int sliceIndex, EDisplayStyle option,
    const QColor &color) const
{ //todo
#if defined(_QT_GUI_USED_)
  if (option == ZStackObject::NORMAL) {
    option = ZStackObject::SOLID;
  }

  if ((option == ZStackObject::SOLID) || (option == ZStackObject::BOUNDARY)) {
    if (m_locseg->seg.r1 * m_locseg->seg.scale <= 0.0) {
      return;
    }
  }

  double center_position[3];
  Local_Neuroseg_Center(m_locseg, center_position);

  int z = painter.getZ(sliceIndex);

  if (sliceIndex >= 0) {
    /* Estimation of z range */
    double r = Neuroseg_Z_Range(&(m_locseg->seg)) / 2.0 + 1.0;
    if (!(IS_IN_CLOSE_RANGE(painter.getZ(sliceIndex), (center_position[2] - r) * m_zscale,
                            (center_position[2] + r) * m_zscale))) {
      return;
    }
  }

  double bottom_position[3];
  Local_Neuroseg_Bottom(m_locseg, bottom_position);

  switch(option) {
  case ZStackObject::SOLID:
  case ZStackObject::BOUNDARY:
    {
      double offpos[3];
      int c[3];          /* position of the original point in filter range */

      Local_Neuroseg_Stack_Position(bottom_position, c, offpos, m_zscale);

      if (m_filterStack == NULL) {
        if (!m_future.isRunning()) {
          asyncGenerateFilterStack();
        }
        m_future.waitForFinished();
      }
      int i;
      int region_corner[3];

      for (i = 0; i < 3; i++) {
        region_corner[i] = m_fieldRange.first_corner[i] + c[i];
      }

      if (sliceIndex >= 0) {
        if (!(IS_IN_CLOSE_RANGE(z, region_corner[2],
                                region_corner[2] + m_fieldRange.size[2] - 1))) {
          return;
        }
      }

      int point[3];
      int offset = 0;

      int area = m_filterStack->height * m_filterStack->width;
      int j;

      if (sliceIndex >= 0) {
        offset = (z - region_corner[2]) * area;
        for (j = 0; j < m_filterStack->height; j++) {
          point[1] = region_corner[1] + j;
          //uchar *line = widget->scanLine(point[1]);
          for (i = 0; i < m_filterStack->width; i++) {
            point[0] = region_corner[0] + i;
            /*
            if ((point[0] >= 0) && (point[0] < widget->width()) &&
                (point[1] >= 0) && (point[1] < widget->height()) &&
                (m_filterStack->array[offset] > 0)) {
                */
            if (m_filterStack->array[offset] > 0) {
              if (option == ZStackObject::BOUNDARY) {
                int k = sliceIndex - region_corner[2];
                if (IS_IN_OPEN_RANGE3(i, j, k, 0, m_filterStack->width-1,
                                      0, m_filterStack->height - 1,
                                      0, m_filterStack->depth - 1)) {
                  if (Stack_Neighbor_Min(m_filterStack, 4, i, j, k) > 0.0) {
                    offset++;
                    continue;
                  }
                }
              }

              int v = m_filterStack->array[offset];
              painter.setPen(QColor(color.red() * v / 255, color.green() * v / 255,
                                    color.blue() * v / 255, color.alpha()));
              painter.drawPoint(QPointF(point[0], point[1]));

            }
            offset++;
          }
        }
      } else { /* projection view */
        int k;
        offset = 0;
        for (j = 0; j < m_filterStack->height; j++) {
          point[1] = region_corner[1] + j;
          //uchar *line = widget->scanLine(point[1]);
          for (i = 0; i < m_filterStack->width; i++) {
            point[0] = region_corner[0] + i;
            /*if ((point[0] >= 0) && (point[0] < widget->width()) &&
                (point[1] >= 0) && (point[1] < widget->height())) {
                */
            if (1) {
              int v = m_filterStack->array[offset];
              int new_offset = offset;
              for (k = 1; k < m_filterStack->depth; k++) {
                if (v < m_filterStack->array[new_offset]) {
                  v = m_filterStack->array[new_offset];
                }
                new_offset += area;
              }
              if (option == ZStackObject::BOUNDARY) {
                int v2;
                int neighbor[4];
                Stack_Neighbor_Offset(4, Stack_Width(m_filterStack),
                                      Stack_Height(m_filterStack), neighbor);
                int is_in_bound[4];
                int n_in_bound =
                    Stack_Neighbor_Bound_Test_I(4,
                                                Stack_Width(m_filterStack),
                                                Stack_Height(m_filterStack),
                                                Stack_Depth(m_filterStack),
                                                offset, is_in_bound);
                if (n_in_bound == 4) {
                  int n;
                  for (n = 0; n < 4; n++) {
                    int noffset = offset+neighbor[n];
                    new_offset = noffset;
                    v2 = m_filterStack->array[noffset];
                    for (k = 1; k < m_filterStack->depth; k++) {
                      if (v2 < m_filterStack->array[new_offset]) {
                        v2 = m_filterStack->array[new_offset];
                      }
                      new_offset += area;
                    }
                    if (v2 <= 0) {
                      break;
                    }
                  }
                  if (n == 4) {
                    v = 0;
                  }
                }
              }
              if (v > 0) {
                painter.setPen(QColor(color.red() * v / 255, color.green() * v / 255,
                                      color.blue() * v / 255, color.alpha()));
                painter.drawPoint(QPointF(point[0], point[1]));

                /*
                uchar *pixel = line + 4 * point[0];
                pixel[RED] = color.red() * v / 255;
                pixel[BLUE] = color.blue() * v / 255;
                pixel[GREEN] = color.green() * v / 255;
                pixel[3] = color.alpha();
                */
              }
            }
            offset++;
          }
        }
      }
    }
    break;
        
  case ZStackObject::SKELETON:
    {
      double top_position[3];
      Local_Neuroseg_Top(m_locseg, top_position);
      voxel_t start, end;
      for (int i = 0; i < 3; i++) {
        start[i] = iround(bottom_position[i]);
        end[i] = iround(top_position[i]);
      }

      Object_3d *obj = Line_To_Object_3d(start, end);

      for (size_t i = 0; i < obj->size; i++) {
        /*
        if (((obj->voxels[i][2] == z) || (z < 0))&&
            IS_IN_CLOSE_RANGE(obj->voxels[i][0], 0, widget->width() - 1) &&
            IS_IN_CLOSE_RANGE(obj->voxels[i][1], 0, widget->height() - 1)){
          uchar *pixel = widget->scanLine(obj->voxels[i][1])
                         + 4 * obj->voxels[i][0];
          pixel[RED] = color.red();
          pixel[BLUE] = color.blue();
          pixel[GREEN] = color.green();
          pixel[3] = color.alpha();
        }
        */
      }

      Kill_Object_3d(obj);
    }
    break;

  default:
    //display(locseg, z_scale, image, n, color, ZStackObject::NORMAL);
    break;
  }
#else
  UNUSED_PARAMETER(&painter);
  UNUSED_PARAMETER(option);
  UNUSED_PARAMETER(&color);
#endif
}

void ZLocalNeuroseg::display(QImage *image, int n, Palette_Color color,
                             EDisplayStyle style, int label) const
{
#if defined(_QT_GUI_USED_)
  double center_position[3];
  Local_Neuroseg_Center(m_locseg, center_position);

  if (n >= 0) {
    double r = Neuroseg_Z_Range(&(m_locseg->seg)) / 2.0 + 1.0;
    if (!(IS_IN_CLOSE_RANGE(n, (center_position[2] - r) * m_zscale,
                            (center_position[2] + r) * m_zscale))) {
      return;
    }
  }

  if (style == ZStackObject::NORMAL) {
    style = ZStackObject::SOLID;
  }

  int channel[3];
  switch (color) {
  case RED:
    channel[0] = 0;
    channel[1] = 1;
    channel[2] = 2;
    break;
  case GREEN:
    channel[0] = 2;
    channel[1] = 0;
    channel[2] = 1;
    break;

  case BLUE:
    channel[0] = 1;
    channel[1] = 2;
    channel[2] = 0;
    break;

  default:
    channel[0] = 0;
    channel[1] = 1;
    channel[2] = 2;
  }

  double bottom_position[3];
  Local_Neuroseg_Bottom(m_locseg, bottom_position);

  switch(style) {
  case ZStackObject::SOLID:
  case ZStackObject::BOUNDARY:
    {
      double offpos[3];
      int c[3];          /* position of the original point in filter range */

      Local_Neuroseg_Stack_Position(bottom_position, c, offpos, m_zscale);

      if (m_filterStack == NULL) {
#if defined(_QT_GUI_USED_)
        if (!m_future.isRunning()) {
          asyncGenerateFilterStack();
        }
        m_future.waitForFinished();
#endif
      }
      //      if (m_filterStack == NULL) {
      //        LERROR() << "This should never happen.";
      //        Neuroseg_Field_Range(&(m_locseg->seg), &m_fieldRange, m_zscale);

      //        double *filter = Neurofilter(&(m_locseg->seg), NULL, NULL,
      //                                     &m_fieldRange, offpos, m_zscale);
      //        m_filterStack = Scale_Double_Stack_P(filter, m_fieldRange.size[0],
      //                                             m_fieldRange.size[1],
      //                                             m_fieldRange.size[2], GREY);
      //        free(filter);
      //      }

      int i;
      int region_corner[3];

      for (i = 0; i < 3; i++) {
        region_corner[i] = m_fieldRange.first_corner[i] + c[i];
      }

      if (n >= 0) {
        if (!(IS_IN_CLOSE_RANGE(n, region_corner[2],
                                region_corner[2] + m_fieldRange.size[2] - 1))) {
          return;
        }
      }

      int point[3];
      int offset = 0;

      int area = Stack_Plane_Area(m_filterStack);
      int j, k;

      for (k = 0; k < m_filterStack->depth; k++) {
        point[2] = region_corner[2] + k;

        if ((point[2] == n) || (n < 0)){
          for (j = 0; j < m_filterStack->height; j++) {
            point[1] = region_corner[1] + j;
            uchar *line = image->scanLine(point[1]);
            for (i = 0; i < m_filterStack->width; i++) {
              point[0] = region_corner[0] + i;
              if ((point[0] >= 0) && (point[0] < image->width()) &&
                  (point[1] >= 0) && (point[1] < image->height()) &&
                  (/*filter[offset] > 0*/ m_filterStack->array[offset] > 0)) {
                if (style == ZStackObject::BOUNDARY) {
                  if (Stack_Neighbor_Min(m_filterStack,
                                         6, point[0], point[1], point[2])
                    > 0.0) {
                    continue;
                  }
                }
                uchar *pixel = line + 4 * point[0];
                pixel[channel[0]] = 0;
                pixel[channel[1]] = 0;

                if (n >= 0) {
                  pixel[channel[2]] = m_filterStack->array[offset];
                } else {
                  if (k == 0) {
                    pixel[channel[2]] = 0;
                  }
                  if (pixel[channel[2]] < m_filterStack->array[offset]) {
                    pixel[channel[2]] = m_filterStack->array[offset];
                  }
                }
                pixel[3] = 255;
                switch (label) {
                case 1:
                  pixel[channel[1]] = pixel[channel[2]] / 2;
                  pixel[channel[0]] = pixel[channel[2]] / 2;
                  break;
                case 2:
                  pixel[channel[1]] = pixel[channel[2]];
                  break;
                default:
                  break;
                }
              }
              offset++;
            }
          }
        } else {
          offset += area;
        }
      }
      //free(filter);

    }
    break;
  case ZStackObject::SKELETON:
    {
      double top_position[3];
      Local_Neuroseg_Top(m_locseg, top_position);
      voxel_t start, end;
      for (int i = 0; i < 3; i++) {
        start[i] = iround(bottom_position[i]);
        end[i] = iround(top_position[i]);
      }

      Object_3d *obj = Line_To_Object_3d(start, end);

      for (size_t i = 0; i < obj->size; i++) {
        if (((obj->voxels[i][2] == n) || (n < 0))&&
            IS_IN_CLOSE_RANGE(obj->voxels[i][0], 0, image->width() - 1) &&
            IS_IN_CLOSE_RANGE(obj->voxels[i][1], 0, image->height() - 1)){
          uchar *pixel = image->scanLine(obj->voxels[i][1])
                         + 4 * obj->voxels[i][0];
          pixel[channel[0]] = 0;
          pixel[channel[1]] = 0;
          pixel[channel[2]] = 255;
          pixel[3] = 255;
          switch (label) {
          case 1:
            pixel[channel[1]] = pixel[channel[2]] / 2;
            pixel[channel[0]] = pixel[channel[2]] / 2;
            break;
          case 2:
            pixel[channel[1]] = pixel[channel[2]];
            break;
          default:
            break;
          }
        }
      }

      Kill_Object_3d(obj);
    }
    break;

  default:
    //display(locseg, z_scale, image, n, color, ZStackObject::NORMAL);
    break;
  }
#endif
}


void ZLocalNeuroseg::display(
    ZPainter &painter, int z, EDisplayStyle option,
    neutube::EAxis /*axis*/) const
{
  display(painter, z, option, getColor());
}

void ZLocalNeuroseg::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

bool ZLocalNeuroseg::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);

  return false;
}

void ZLocalNeuroseg::updateProfile(const Stack *stack, int option)
{
  if (stack != NULL) {
    int nsample = 11;
    if (m_profile == NULL) {
      m_profile = new double[nsample];
    }

    Local_Neuroseg_Height_Profile(m_locseg, stack, m_zscale, nsample,
                                  option, NULL, m_profile);
  }
}

void ZLocalNeuroseg::topPosition(double pos[]) const
{
  Neuroseg_Top(&(m_locseg->seg), pos);
  pos[0] += m_locseg->pos[0];
  pos[1] += m_locseg->pos[1];
  pos[2] += m_locseg->pos[2];
}

void ZLocalNeuroseg::bottomPosition(double pos[]) const
{
  Neuroseg_Bottom(&(m_locseg->seg), pos);
  pos[0] += m_locseg->pos[0];
  pos[1] += m_locseg->pos[1];
  pos[2] += m_locseg->pos[2];
}

void ZLocalNeuroseg::asyncGenerateFilterStack() const
{
#if defined(_QT_GUI_USED_)
  if (m_filterStack || m_future.isRunning())
    return;
  m_future = QtConcurrent::run(const_cast<ZLocalNeuroseg*>(this),
                               &ZLocalNeuroseg::generateFilterStack);
  m_future.waitForFinished();
#endif
}

void ZLocalNeuroseg::generateFilterStack()
{
  double bottom_position[3];
  Local_Neuroseg_Bottom(m_locseg, bottom_position);
  double offpos[3];
  int c[3];          /* position of the original point in filter range */
  Local_Neuroseg_Stack_Position(bottom_position, c, offpos, m_zscale);

  Neuroseg_Field_Range(&(m_locseg->seg), &m_fieldRange, m_zscale);

  double *filter = Neurofilter(&(m_locseg->seg), NULL, NULL,
                               &m_fieldRange, offpos, m_zscale);
  if (m_filterStack != NULL) {
    C_Stack::kill(m_filterStack);
    m_filterStack = NULL;
  }
  m_filterStack = Scale_Double_Stack_P(filter, m_fieldRange.size[0],
                                       m_fieldRange.size[1],
                                       m_fieldRange.size[2], GREY);
  free(filter);

#ifdef _DEBUG_2
  C_Stack::write("/Users/zhaot/Work/neutube/neurolabi/data/test.tif", m_filterStack);
#endif
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZLocalNeuroseg)
