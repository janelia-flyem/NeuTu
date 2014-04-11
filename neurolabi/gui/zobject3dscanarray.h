#ifndef ZOBJECT3DSCANARRAY_H
#define ZOBJECT3DSCANARRAY_H

#include <vector>
#include <string>
#include "zobject3dscan.h"

class ZObject3dScanArray : public std::vector<ZObject3dScan>
{
public:
  ZObject3dScanArray();

  void importDir(const std::string &dirPath);
};

#endif // ZOBJECT3DSCANARRAY_H
