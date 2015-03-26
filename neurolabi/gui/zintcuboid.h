#ifndef ZINTCUBOID_H
#define ZINTCUBOID_H

#include "zintpoint.h"

class ZIntCuboid
{
public:
  ZIntCuboid();
  ZIntCuboid(int x1, int y1, int z1, int x2, int y2, int z2);
  ZIntCuboid(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner);
  //ZIntCuboid(const ZIntCuboid &cuboid);

  inline const ZIntPoint& getFirstCorner() const { return m_firstCorner; }
  inline const ZIntPoint& getLastCorner() const { return m_lastCorner; }

  inline void setFirstCorner(const ZIntPoint &corner) {
    m_firstCorner = corner;
  }

  inline void setFirstCorner(int x, int y, int z) {
    m_firstCorner.set(x, y, z);
  }

  inline void setLastCorner(const ZIntPoint &corner) {
    m_lastCorner = corner;
  }

  inline void setLastCorner(int x, int y, int z) {
    m_lastCorner.set(x, y, z);
  }

  /*!
   * \brief Reset to the default constructing state.
   */
  void reset();

  void set(int x1, int y1, int z1, int x2, int y2, int z2);
  void set(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner);
  //void set(const int *corner);

  void setFirstX(int x);
  void setLastX(int x);
  void setFirstY(int y);
  void setLastY(int y);
  void setFirstZ(int z);
  void setLastZ(int z);

  void translateX(int dx);

  /*!
   * \brief Change the size of the cuboid by fixing the first corner
   */
  void setSize(int width, int height, int depth);

  int getWidth() const;
  int getHeight() const;
  int getDepth() const;

  /*!
   * \brief Set the depth
   *
   * It change the depth of the cuboid with the first corner fixed.
   *
   * \param depth New depth of the cuboid.
   */
  void setDepth(int depth);

  //union
  void join(const ZIntCuboid &cuboid);
  void joinX(int x);
  void joinY(int y);
  void joinZ(int z);

  void expandX(int dx);
  void expandY(int dy);

  /*!
   * \brief Get the volume of the cuboid.
   *
   * It returns 0 if the cuboid is invalid.
   */
  size_t getVolume() const;

  /*!
   * \brief Check if a point is in the cuboid.
   */
  bool contains(int x, int y, int z) const;

  /*!
   * \brief Check if a cuboid is empty.
   *
   * A cuboid is empty if any of its dimension <= 0.
   */
  bool isEmpty() const;

  bool equals(const ZIntCuboid &cuboid) const;

private:
  ZIntPoint m_firstCorner;
  ZIntPoint m_lastCorner;
};

#endif // ZINTCUBOID_H
