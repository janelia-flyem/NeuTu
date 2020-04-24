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
  ZIntPoint offset=box.getMinCorner();
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
  /*
  double sig=2.0;
  double sig2 = sig * sig;
  double center_x=m_sX/2.0,center_y=m_sY/2.0,center_z=m_sZ/2.0;

  size_t offset = 0;
  double weight = 0.0;
  double factor[20*20*20];*/

  /*
  for (k = 0; k < filter->dim[2]; k++) {
    for (j = 0; j < filter->dim[1]; j++) {
      for (i = 0; i < filter->dim[0]; i++) {
  coord[0] = ((double) i) - wndsize;
  coord[1] = ((double) j) - wndsize;
  coord[2] = ((double) k) - wndsize;
  r = coord[0] * coord[0] + coord[1] * coord[1] +
     coord[2] * coord[2];
  r = r / sigma2;
  filter->array[offset] = (1.0 - r / 3) * exp(-r / 2);
  weight += fabs(filter->array[offset]);
  offset++;
      }
    }
  }

  size_t idx;
  for (idx = 0; idx < offset; idx++) {
    filter->array[idx] /= weight;
  }   */
  /*
  for (int k = 0; k < m_sZ; k++) {
    double z=((double) k) - center_z;
    for (int j = 0; j < m_sY; j++) {
      double y= ((double) j) - center_y;
      for (int i = 0; i < m_sX; i++) {
        double x = ((double) i) - center_x;
        double r=x*x+y*y+z*z;
        r=r/sig2;
        factor[offset] = (1.0 - r/3.0) * std::exp(-r/2.0);
        weight += std::fabs(factor[offset]);
        offset++;
       }
     }
   }

  size_t idx;
  for (idx = 0; idx < offset; idx++) {
      factor[idx]/= weight;
  }

  double rv=0.0;

  for (idx = 0; idx < offset; idx++) {
      rv+=factor[idx]*buffer[idx];
  }
  return std::fabs(rv);*/
  /*size_t cnt=m_sX*m_sY*m_sZ;
  double aver=0;
  for(size_t i=0;i<cnt;++i){
    aver+=buffer[i];
  }
  aver/=cnt;
  double stddev=0.0;
  for(size_t i=0;i<cnt;++i){
    stddev+=(buffer[i]-aver)*(buffer[i]-aver);
  }
  stddev/=cnt;
  stddev=std::sqrt(stddev);
  if(stddev>255){
    stddev=255;
  }
  if(aver==0.0){
    return stddev;
  }
  return 255-stddev;*/
  int cnt=m_sX*m_sY*m_sZ;

  int que_cnt=std::min(std::max(1,cnt/10),10);
  int min[10]={255},max[10]={0};
  for(int i=0;i<cnt;++i){
    for(int j=0;j<que_cnt;++j){
      if (buffer[i]<min[j]){
        for(int k=que_cnt-1;k>j;--k){
          min[k]=min[k-1];
        }
        min[j]=buffer[i];
        break;
      }
    }
    for(int j=0;j<que_cnt;++j){
      if (buffer[i]>max[j]){
        for(int k=que_cnt-1;k>j;--k){
          max[k]=max[k-1];
        }
        max[j]=buffer[i];
        break;
      }
    }
  }
  double max_sum=0.0,min_sum=0.0;
  for(int i=0;i<que_cnt;++i){
    max_sum+=max[i];
    min_sum+=min[i];
  }
  double rv=(max_sum-min_sum)/que_cnt;
  if(rv==0.0){
    return 0.0;
  }
  return 255-rv;

}
