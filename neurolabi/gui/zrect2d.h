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
  virtual ~ZRect2d() override;

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::RECT2D;
  }

  void set(int x0, int y0, int width, int height);

  inline int getX0() const { return m_x0; }
  inline int getY0() const { return m_y0; }
  inline int getWidth() const { return m_width; }
  inline int getHeight() const { return m_height; }

  bool hit(double x, double y, neutu::EAxis axis) override;
  bool hit(double x, double y, double z) override;

  bool contains(double x, double y) const;

public:
  virtual void display(ZPainter &painter, int slice, EDisplayStyle option,
                       neutu::EAxis sliceAxis) const override;
  bool display(QPainter *rawPainter, int z, EDisplayStyle option,
               EDisplaySliceMode sliceMode, neutu::EAxis sliceAxis) const override;

//  virtual const std::string& className() const;
  bool isSliceVisible(int z, neutu::EAxis sliceAxis) const override;
  inline void setPenetrating(bool p) {
    m_isPenetrating = p;
  }
  void setZSpan(int zSpan) {
    m_zSpan = zSpan;
  }
  int getZSpan() const {
    return m_zSpan;
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
  void setMaxCorner(int x, int y);
  void setMinCorner(int x, int y);

  /*!
    * \brief Set size by fixing the first corner.
    */
  void setSize(int width, int height);

  int getMinX() const;
  int getMinY() const;
  int getMaxX() const;
  int getMaxY() const;

  inline void setZ(int z) { m_z = z; }
  inline int getZ() const { return m_z; }

  //QRect/QRectF utilities
  static bool IsEqual(const QRect &rect1, const QRect &rect2);
  static bool IsEqual(const QRectF &rect1, const QRectF &rect2);
  static QRect QRectBound(const QRectF &rect);

  static QRectF CropRect(
      const QRectF &sourceRectIn, const QRectF &sourceRectOut,
      const QRectF &targetRectIn);

  ZCuboid getBoundBox() const override;

private:
  void init(int x0, int y0, int width, int height);
  void preparePen(QPen &pen) const;

private:
  int m_x0;
  int m_y0;
  int m_width;
  int m_height;
  int m_z;
  int m_zSpan;
  bool m_isPenetrating;
//  NeuTube::EAxis m_sliceAxis;
};

#endif // ZRECT2D_H
