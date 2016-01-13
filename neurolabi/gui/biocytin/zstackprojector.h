#ifndef ZSTACKPROJECTOR_H
#define ZSTACKPROJECTOR_H

#include <vector>
#include <string>

#include "zprogressable.h"
#include "tz_fmatrix.h"
#include "tz_image_lib_defs.h"
#include "neutube_def.h"

class ZStack;

namespace Biocytin {
class ZStackProjector : public ZProgressable {
public:
  ZStackProjector();

  ZStack* project(const ZStack *stack, NeuTube::EImageBackground bg,
                  bool includingDepth, int slabIndex);
  ZStack* project(
      const ZStack *stack, NeuTube::EImageBackground bg);

  inline void setAdjustingContrast(bool adj) {
    m_adjustingConstrast = adj;
  }

  inline void setSmoothingDepth(bool stat) {
    m_smoothingDepth = stat;
  }

  inline void setSpeedLevel(int level) {
    m_speedLevel = level;
  }

  inline void setUsingExisted(bool stat) {
    m_usingExisted = stat;
  }

  inline void setSlabNumber(int nslab) {
    m_slabCount = nslab;
  }

  inline int getSlabNumber() const {
    return m_slabCount;
  }

  inline const std::vector<int>& getDepthArray() { return m_depthArray; }

  static std::string GetDefaultResultFilePath(
      const std::string &basePath, int minZ, int maxZ);
  static std::string GetDefaultResultFilePath(
      const std::string &basePath, int slabCount);
private:
  inline double colorToValue(double g, double sr, double sg, double reg,
                             double redScale, double redOffset,
                             double greenScale, double greenOffset) {
    sr = sr * redScale + redOffset;
    sg = sg * greenScale + greenOffset;
    double ratio = (sg + reg) / (sr + reg);
    return g * ratio * ratio;
  }

  inline double colorToValue(double g, double sr, double sg, double reg) {
    double ratio = (sg + reg) / (sr + reg);
    return g * ratio * ratio;
  }

  double colorToValueH(double sr, double sg, double sb, double reg);

  /*!
   * \brief Get the Z range of a slab
   * \param slabIndex Starting from 0.
   * \return <min, max>
   */
  std::pair<int, int> getSlabRange(int depth, int slabIndex);

private:
  FMatrix* smoothStackNull(Stack *stack);
  FMatrix* smoothStack(Stack *stack);
  FMatrix* smoothStackGaussian(Stack *stack);

private:
  bool m_adjustingConstrast;
  bool m_smoothingDepth;
  int m_speedLevel;
  bool m_usingExisted;
  int m_slabCount;
  std::vector<int> m_depthArray;
};
}

#endif // ZSTACKPROJECTOR_H
