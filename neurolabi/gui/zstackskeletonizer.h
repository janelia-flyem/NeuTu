#ifndef ZSTACKSKELETONIZER_H
#define ZSTACKSKELETONIZER_H

#include "tz_image_lib_defs.h"
#include "zprogressable.h"

class ZSwcTree;
class ZStack;
class ZObject3dScan;
class ZJsonObject;

class ZStackSkeletonizer : public ZProgressable
{
public:
  ZStackSkeletonizer();

  void configure(const ZJsonObject &config);

  inline void setLengthThreshold(double threshold) {
    m_lengthThreshold = threshold;
  }

  inline void setDistanceThreshold(double threshold) {
    m_distanceThreshold = threshold;
  }

  inline void setRebase(bool rebase) {
    m_rebase = rebase;
  }

  inline void setInterpolating(bool inter) {
    m_interpolating = inter;
  }

  inline void setRemovingBorder(bool r) {
    m_removingBorder = r;
  }

  inline void setMinObjSize(int s) {
    m_minObjSize = s;
  }

  inline void setKeepingSingleObject(bool k) {
    m_keepingSingleObject = k;
  }

  inline void setLevel(int level) {
    m_level = level;
  }

  inline void setResolution(double xyRes, double zRes) {
    m_resolution[0] = xyRes;
    m_resolution[1] = xyRes;
    m_resolution[2] = zRes;
  }

  inline void setDownsampleInterval(int xintv, int yintv, int zintv) {
    m_downsampleInterval[0] = xintv;
    m_downsampleInterval[1] = yintv;
    m_downsampleInterval[2] = zintv;
  }

  ZSwcTree* makeSkeleton(const Stack *stack);
  ZSwcTree* makeSkeleton(const ZStack &stack);
  ZSwcTree* makeSkeleton(const ZObject3dScan &obj);

  void reconnect(ZSwcTree *tree);
  inline void setConnectingBranch(bool conn) {
    m_connectingBranch = conn;
  }

  void print() const;

private:
  /*!
   * \a stack will be destroyed after the function call.
   */
  ZSwcTree *makeSkeletonWithoutDs(Stack *stack);

private:
  double m_lengthThreshold;
  double m_distanceThreshold;
  bool m_rebase;
  bool m_interpolating;
  bool m_removingBorder;
  int m_minObjSize;
  bool m_keepingSingleObject;
  int m_level;
  bool m_connectingBranch;
  double m_resolution[3];
  int m_downsampleInterval[3];
};

#endif // ZSTACKSKELETONIZER_H
