#ifndef SWCPROCESSOR_H
#define SWCPROCESSOR_H

#include "c_stack.h"
#include "zresolution.h"

class ZSwcTree;

namespace Biocytin {
class SwcProcessor
{
public:
  SwcProcessor();

//  void removeZJump(ZSwcTree *tree);
  void breakZJump(ZSwcTree *tree);

  void setResolution(const ZResolution &resolution);
  void setMinDeltaZ(double dz) { m_minDeltaZ = dz; }
  void setMaxVLRatio(double r) { m_maxVLRatio = r; }

public:
  static void AssignZ(ZSwcTree *tree, const Stack &depthImage);
//  static void RemoveZJump(ZSwcTree *tree, double minDeltaZ);
  static void BreakZJump(ZSwcTree *tree, double minDeltaZ);
  static void RemoveOrphan(ZSwcTree *tree);
  static void SmoothZ(ZSwcTree *tree);
  static void SmoothRadius(ZSwcTree *tree);

private:
  void init();

private:
  ZResolution m_resolution;
  double m_minDeltaZ;
  double m_maxVLRatio;
};
}

#endif // SWCPROCESSOR_H
