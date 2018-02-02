#ifndef ZOBJECT3DSCANARRAY_H
#define ZOBJECT3DSCANARRAY_H

#include <vector>
#include <string>
#include "zobject3dscan.h"

class ZIntCuboid;
class ZStack;

class ZObject3dScanArray : public std::vector<ZObject3dScan*>
{
public:
  ZObject3dScanArray();
  ~ZObject3dScanArray();

  void clearAll();
  void shallowClear();

  void append(ZObject3dScan *obj);
  void append(const ZObject3dScan &obj);

  ZIntCuboid getBoundBox() const;
  size_t getVoxelNumber() const;

  void importDir(const std::string &dirPath);
  void downsample(int xintv, int yintv, int zintv);
  void upsample(int xIntv, int yIntv, int zIntv);
  void translate(int dx, int dy, int dz);

  ZStack* toStackObject() const;
  ZStack* toLabelField() const;
  ZStack* toLabelField(const ZIntCuboid &box) const;
  ZStack* toColorField() const;
};

#endif // ZOBJECT3DSCANARRAY_H
