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
  ZStackMultiScaleWatershed(){}
  ~ZStackMultiScaleWatershed();
public:
#if defined(_QT_GUI_USED_)
  ZStack* run(ZStack *src,std::vector<ZObject3d*>& seeds,int scale,const QString &algorithm,const QString &dsMethod);
  ZStack* run(ZStack *src,std::vector<ZObject3d*>& seeds,const QString &algorithm,const QString &dsMethod);
#endif

public:
  void setScale(int scale){m_scale=scale;}
  ZStack* labelAreaNeedUpdate(ZStack* boundary_map,ZStack* seed,ZIntCuboid& boundbox,ZStack* src=NULL);
  void generateSeeds(ZStack* seed,const ZStack* boundary_map,const ZStack* stack);
  ZStack* getBoundaryMap(const ZStack& stack);
  ZStack* upSample(int width,int height,int depth,ZStack* sampled);
  ZStack* upSampleAndRecoverBoundary(ZStack* sampled_watershed,ZStack* src);
  void computeSeeds(ZStack* sampled_stack,std::vector<ZObject3d*>& seeds);
private:
  ZStack* toSeedStack(std::vector<ZObject3d*>& seeds,int width,int height,int depth,ZIntPoint offset);
  inline void addSeed(ZStack* pSeed,int sx,int ex,int sy,int ey,int sz,int ez,uint8_t v);
  bool checkNeighbors(const uint8_t* pboundary, const uint8_t* pstack,int x,int y, int z,int width,int height,int depth);
private:
  int m_scale = 1;
};

#endif // ZSTACKMULTISCALEWATERSHED_H
