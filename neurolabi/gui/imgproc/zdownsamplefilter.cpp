#include <cstdlib>
#include <cmath>

#include "zdownsamplefilter.h"
#include "zstack.hxx"
#include "zsparsestack.h"
#include "geometry/zintcuboid.h"
#include "geometry/zintpoint.h"

ZDownsampleFilter::ZDownsampleFilter()
{
  m_dsX=m_dsY=m_dsZ=1;
}

 ZDownsampleFilter* ZDownsampleFilter::create(std::string type){
  if (type=="Min"){
    return new ZMinDsFilter();
  }
  if (type=="Min(ignore zero)"){
    return new ZMinIgnoreZeroDsFilter();
  }
  if (type=="Max"){
    return new ZMaxDsFilter();
  }
  if (type=="Mean"){
    return new ZMeanDsFilter();
  }
  if (type=="Edge"){
    return new ZEdgeDsFilter();
  }
  return nullptr;
}

ZStack* ZDownsampleFilter::filterStack(const ZSparseStack& spStack)
{

  ZIntCuboid box=spStack.getBoundBox();
  int width=box.getWidth(),height=box.getHeight(),depth=box.getDepth();

  int x=(width+m_dsX-1)/m_dsX;
  int y=(height+m_dsY-1)/m_dsY;
  int z=(depth+m_dsZ-1)/m_dsZ;

  ZStack* rv=new ZStack(GREY,x,y,z,1);
  ZIntPoint offset=box.getFirstCorner();
  rv->setOffset(offset.m_x/m_dsX,offset.m_y/m_dsY,offset.m_z/m_dsZ);
  rv->setDsIntv(m_dsX-1,m_dsY-1,m_dsZ-1);

  int ofx=offset.m_x,ofy=offset.m_y,ofz=offset.m_z;
  uint8_t* pDst=rv->array8();
  uint8_t *pCur=pDst;

  int size_buffer=m_dsX*m_dsY*m_dsZ;
  double* buffer=new double[size_buffer];

  for (int k=0;k<z;++k)
  {
    int startZ=k*m_dsZ,endZ=std::min(depth,(k+1)*m_dsZ);
    m_sZ=endZ-startZ;
    for(int j=0;j<y;++j)
    {
      int startY=j*m_dsY,endY=std::min(height,(j+1)*m_dsY);
      m_sY=endY-startY;
      for(int i=0;i<x;++i)
      {
        int startX=i*m_dsX,endX=std::min(width,(i+1)*m_dsX);
        m_sX=endX-startX;
        int index=0;
        for (int kk=startZ;kk<endZ;++kk)
        {
          for(int jj=startY;jj<endY;++jj)
          {
            spStack.getLineValue(startX+ofx,jj+ofy,kk+ofz,m_sX,buffer+index);
            index+=m_sX;
          }
        }
        *pCur++=(uint8)filter(buffer);
      }
    }
  }
  delete []buffer;
  return rv;
}

ZStack* ZDownsampleFilter::filterStack(const ZStack& stack)
{
  int width=stack.width(),height=stack.height(),depth=stack.depth();

  int x=(width+m_dsX-1)/m_dsX;
  int y=(height+m_dsY-1)/m_dsY;
  int z=(depth+m_dsZ-1)/m_dsZ;
  ZStack* rv=new ZStack(GREY,x,y,z,1);
  rv->setOffset(stack.getOffset().m_x/m_dsX,
                stack.getOffset().m_y/m_dsY,
                stack.getOffset().m_z/m_dsZ);
  rv->setDsIntv(m_dsX-1,m_dsY-1,m_dsZ-1);

  const uint8_t* pSrc=stack.array8();
  uint8_t* pDst=rv->array8();
  uint8_t *pCur=pDst;

  int size_buffer=m_dsX*m_dsY*m_dsZ;
  double* buffer=new double[size_buffer];

  for (int k=0;k<z;++k)
  {
    int startZ=k*m_dsZ,endZ=std::min(depth,(k+1)*m_dsZ);
    m_sZ=endZ-startZ;
    for(int j=0;j<y;++j)
    {
      int startY=j*m_dsY,endY=std::min(height,(j+1)*m_dsY);
      m_sY=endY-startY;
      for(int i=0;i<x;++i)
      {
        int startX=i*m_dsX,endX=std::min(width,(i+1)*m_dsX);
        m_sX=endX-startX;
        memset(buffer,0,size_buffer*sizeof(double));

        int index=0;
        for (int kk=startZ;kk<endZ;++kk)
        {
          for(int jj=startY;jj<endY;++jj)
          {
            size_t offset=kk*width*height+jj*width+startX;
            for(int ii=startX;ii<endX;++ii)
            {
              buffer[index++]=(double)pSrc[offset++];
            }
          }
        }
        *pCur++=(uint8)filter(buffer);
      }
    }
  }
  delete []buffer;
  return rv;
}


double ZMinDsFilter::filter(double *buffer)
{
  int size=m_sX*m_sY*m_sZ;
  double min=255;
  for(int i=0;i<size;++i)
  {
    if(buffer[i]<min){
      min=buffer[i];
    }
  }
  return min;
}


double ZMinIgnoreZeroDsFilter::filter(double *buffer)
{
  int size=m_sX*m_sY*m_sZ;
  double min=255;
  bool all_zero=true;
  for(int i=0;i<size;++i)
  {
    if(buffer[i]<min && buffer[i]!=0){
      min=buffer[i];
      all_zero=false;
    }
  }
  if(all_zero){
    return 0;
  }
  else{
    return min;
  }
}


double ZMaxDsFilter::filter(double *buffer)
{
  int size=m_sX*m_sY*m_sZ;
  double max=0;
  for(int i=0;i<size;++i)
  {
    if(buffer[i]>max){
      max=buffer[i];
    }
  }
  return max;
}


double ZMeanDsFilter::filter(double *buffer)
{
  int size=m_sX*m_sY*m_sZ;
  double mean=0;
  for(int i=0;i<size;++i)
  {
    mean+=buffer[i];
  }
  return mean/size;
}

double ZEdgeDsFilter::filter(double *buffer)
{
  int center_x=m_sX/2,center_y=m_sY/2,center_z=m_sZ/2;
  if(buffer[center_z*m_sX*m_sY+center_y*m_sX+center_x]==0){
    return 0.0;
  }

  if(m_sX==1 || m_sY==1 || m_sZ==1){
    return 0.0;
  }
  double sigx=(m_sX-1)/6.0,sigy=(m_sY-1)/6.0,sigz=(m_sZ-1)/6.0;
  double sigx2=sigx*sigx,sigy2=sigy*sigy,sigz2=sigz*sigz;

  double weight=0.0;
  double rv=0.0;
  int off=0;

  for(int k=0;k<m_sZ;++k){
    double z=k-center_z;
    for(int j=0;j<m_sY;++j){
      double y=j-center_y;
      for(int i=0;i<m_sX;++i){
        double x=i-center_x;
        double factor=(1-x*x/(3*sigx2)-y*y/(3*sigy2)-z*z/(3*sigz2));
        factor*=std::exp(-0.5*((x*x)/sigx2+(y*y)/sigy2+(z*z)/sigz2));
        weight+=std::abs(factor);
        rv+=factor*buffer[off++];
      }
    }
  }
  return std::abs(rv/weight);
}
