#ifndef ZSTACKSKELETONIZER_H
#define ZSTACKSKELETONIZER_H

#include <string>
#include <vector>
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

  /*!
   * \brief Configure the skeletonizer from a json file.
   */
  void configure(const std::string &filePath);

  /*!
   * \brief Configure the skeletonizer from a json object.
   *
   * See "neurolabi/json/skeletonize.schema.json" for the schema.
   */
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

  inline void setFillingHole(bool f) {
    m_fillingHole = f;
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

  inline void setLevelOp(int op) {
    m_grayOp = op;
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

  inline void getDownsampleInterval(int *xintv, int *yintv, int *zintv) {
    *xintv = m_downsampleInterval[0];
    *yintv = m_downsampleInterval[1];
    *zintv = m_downsampleInterval[2];
  }

  ZSwcTree* makeSkeleton(const Stack *stack);
  ZSwcTree* makeSkeleton(const ZStack &stack);
  ZSwcTree* makeSkeleton(const ZObject3dScan &obj);

  /*!
   * \brief Make a skeleton from an array of masks
   */
  ZSwcTree* makeSkeleton(const std::vector<ZStack *> &stackArray);

  void reconnect(ZSwcTree *tree);
  inline void setConnectingBranch(bool conn) {
    m_connectingBranch = conn;
  }

  void print() const;

  inline void useOriginalSignal(bool state) {
    m_usingOriginalSignal = state;
  }

private:
  /*!
   * \a stack will be destroyed after the function call.
   */
  ZSwcTree *makeSkeletonWithoutDs(Stack *stack);

  ZSwcTree *makeSkeletonWithoutDsTest(Stack *stack);


private:
  double m_lengthThreshold;
  double m_distanceThreshold;
  bool m_rebase;
  bool m_interpolating;
  bool m_removingBorder;
  bool m_fillingHole;
  int m_minObjSize;
  bool m_keepingSingleObject;
  int m_level;
  bool m_connectingBranch;
  double m_resolution[3];
  int m_downsampleInterval[3];
  bool m_usingOriginalSignal;
  bool m_resampleSwc;
  bool m_autoGrayThreshold;
  int m_grayOp;
};

#endif // ZSTACKSKELETONIZER_H
