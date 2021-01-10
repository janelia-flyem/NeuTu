#ifndef ZSTACKBALL_H
#define ZSTACKBALL_H

#include "zqtheader.h"

#include "zstackobject.h"
#include "geometry/zpoint.h"
#include "common/neutudefs.h"

class ZIntPoint;

/*!
 * \brief The class of a ball (x, y, z, r)
 */
class ZStackBall : public ZStackObject {
public:
  ZStackBall();
  ZStackBall(double x, double y, double z, double r);
  ZStackBall(const ZIntPoint &center, double r);
  ZStackBall(const ZPoint &center, double r);
  virtual ~ZStackBall() {}

  void set(double x, double y, double z, double r);
  void set(const ZPoint &center, double r);
  void set(const ZIntPoint &center, double r);

  void setCenter(double x, double y, double z);
  void setCenter(const ZPoint &center);
  void setCenter(const ZIntPoint &center);

  inline void setX(double x) { m_center.setX(x); }
  inline void setY(double y) { m_center.setY(y); }
  inline void setZ(double z) { m_center.setZ(z); }
  inline void setRadius(double r) { m_r = r; }

  inline double getX() const { return m_center.x(); }
  inline double getY() const { return m_center.y(); }
  inline double getZ() const { return m_center.z(); }
  inline double getRadius() const { return m_r; }
  inline const ZPoint& getCenter() const { return m_center; }

  inline double x() const { return getX(); }
  inline double y() const { return getY(); }
  inline double z() const { return getZ(); }
  inline double radius() const { return getRadius(); }

//  virtual const std::string& className() const;

public:
  ZCuboid getBoundBox() const override;

  void display(ZPainter &painter, int slice,
               EDisplayStyle option, neutu::EAxis sliceAxis) const override;
  bool display(QPainter *painter, int z, EDisplayStyle option,
                       EDisplaySliceMode sliceMode, neutu::EAxis sliceAxis) const override;
  void display(ZPainter &painter, const DisplayConfig &config) const override;

  virtual void save(const char *filePath);
  virtual bool load(const char *filePath);

  void displayHelper(
      ZPainter *painter, int slice, EDisplayStyle style,
      neutu::EAxis sliceAxis) const;

  bool isSliceVisible(int z, neutu::EAxis sliceAxis) const override;
  bool isSliceVisible(
        int z, neutu::EAxis axis, const ZAffinePlane &plane) const override;

  /*!
   * \brief Test if a circle is cut by a plane.
   */
  static bool isCuttingPlane(double z, double r, double n, double zScale);
  bool isCuttingPlane(double n, double zScale, neutu::EAxis sliceAxis) const;


  void translate(double dx, double dy, double dz);
  void translate(const ZPoint &offset);
  void scaleCenter(double sx, double sy, double sz);
  void scale(double sx, double sy, double sz);

  bool hit(double x, double y, double z) override;
  bool hit(double x, double y, neutu::EAxis axis) override;

private:
  double getAdjustedRadius(double r) const;
  void init(double x, double y, double z, double r);

private:
  ZPoint m_center;
  double m_r;
//  TVisualEffect m_visualEffect;
};
#endif // ZSTACKBALL_H
