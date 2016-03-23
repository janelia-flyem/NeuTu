/**@file zcircle.h
 * @brief Circle class
 * @author Ting Zhao
 */
#ifndef ZCIRCLE_H
#define ZCIRCLE_H

#include "zqtheader.h"

#include "include/tz_stdint.h"
#include "zpoint.h"
#include "zstackobject.h"

class ZIntPoint;

/*!
 * \brief The class of a ball (x, y, z, r)
 *
 * The class name ZCircle is a misnomer. Use ZStackBall instead.
 */
class ZCircle : public ZStackObject {
public:
  ZCircle();
  ZCircle(double x, double y, double z, double r);
  virtual ~ZCircle() {}

  void set(double x, double y, double z, double r);
  void set(const ZPoint &center, double r);
  void set(const ZIntPoint &center, double r);

  virtual const std::string& className() const;

  typedef uint32_t TVisualEffect;

  const static TVisualEffect VE_NONE;
  const static TVisualEffect VE_DASH_PATTERN;
  const static TVisualEffect VE_BOUND_BOX;
  const static TVisualEffect VE_NO_CIRCLE;
  const static TVisualEffect VE_NO_FILL;
  const static TVisualEffect VE_GRADIENT_FILL;
  const static TVisualEffect VE_OUT_FOCUS_DIM;

public:
  using ZStackObject::display; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  virtual void display(ZPainter &painter, int z,
                       EDisplayStyle option, NeuTube::EAxis sliceAxis) const;

  virtual void save(const char *filePath);
  virtual bool load(const char *filePath);

  void displayHelper(ZPainter *painter, int n, EDisplayStyle style) const;

  /*!
   * \brief Test if a circle is cut by a plane.
   */
  static bool isCuttingPlane(double z, double r, double n, double zScale = 1.0);
  bool isCuttingPlane(double n, double zScale = 1.0);

  using ZStackObject::setVisualEffect; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  inline void setVisualEffect(TVisualEffect effect) {
    m_visualEffect = effect;
  }

  inline bool hasVisualEffect(TVisualEffect effect) const {
    return effect & m_visualEffect;
  }

private:
  double getAdjustedRadius(double r) const;
  void _init(double x, double y, double z, double r);

private:
  ZPoint m_center;
  double m_r;
  TVisualEffect m_visualEffect;
};

#endif // ZCIRCLE_H
