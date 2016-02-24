#ifndef ZRECT2D_H
#define ZRECT2D_H

#include "zstackobject.h"

class QRect;
class QRectF;

class ZRect2d : public ZStackObject
{
public:
  ZRect2d();
  ZRect2d(int x0, int y0, int width, int height);
  virtual ~ZRect2d();

  void set(int x0, int y0, int width, int height);

  inline int getX0() const { return m_x0; }
  inline int getY0() const { return m_y0; }
  inline int getWidth() const { return m_width; }
  inline int getHeight() const { return m_height; }

  bool hit(double x, double y, NeuTube::EAxis axis);
  bool hit(double x, double y, double z);

  bool contains(double x, double y) const;

public:
  virtual void display(ZPainter &painter, int slice, EDisplayStyle option,
                       NeuTube::EAxis sliceAxis) const;
  bool display(QPainter *rawPainter, int z, EDisplayStyle option,
               EDisplaySliceMode sliceMode, NeuTube::EAxis sliceAxis) const;

  virtual const std::string& className() const;
  bool isSliceVisible(int z, NeuTube::EAxis sliceAxis) const;
  inline void setPenetrating(bool p) {
    m_isPenetrating = p;
  }

  bool isValid() const;

  /*!
   * \brief Try the make rectangle a valid one.
   *
   * It tries to convert a negaive dimension into a positive one by rearranging
   * the corners.
   *
   * \return true iff if the rectangle is valid after the function call.
   */
  bool makeValid();

  /*!
   * \brief Set the last corner of the rectable
   */
  void setLastCorner(int x, int y);
  void setFirstCorner(int x, int y);

  /*!
    * \brief Set size by fixing the first corner.
    */
  void setSize(int width, int height);

  int getFirstX() const;
  int getFirstY() const;
  int getLastX() const;
  int getLastY() const;

  inline void setZ(int z) { m_z = z; }
  inline int getZ() const { return m_z; }

  //QRect/QRectF utilities
  static bool IsEqual(const QRect &rect1, const QRect &rect2);
  static bool IsEqual(const QRectF &rect1, const QRectF &rect2);
  static QRect QRectBound(const QRectF &rect);

  static QRectF CropRect(
      const QRectF &sourceRectIn, const QRectF &sourceRectOut,
      const QRectF &targetRectIn);

private:
  int m_x0;
  int m_y0;
  int m_width;
  int m_height;
  int m_z;
  bool m_isPenetrating;
//  NeuTube::EAxis m_sliceAxis;
};

#endif // ZRECT2D_H
