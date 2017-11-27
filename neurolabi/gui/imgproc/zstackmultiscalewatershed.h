#ifndef ZSTACKMULTISCALEWATERSHED_H
#define ZSTACKMULTISCALEWATERSHED_H

#if defined(_QT_GUI_USED_)
#include <QList>
#endif

#include <vector>


class ZStack;
class ZSwcTree;
class ZIntPoint;
class ZObject3d;
class ZIntCuboid;
class ZStackMultiScaleWatershed
{
public:
  ZStackMultiScaleWatershed();
  ~ZStackMultiScaleWatershed();
public:
#if defined(_QT_GUI_USED_)
  ZStack* run(ZStack *src,std::vector<ZObject3d*>& seeds,int scale,const QString &algorithm);
  ZStack* run(ZStack *src,std::vector<ZObject3d*>& seeds,const QString &algorithm);
#endif

public:
  void setScale(int scale){m_scale=scale;}
  ZStack* labelAreaNeedUpdate(ZStack* boundary_map,ZStack* seed,ZIntCuboid& boundbox,ZStack* src=NULL);
  void generateSeeds(ZStack* seed,int width,int height,int depth,const ZStack* boundary_map,const ZStack* stack);
  ZStack* getBoundaryMap(const ZStack& stack);
  ZStack* upSample(int width,int height,int depth,ZStack* sampled);
  ZStack* upSampleAndRecoverBoundary(ZStack* sampled_watershed,ZStack* src);

private:
  ZStack* toSeedStack(std::vector<ZObject3d*>& seeds,int width,int height,int depth,ZIntPoint offset);
  inline void addSeedX(ZStack* pSeed,int x,int sy,int ey,int sz,int ez,uint8_t v);
  inline void addSeedY(ZStack* pSeed,int y,int sx,int ex,int sz,int ez,uint8_t v);
  inline void addSeedZ(ZStack* pSeed,int z,int sx,int ex,int sy,int ez,uint8_t v);
private:
  int m_scale;
};

#endif // ZSTACKMULTISCALEWATERSHED_H
