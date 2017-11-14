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

class ZStackMultiScaleWatershed
{
public:
  ZStackMultiScaleWatershed();
  ~ZStackMultiScaleWatershed();
public:
#if defined(_QT_GUI_USED_)
  ZStack* run(ZStack *src,std::vector<ZObject3d*>& seeds,int scale);
  ZStack* run(ZStack *src,std::vector<ZObject3d*>& seeds);
#endif

public:
  void setScale(int scale){m_scale=scale;}
  ZStack* labelAreaNeedUpdate(ZStack* edge_map,ZStack* seed,ZStack* src=NULL);
  void generateSeeds(ZStack* seed,int width,int height,int depth,const ZStack* edge_map,const ZStack* stack);
  ZStack* getEdgeMap(const ZStack& stack);
  ZStack* upSample(int width,int height,int depth,ZStack* sampled);
  ZStack* upSampleAndRecoverEdge(ZStack* sampled_watershed,ZStack* src);

private:
  ZStack* toSeedStack(std::vector<ZObject3d*>& seeds,int width,int height,int depth,ZIntPoint offset);
private:
  int m_scale;
};

#endif // ZSTACKMULTISCALEWATERSHED_H
