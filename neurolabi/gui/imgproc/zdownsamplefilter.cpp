#include "cstdlib"
#include "zdownsamplefilter.h"
#include "zstack.hxx"

ZDownsampleFilter::ZDownsampleFilter()
{
  m_dsX=m_dsY=m_dsZ=1;
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
  delete buffer;
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
  const double sigma_x=3.0,sigma_y=3.0,sigma_z=1.0;
  const double sigma_x2=sigma_x*sigma_x;
  const double sigma_y2=sigma_y*sigma_y;
  const double sigma_z2=sigma_z*sigma_z;
  const double sigma_x4=sigma_x2*sigma_x2;
  const double sigma_y4=sigma_y2*sigma_y2;
  const double sigma_z4=sigma_z2*sigma_z2;

  int center_x=m_sX/2,center_y=m_sY/2,center_z=m_sZ/2;
  if(buffer[center_z*m_sX*m_sY+center_y*m_sX+center_x]==0){
    return 0.0;
  }

  double rv=0;
  double sum=0.0;
  for(int k=0;k<m_sZ;++k){
    double z=k-center_z;
    for(int j=0;j<m_sY;++j){
      double y=j-center_y;
      for(int i=0;i<m_sX;++i){
        double x=i-center_x;
        double factor=(x*x/sigma_x4-1/sigma_x2 +
                       y*y/sigma_y4-1/sigma_y2 +
                       z*z/sigma_z4-1/sigma_z2)*
                       std::exp(-x*x/(2*sigma_x2)-y*y/(2*sigma_y2)-z*z/(2*sigma_z2));
        sum+=factor;
        rv+=factor*buffer[k*m_sX*m_sY+j*m_sX+i];
      }
    }
  }
  return rv/sum;
}
