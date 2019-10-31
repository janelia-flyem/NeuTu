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
  //if label !=NULL, super voxels are constructed according to it; it should float type.
  shared_ptr<ZStack> run(const ZStack& stack, const vector<ZStackObject*>& seeds, shared_ptr<ZStack> label = nullptr);

  void run(vector<int>& result_mapping, const ZStack& stack, const vector<ZStackObject*>& seeds, shared_ptr<ZStack> label = nullptr);

private:
  inline double _weight(double a, double b)const;
};

#endif // ZMSTCONTAINER_H
