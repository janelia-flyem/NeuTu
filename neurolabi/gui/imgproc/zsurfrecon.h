#ifndef ZSURFRECON_H
#define ZSURFRECON_H

#undef ASCII
#undef BOOL
#undef TRUE
#undef FALSE

//#include <surfrecon.h>
#include "imgproc/surfrecon.h"

class ZStack;
class ZSwcTree;
class ZSurfRecon
{
public:
  ZSurfRecon();
  ~ZSurfRecon();
public:
  static void SurfRecon(VoxelSet& in,VoxelSet& out);
  static void LabelStack(VoxelSet& surface,ZStack* stack,int v=0);
  static void GaussianBlur(VoxelSet& surface,ZStack* stack,int r,double sigma=1.2);
  static void PruneSkeleton(VoxelSet& surface,ZSwcTree* tree);
private:

};

#endif // ZSURFRECON_H

