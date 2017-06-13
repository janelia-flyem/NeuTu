#ifndef ZSTACKMULTISCALEWATERSHED_H
#define ZSTACKMULTISCALEWATERSHED_H

#if defined(_QT_GUI_USED_)
#include <QList>
#endif

#include <vector>


class ZStack;
class ZSwcTree;
class ZIntPoint;

class ZStackMultiScaleWatershed
{
public:
  ZStackMultiScaleWatershed();
  ~ZStackMultiScaleWatershed();
public:
#if defined(_QT_GUI_USED_)
  ZStack* run(ZStack *src,QList<ZSwcTree*>& trees,int scale);
#endif

  void test();
public:
  void setScale(int scale){_scale=scale;}
  ZStack* labelAreaNeedUpdate(ZStack* edge_map,ZStack* seed,ZStack* src=nullptr);
  void generateSeeds(ZStack* seed,int width,int height,int depth,const ZStack* edge_map,const ZStack* stack);
  ZStack* getEdgeMap(const ZStack& stack);
  ZStack* upSample(int width,int height,int depth,ZStack* sampled);
#if defined(_QT_GUI_USED_)
  void fillSeed(ZStack* seed,QList<ZSwcTree*>& trees,const ZIntPoint& offset);
#endif

  ZStack* upSampleAndRecoverEdge(ZStack* sampled_watershed,ZStack* src);

private:
  double _scale;
};

#endif // ZSTACKMULTISCALEWATERSHED_H
