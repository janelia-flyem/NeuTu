#include "zgeometry.h"
#include <cmath>
#include "zpoint.h"
#include "geometry/zaffinerect.h"


std::vector<ZAffineRect> zgeom::Partition(
    const ZAffineRect &rect, int row, int col)
{
  std::vector<ZAffineRect> result;

  int subwidth = rect.getWidth() / col;
  int subheight = rect.getHeight() / row;
  int heightRemainder = rect.getHeight() % row;

  int currentOffsetY = -rect.getHeight() / 2;
  for (int i = 0; i < row; ++i) {
    int currentOffsetX = -rect.getWidth() / 2;
    int height = subheight;
    if (heightRemainder > 0) {
      ++height;
      --heightRemainder;
    }
    int widthRemainder = rect.getWidth() % col;

    for (int j = 0; j < col; ++j) {
      int width = subwidth;
      if (widthRemainder > 0) {
        ++width;
        --widthRemainder;
      }

      ZAffineRect subrect;

      ZPoint center = rect.getV1() * (currentOffsetX + width / 2) +
          rect.getV2() * (currentOffsetY + height / 2) + rect.getCenter();
      subrect.set(center, rect.getV1(), rect.getV2(), width, height);

      result.push_back(subrect);

      currentOffsetX += width;
    }
    currentOffsetY += height;
  }

  return result;
}

void zgeom::transform(ZGeo3dScalarField *field,
                     const ZGeo3dTransform &transform)
{
  transform.transform(field->getRawPointArray(), field->getPointNumber());
}

std::vector<ZPoint> zgeom::LineShpereIntersection(
    const ZPoint &lineStart, const ZPoint &lineNorm,
    const ZPoint &center, double r)
{
  std::vector<ZPoint> result;

  ZPoint dc = lineStart - center;

  double b = dc.dot(lineNorm);
  double c = dc.lengthSqure() - r * r;
  double q = b * b - c;

  if (q >= 0) {
    double d = -b - std::sqrt(q);
    result.push_back(lineStart + lineNorm * d);
    d = -b + std::sqrt(q);
    result.push_back(lineStart + lineNorm * d);
  }

  return result;
}

std::vector<std::pair<int, int> > LineToPixel(int x0, int y0, int x1, int y1)
{
  std::vector<std::pair<int, int>> result;

  int dx = x1 - x0;
  int dy = y1 - y0;
  int x = x0;
  int y = y0;

  if (dy >= 0) {
    if (dx >= 0 && (dx >= dy)) {
      int p = 2 * dy - dx;
      while (x <= x1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          ++y;
          p = p + 2 * dy - 2 * dx;
        } else {
          result.emplace_back(x, y);
          p = p + 2 * dy;
        }
        ++x;
      }
    } else if (dx >= 0) {
      int p = 2 * dx - dy;
      while (y <= y1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          ++x;
          p = p + 2 * dx - 2 * dy;
        } else {
          result.emplace_back(x, y);
          p = p + 2 * dx;
        }
        ++y;
      }
    } else if (dx < 0 && -dx >= dy) {
      int p = 2 * dy + dx;
      while (x >= x1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          ++y;
          p = p + 2 * dy + 2 * dx;
        } else {
          result.emplace_back(x, y);
          p = p + 2 * dy;
        }
        --x;
      }
    } else if (dx < 0) {
      int p = -2 * dx - dy;
      while (y <= y1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          --x;
          p = p - 2 * dx - 2 * dy;
        } else {
          result.emplace_back(x, y);
          p = p - 2 * dx;
        }
        ++y;
      }
    }
  } else {
    if (dx >= 0 && (dx >= -dy)) {
      int p = -2 * dy - dx;
      while (x <= x1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          --y;
          p = p - 2 * dy - 2 * dx;
        } else {
          result.emplace_back(x, y);
          p = p - 2 * dy;
        }
        ++x;
      }
    } else if (dx >= 0) {
      int p = 2 * dx + dy;
      while (y >= y1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          ++x;
          p = p + 2 * dx + 2 * dy;
        } else {
          result.emplace_back(x, y);
          p = p + 2 * dx;
        }
        --y;
      }
    } else if (dx < 0 && -dx >= -dy) {
      int p = -2 * dy + dx;
      while (x >= x1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          --y;
          p = p - 2 * dy + 2 * dx;
        } else {
          result.emplace_back(x, y);
          p = p - 2 * dy;
        }
        --x;
      }
    } else if (dx < 0) {
      int p = -2 * dx + dy;
      while (y >= y1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          --x;
          p = p - 2 * dx + 2 * dy;
        } else {
          result.emplace_back(x, y);
          p = p - 2 * dx;
        }
        --y;
      }
    }
  }

//  result.emplace_back(x1, y1);

  return result;
}
