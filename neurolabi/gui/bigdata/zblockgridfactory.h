#ifndef ZBLOCKGRIDFACTORY_H
#define ZBLOCKGRIDFACTORY_H

class ZObject3dScan;
class ZStackBlockGrid;

class ZBlockGridFactory
{
public:
  ZBlockGridFactory();

  ZStackBlockGrid* createStackBlockGrid(const ZObject3dScan &obj);
};

#endif // ZBLOCKGRIDFACTORY_H
