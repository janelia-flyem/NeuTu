#ifndef ZINTCUBOIDFACE_H
#define ZINTCUBOIDFACE_H

#include "neutube.h"
#include <vector>
#include "tz_cuboid_i.h"
#include "zpoint.h"
#include "zintpoint.h"

class ZIntCuboidFaceArray;
class ZGraph;
class ZStack;

class ZIntCuboidFace
{
public:
  ZIntCuboidFace();
  ZIntCuboidFace(NeuTube::EAxis axis, bool positive) {
    m_normalAxis = axis;
    m_isNormalPositive = positive;
    m_z = 0;
  }

  class Corner {
  public:
    Corner() : m_x(0), m_y(0) {}
    Corner(int x, int y) : m_x(x), m_y(y) {}

  public:
    inline int getX() const { return m_x; }
    inline int getY() const { return m_y; }
    inline void set(int x, int y) {
      m_x = x;
      m_y = y;
    }

  private:
    int m_x;
    int m_y;
  };

  /*!
   * \brief Crop a face by another face.
   *
   *
   */
  ZIntCuboidFaceArray cropBy(const ZIntCuboidFace &face) const;
  ZIntCuboidFaceArray cropBy(const ZIntCuboidFaceArray &faceArray) const;

  const Corner& getFirstCorner() const { return m_firstCorner; }
  const Corner& getLastCorner() const { return m_lastCorner; }

  ZIntPoint getCornerCoordinates(int index) const;

  /*!
   * \brief Get a corner of the face.
   *
   * 0-----1
   * |     |
   * |     |
   * 2-----3
   *
   * \param index Corner index.
   */
  Corner getCorner(int index) const;


  /*!
   * \brief Get the bound value
   *
   * lower(0, 1) --- upper (0)
   * |
   * |
   * upper(1)--------upper(0, 1)
   */
  int getLowerBound(int index) const;
  int getUpperBound(int index) const;
  int getPlanePosition() const;

  void set(const Corner &firstCorner, const Corner &lastCorner);
  void set(int x0, int y0, int x1, int y1);
  void setNormal(NeuTube::EAxis axis);
  void setNormal(NeuTube::EAxis axis, bool isPositive);
  void setZ(int z);

  void flip();

  bool isValid() const;

  bool hasOverlap(const ZIntCuboidFace &face) const;

  bool contains(int x, int y, int z) const;
  bool contains(Corner pt) const;
  bool isWithin(const ZIntCuboidFace &face) const;

  inline NeuTube::EAxis getAxis() const { return m_normalAxis; }

  inline bool isNormalPositive() const { return m_isNormalPositive; }

  void print() const;

  void moveAxis(int dz);
  /*!
   * \brief Move the face backwards
   */
  void moveBackward(int dz);

  void moveForward(int dz);

  double computeDistance(double x, double y, double z);

private:
  Corner m_firstCorner;
  Corner m_lastCorner;
  int m_z;
  NeuTube::EAxis m_normalAxis;
  bool m_isNormalPositive;
};

class ZIntCuboidFaceArray : public std::vector<ZIntCuboidFace>
{
public:
  void appendValid(const ZIntCuboidFace &face);
  void append(const ZIntCuboidFace &face);
  void append(const ZIntCuboidFaceArray &faceArray);
  void append(const Cuboid_I *cuboid);

  ZIntCuboidFaceArray cropBy(const ZIntCuboidFace &face);
  ZIntCuboidFaceArray cropBy(const ZIntCuboidFaceArray &faceArray);

  void moveBackward(int dz);

  bool contains(int x, int y, int z) const;

  void print() const;

  void exportSwc(const std::string &filePath) const;

public: //Routines developed for checking face orphans
  ZGraph* getConnection() const;
  std::vector<ZStack*> toStack(int kind) const;

};

#endif // ZINTCUBOIDFACE_H
