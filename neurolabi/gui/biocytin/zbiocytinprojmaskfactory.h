#ifndef ZBIOCYTINPROJMASKFACTORY_H
#define ZBIOCYTINPROJMASKFACTORY_H

#include "common/zsharedpointer.h"
#include "common/neutudefs.h"

class ZStack;
class ZObject3dScan;
class ZStackView;

class ZBiocytinProjMaskFactory
{
public:
  ZBiocytinProjMaskFactory();

public:
  static ZStack* MakeMask(ZStackView *view, int label);
};

#endif // ZBIOCYTINPROJMASKFACTORY_H
