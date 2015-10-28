#ifndef ZBIOCYTINPROJMASKFACTORY_H
#define ZBIOCYTINPROJMASKFACTORY_H

#include "zsharedpointer.h"
#include "neutube_def.h"

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
