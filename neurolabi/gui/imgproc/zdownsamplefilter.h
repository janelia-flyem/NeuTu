#ifndef ZDOWNSAMPLEFILTER_H
#define ZDOWNSAMPLEFILTER_H

#include <string>

class ZStack;
class ZSparseStack;
class ZIntPoint;

class ZDownsampleFilter
{
public:
  ZDownsampleFilter();
  virtual ~ZDownsampleFilter(){}
public:
  virtual ZStack* filterStack(const ZStack& stack);
  virtual ZStack* filterStack(const ZSparseStack& spStack);
  static ZDownsampleFilter* create(std::string type);
  void setDsFactor(int dsX,int dsY,int dsZ){
    m_dsX=dsX;
    m_dsY=dsY;
    m_dsZ=dsZ;
  }
protected:
  virtual double filter(double* buffer)=0;
private:
  int m_dsX;
  int m_dsY;
  int m_dsZ;
protected:
  int m_sX;
  int m_sY;
  int m_sZ;
};

class ZMinDsFilter:public ZDownsampleFilter
{
protected:
  double filter(double* buffer);
};

class ZMinIgnoreZeroDsFilter:public ZDownsampleFilter
{
protected:
  double filter(double* buffer);
};

class ZMaxDsFilter:public ZDownsampleFilter
{
protected:
  double filter(double* buffer);
};

class ZMeanDsFilter:public ZDownsampleFilter
{
protected:
  double filter(double* buffer);
};

class ZEdgeDsFilter:public ZDownsampleFilter
{
protected:
  double filter(double* buffer);
};


#endif // ZDOWNSAMPLEFILTER_H
