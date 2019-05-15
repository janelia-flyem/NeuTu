#ifndef ZMSTCONTAINER_H
#define ZMSTCONTAINER_H

#include<map>
#include<memory>
#include"zsegmentationencoder.h"
#include"imgproc/zwatershedmst.h"

using std::map;
using std::shared_ptr;


class ZMSTContainer
{
public:
  ZMSTContainer(){}
  ~ZMSTContainer(){}

public:
  shared_ptr<ZStack> run(const ZStack& stack, const vector<ZStackObject*>& seeds);

private:
  shared_ptr<ZStack> _createSuperVoxel(const ZStack& stack, uint& start_label, int sx, int sy, int sz)const;

  std::pair<bool, shared_ptr<ZStack>> _seedsFromLocalMaximum(const ZStack& stack, int sx, int sy, int sz)const;

  inline double _weight(double a, double b)const;
  /*void consume(const ZStack* seg, const ZStack* img);

  const vector<int>& vertices()const {return m_vertices;}

  const vector<ZEdge>& edges()const {return m_edges;}


  const map<int,shared_ptr<ZSegmentationEncoder>>& data()const {return m_data;}

  const shared_ptr<ZSegmentationEncoder>& getSuperVoxel(int label)const;*/

private:
  /*template<typename T>
  void _consume(const T* array, const ZStack& seg, const ZStack& img);
  inline void _addSegment(int v, int z, int y, int x1, int x2, const ZIntPoint& offset);*/

};

#endif // ZMSTCONTAINER_H
