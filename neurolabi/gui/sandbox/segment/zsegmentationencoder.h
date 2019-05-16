#ifndef ZSEGMENTATIONENCODER_H
#define ZSEGMENTATIONENCODER_H


#include <vector>
#include "zstack.hxx"
#include "geometry/zintcuboid.h"
#include "zobject3dscan.h"


using std::vector;


class ZSegmentationEncoder{
public:
  ZSegmentationEncoder(const ZIntPoint& offset = ZIntPoint(0,0,0));
  virtual ~ZSegmentationEncoder(){m_data.clear();}

public:
  void consume(const ZStack& stack);

  void addSegment(int z, int y, int start, int end);

  vector<int>& getSegment(int z, int y);
  const vector<int>& getSegment(int z, int y)const;

  void add(const ZIntPoint& point){add(point.getX(),point.getY(),point.getZ());}
  void add(int x,int y,int z){addSegment(z,y,x,x);}

  bool contains(const ZIntPoint& point)const{return contains(point.getX(),point.getY(),point.getZ());}
  bool contains(int x, int y, int z)const;

  void unify(const ZSegmentationEncoder& encoder);
  void unify(const ZSegmentationEncoder* pEncoder){if(pEncoder)unify(*pEncoder);}

  void labelStack(ZStack& stack, int value)const;
  void labelStack(ZStack* pStack, int value){if(pStack)labelStack(*pStack,value);}

  ZSegmentationEncoder* clone()const;

  double memUsage()const;

  ZIntCuboid getBoundBox()const{return ZIntCuboid(m_minx,m_miny,m_minz,m_maxx,m_maxy,m_maxz);}

private:
  inline void _maybe_update_bound_box(int z, int y, int start, int end);

protected:
  ZIntPoint m_offset;
  vector<vector<vector<int>>> m_data;
  int m_minx, m_maxx, m_miny, m_maxy, m_minz, m_maxz;
};


#endif // ZSEGMENTATIONENCODER_H
