/**@file zpoint.h
 * @brief 3D point class
 * @author Ting Zhao
 */
#ifndef ZPOINT_H
#define ZPOINT_H

#include <iostream>
#include <string>

#include "neutube_def.h"

class ZIntPoint;

class ZPoint {
public:
  ZPoint();
  ZPoint(double x, double y, double z);
  ZPoint(const double *pt);
  ZPoint(const ZPoint &pt);

public:
  inline void set(double x, double y, double z) {
    m_x = x;
    m_y = y;
    m_z = z;
  }
  inline void set(const ZPoint &pt) {
    set(pt.x(), pt.y(), pt.z());
  }
  inline double x() const { return m_x; }
  inline double y() const { return m_y; }
  inline double z() const { return m_z; }

  inline double* xRef() { return &m_x; }
  inline double* yRef() { return &m_y; }
  inline double* zRef() { return &m_z; }

  const double& operator[] (int index) const;
  double& operator[] (int index);
  ZPoint& operator= (const ZPoint &pt);
  bool operator== (const ZPoint &pt);

  bool operator < (const ZPoint &pt) const;

  inline void setX(double x) { m_x = x; }
  inline void setY(double y) { m_y = y; }
  inline void setZ(double z) { m_z = z; }

  double distanceTo(const ZPoint &pt) const;
  double distanceTo(double x, double y, double z) const;
  double length() const;

  ZPoint& operator += (const ZPoint &pt);
  ZPoint& operator += (const ZIntPoint &pt);
  ZPoint& operator -= (const ZPoint &pt);
  ZPoint& operator *= (const ZPoint &pt);
  ZPoint& operator /= (const ZPoint &pt);
  ZPoint& operator += (double offset);
  ZPoint& operator -= (double offset);
  ZPoint& operator *= (double scale);
  ZPoint& operator /= (double scale);
  ZPoint operator - () const;

  friend ZPoint operator + (const ZPoint &pt1, const ZPoint &pt2);
  friend ZPoint operator + (const ZPoint &pt1, const ZIntPoint &pt2);
  friend ZPoint operator - (const ZPoint &pt1, const ZPoint &pt2);
  friend ZPoint operator * (const ZPoint &pt1, double scale);
  friend ZPoint operator / (const ZPoint &pt1, double scale);

  void toArray(double *pt) const;

  void normalize();
  double dot(const ZPoint &pt) const;
  double cosAngle(const ZPoint &pt) const;
  ZPoint cross(const ZPoint &pt) const;

  bool isApproxOrigin() const;
  bool approxEquals(const ZPoint &pt) const;

  std::string toString() const;
  std::string toJsonString() const;

  void print() const;

  inline void translate(double dx, double dy, double dz) {
    m_x += dx;
    m_y += dy;
    m_z += dz;
  }
  void translate(const ZPoint &dp);

  void rotate(double theta, double psi);
  void rotate(double theta, double psi, const ZPoint &center);

  ZIntPoint toIntPoint() const;

public:
  //virtual void display(ZPainter &painter, int n = 0, Display_Style style = NORMAL) const;

  virtual void save(const char *filePath);
  virtual void load(const char *filePath);

  static inline double minimalDistance() { return m_minimalDistance; }

  struct ZCompare {
    bool operator() (const ZPoint &pt1, const ZPoint &pt2) {
      return (pt1.z() < pt2.z());
    }
  };

  struct YCompare {
    bool operator() (const ZPoint &pt1, const ZPoint &pt2) {
      return (pt1.y() < pt2.y());
    }
  };

  struct XCompare {
    bool operator() (const ZPoint &pt1, const ZPoint &pt2) {
      return (pt1.x() < pt2.x());
    }
  };

  void shiftSliceAxis(NeuTube::EAxis axis);
  void shiftSliceAxisInverse(NeuTube::EAxis axis);

  double getSliceCoord(NeuTube::EAxis axis) const;

private:
  double m_x;
  double m_y;
  double m_z;

  const static double m_minimalDistance;
};

#endif // ZPOINT_H
