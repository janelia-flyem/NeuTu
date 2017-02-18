#include "zstackmultiscalewatershed.h"
#include "zstackwatershed.h"
#include "zstackdoc.h"

void getEdgePoints(ZStack* stack,std::vector<ZIntPoint>** metrix)
{
  int width=stack->width();
  int height=stack->height();
  int depth=stack->depth();
  int slice=width*height;
  uint8_t* po=stack->array8();
  for(int k=0;k<depth;++k)
  {
    size_t offset=k*slice;
    for(int j=0;j<height;++j)
    {
      size_t _offset=offset+j*width;
      for(int i=0;i<width;++i)
      {
        uint8_t v=po[_offset+i];
        for(int z=k;z<=std::min(depth-1,k+1);++z)
          for(int y=j;y<=std::min(height-1,j+1);++y)
            for(int x=i;x<=std::min(width-1,i+1);++x)
            {
              uint8_t t=po[z*slice+y*width+x];
              if(v!=t)
              {
                metrix[v][t].push_back(ZIntPoint(i,j,k));
                metrix[t][v].push_back(ZIntPoint(x,y,z));
                break;
              }
            }
      }
    }
  }

}

ZStack* recover2OriginalSize(ZStack* original,ZStack* sampled,int step)
{
  int width=original->width();
  int height=original->height();
  int depth=original->depth();
  ZStack* recover=new ZStack(original->kind(),width,height,depth,original->channelNumber());
  uint8_t* pr=recover->array8();
  uint8_t* ps=sampled->array8();
  int s_width=sampled->width();
  int s_slice=s_width*sampled->height();
  size_t s_offset=0;
  int r_slice=width*height;

  for(int k=0;k<sampled->depth();++k)
  {
    s_offset=k*s_slice;
    for(int j=0;j<sampled->height();++j)
    {
      size_t ss_offset=s_offset+j*s_width;
      for(int i=0;i<s_width;++i)
      {
        uint8_t v=ps[ss_offset+i];
        for(int d=k*step;d<std::min((k+1)*step,depth);++d)
          for(int h=j*step;h<std::min((j+1)*step,height);++h)
            for(int w=i*step;w<std::min((i+1)*step,width);++w)
              pr[d*r_slice+h*width+w]=v;

      }
    }
  }
  return recover;
}


ZStack* getBoundBoxes(ZStack* stack,std::vector<ZIntCuboid>&boxes)
{
  int width=stack->width();
  int height=stack->height();
  int slice=width*height;

  int size=std::ceil(stack->max());
  std::vector<ZIntPoint>** metrix=new std::vector<ZIntPoint>*[size+1];
  for(int i=0;i<size+1;++i)
  {
    metrix[i]=new std::vector<ZIntPoint>[size+1];
  }
  getEdgePoints(stack,metrix);

  ZStack* rv=new ZStack(stack->kind(),width,height,stack->depth(),stack->channelNumber());
  uint8_t* pr=rv->array8();
  //rv->setZero();
  int index=1;
  for(int i=0;i<size;++i)
  {
    for(int j=i+1;j<size+1;++j)
    {
      int s=metrix[i][j].size();
      for(int k=0;k<s;++k)
      {
        const ZIntPoint& p=metrix[i][j][k];
        const ZIntPoint& q=metrix[j][i][k];
        pr[p.m_x+p.m_y*width+p.m_z*slice]=index;
        pr[q.m_x+q.m_y*width+q.m_z*slice]=index;
      }
      if(s)
      {
        index++;
        assert(index<256);
      }
    }
  }

  for(int i=0;i<size+1;++i)
  {
    delete[] metrix[i];
  }
  delete[] metrix;

  std::vector<ZObject3dScan*>edges=ZObject3dScan::extractAllObject(*rv);
  for(std::vector<ZObject3dScan*>::iterator it=edges.begin();it!=edges.end();++it)
  {
    boxes.push_back((*it)->getBoundBox());
    delete *it;
  }
  return rv;
}


inline bool isEdgePoint(const ZStack* edge_map,int x,int y,int z)
{
  int width=edge_map->width();
  int height=edge_map->height();
  int slice=width*height;
  const uint8_t* p=edge_map->array8();
  return p[x+y*width+z*slice];
}

inline bool hasNeighborOnEdge(const ZStack* edge_map,int x,int y,int z)
{
  int width=edge_map->width();
  int height=edge_map->height();
  int depth=edge_map->depth();
  int slice=width*height;
  const uint8_t* p=edge_map->array8();
  for(int k=std::max(0,z-1);k<=std::min(depth-1,z+1);++k)
  {
    for(int j=std::max(0,y-1);j<=std::min(height-1,y+1);++j)
    {
      for(int i=std::max(0,x-1);i<=std::min(width-1,x+1);++i)
      {
        if(p[i+j*width+k*slice])
        {
          return true;
        }
      }
    }
  }
  return false;
}

void generateSeedsInSampledSpace(std::vector<ZStack*>& seeds,
                                 const ZIntCuboid& box,
                                 const ZStack* edge_map,
                                 const ZStack* stack,int step)
{
  int width=stack->width();
  int height=stack->height();
  int slice=width*height;
  const uint8_t* ps=stack->array8();

  int start_x=box.getFirstCorner().m_x;
  int start_y=box.getFirstCorner().m_y;
  int start_z=box.getFirstCorner().m_z;
  int end_x=box.getLastCorner().m_x;
  int end_y=box.getLastCorner().m_y;
  int end_z=box.getLastCorner().m_z;

  int ofx=stack->getOffset().m_x;
  int ofy=stack->getOffset().m_y;
  int ofz=stack->getOffset().m_z;
  //find points which are appropriate to be seeds in the box
  for(int z=start_z;z<=end_z;++z)
  {
    for(int y=start_y;y<=end_y;++y)
    {
      for(int x=start_x;x<=end_x;++x)
      {
        if( (!isEdgePoint(edge_map,x,y,z)) && hasNeighborOnEdge(edge_map,x,y,z))
        {
          ZStack* seed=new ZStack(GREY,step,step,step,1);
          int v=ps[x+y*width+z*slice];
          for(uint i=0;i<seed->getVoxelNumber();++i)
          {
            seed->array8()[i]=v;
          }
          seed->setOffset(x+ofx,y+ofy,z+ofz);
          seeds.push_back(seed);
        }
      }
    }
  }
}


void mapSeeds2OriginalSpace(std::vector<ZStack*>& seeds,const int step,ZStack* src)
{
  int width=src->width(),height=src->height(),depth=src->depth();
  for(std::vector<ZStack*>::iterator it=seeds.begin();it!=seeds.end();++it)
  {
    ZStack* seed=*it;
    ZIntPoint offset=seed->getOffset();
    int x=std::max(offset.m_x*step,src->getOffset().m_x);
    int y=std::max(offset.m_y*step,src->getOffset().m_y);
    int z=std::max(offset.m_z*step,src->getOffset().m_z);
    if(x>=src->getOffset().m_x+width)x=src->getOffset().m_x+width-1;
    if(y>=src->getOffset().m_y+height)y=src->getOffset().m_y+height-1;
    if(z>=src->getOffset().m_z+depth)z=src->getOffset().m_z+depth-1;
    seed->setOffset(x,y,z);
  }
}


void drawSeeds(const std::vector<ZStack*>& seeds,ZStack* stack,int step,const int v)
{
  uint8_t* p=stack->array8();
  const int width=stack->width();
  const int height=stack->height();
  const int area=width*stack->height();
  for(std::vector<ZStack*>::const_iterator it=seeds.begin();it!=seeds.end();++it)
  {
    const ZStack* seed=*it;
    const ZIntPoint point=seed->getOffset();
    const int x=point.m_x,y=point.m_y,z=point.m_z;
    for(int i=x;i<=std::min(width-1,x+step);++i)
    {
      for(int j=y;j<=std::min(height-1,y+step);++j)
      {
         p[z*area+j*width+i]=v;
       }
    }
  }
}


void drawRange(ZStack* stack,const Cuboid_I& range,int v)
{
  const int x1=range.cb[0],x2=range.ce[0];
  const int y1=range.cb[1],y2=range.ce[1];
  const int z1=range.cb[2],z2=range.ce[2];
  const int width=stack->width();
  const int area=stack->height()*width;
  uint8_t* p=stack->array8();

  for(int k=z1;k<=z2;++k)
    for(int i=x1;i<=x2;++i)
      p[i+y1*width+k*area]=p[i+y2*width+k*area]=v;

  for(int k=z1;k<=z2;++k)
    for(int j=y1;j<=y2;++j)
      p[x1+j*width+k*area]=p[x2+j*width+k*area]=v;

}


void setNoneEdgePoints2Zero(ZStack* stack,const ZStack* edge_map,
                            const Cuboid_I& range,int step)
{

  int width=stack->width(),height=stack->height(),depth=stack->depth(),slice=width*height;

  uint8_t* q=stack->array8();

  for(int z=range.cb[2]/step;z<=range.ce[2]/step;++z)
  {
    for(int y=range.cb[1]/step;y<=range.ce[1]/step;++y)
    {
      for(int x=range.cb[0]/step;x<=range.ce[0]/step;++x)
      {
        if(!hasNeighborOnEdge(edge_map,x,y,z))
        {
          for(int d=z*step;d<std::min((z+1)*step,depth);++d)
            for(int h=y*step;h<std::min((y+1)*step,height);++h)
              for(int w=x*step;w<std::min((x+1)*step,width);++w)
              {
                q[d*slice+h*width+w]=0;
              }
        }
      }
    }
  }
}

void localWaterShed(const std::vector<ZStack*>& seeds,
                    Cuboid_I& range,
                    ZStack* recovered,ZStack* original,const ZStack* edge_map,int step)
{

  int x1=range.cb[0],x2=range.ce[0];
  int y1=range.cb[1],y2=range.ce[1];
  int z1=range.cb[2],z2=range.ce[2];
  ZStackWatershed watershed;
  //set points that are (not seeds and not at any edge) to zero
  setNoneEdgePoints2Zero(original,edge_map,range,step);

  range.cb[0]+=original->getOffset().m_x;
  range.ce[0]+=original->getOffset().m_x;
  range.cb[1]+=original->getOffset().m_y;
  range.ce[1]+=original->getOffset().m_y;
  range.cb[2]+=original->getOffset().m_z;
  range.ce[2]+=original->getOffset().m_z;
  watershed.setRange(range);
  watershed.setFloodingZero(false);
/*  std::cout<<"no"<<range.cb[0]<<" "<<range.cb[1]<<" "<<range.cb[2]<<std::endl;
  std::cout<<"no"<<range.ce[0]<<" "<<range.ce[1]<<" "<<range.ce[2]<<std::endl;
  std::cout<<"no"<<seeds[0]->getOffset().m_x<<" "<<seeds[0]->getOffset().m_y<<" "<<
             seeds[0]->getOffset().m_z<<std::endl;
  std::cout<<"no"<<original->getOffset().m_x<<" "<<
             original->getOffset().m_y<<" "<<
             original->getOffset().m_z<<std::endl;
  std::cout.flush();*/

  ZStack* result=watershed.run(original,seeds);
  if(!result)
  {
    std::cout<<"local watershed failed"<<std::endl;
    return;
  }
  //merge result into recovered image
  uint8_t* p=recovered->array8();
  uint8_t* q=result->array8();
  int width=original->width();
  int slice=width*original->height();

  int r_w=result->width(),r_s=result->height()*r_w;

  int rofx=result->getOffset().m_x-original->getOffset().m_x,
      rofy=result->getOffset().m_y-original->getOffset().m_y,
      rofz=result->getOffset().m_z-original->getOffset().m_z;
  for(int z=z1;z<=z2;++z)
  {
    for(int y=y1;y<=y2;++y)
    {
      for(int x=x1;x<=x2;++x)
      {
        int v=q[x-rofx+(y-rofy)*r_w+(z-rofz)*r_s];
        if(v)
        {
          p[x+y*width+z*slice]=v;
        }
      }
    }
  }
 // recovered->setBlockValue(range.cb[0],range.cb[1],range.cb[2],result);
  delete result;
}


Cuboid_I getRange(const ZIntCuboid& box,const ZStack* stack,int step)
{
  Cuboid_I range;
  const ZIntPoint&p=box.getFirstCorner();
  const ZIntPoint&q=box.getLastCorner();

  int width=stack->width();
  int height=stack->height();
  int depth=stack->depth();

  range.cb[0]=p.m_x*step;
  range.cb[1]=p.m_y*step;
  range.cb[2]=p.m_z*step;
  range.ce[0]=std::min((q.m_x+1)*step-1,width-1);
  range.ce[1]=std::min((q.m_y+1)*step-1,height-1);
  range.ce[2]=std::min((q.m_z+1)*step-1,depth-1);

  return range;
}


ZStackMultiScaleWatershed::ZStackMultiScaleWatershed()
{

}


ZStackMultiScaleWatershed::~ZStackMultiScaleWatershed()
{

}


ZStack* ZStackMultiScaleWatershed::run(ZStack *src,QList<ZSwcTree*>& trees,int scale)
{
  _scale=scale;
  ZStack* rv=0;
  std::vector<ZStack*> seeds;
  getSeeds(seeds,trees);
  //run watershed
  ZStackWatershed watershed;
  if(scale==1)
  {
    rv=watershed.run(src,seeds);
    for(std::vector<ZStack*>::iterator it=seeds.begin();it!=seeds.end();++it)
    {
      delete *it;
    }
    return rv;
  }
  //down sample src stack
  ZStack* sampled=src->clone();
  sampled->downsampleMin(scale-1,scale-1,scale-1);

  ZStack* sampled_watershed=watershed.run(sampled,seeds);

  if(sampled_watershed)
  {
    ZStack* src_clone=src->clone();
      /*uint8_t* p=src_clone->array8();
      for(uint i=0;i<src_clone->getVoxelNumber();++i)
      {
        if(p[i]==0)p[i]=1;
      }*/
    rv=upSampleAndRecoverEdge(sampled_watershed,src_clone);
    delete sampled_watershed;
    delete src_clone;
  }
  delete sampled;
  for(std::vector<ZStack*>::iterator it=seeds.begin();it!=seeds.end();++it)
  {
    delete *it;
  }
  return rv;
}


ZStack* ZStackMultiScaleWatershed::upSampleAndRecoverEdge(ZStack* sampled_watershed,ZStack* src)
{
  ZStack* recovered=recover2OriginalSize(src,sampled_watershed,_scale);
  //ZStack* recovered=new ZStack(src->kind(),src->width(),src->height(),src->depth(),1);
  recovered->setOffset(src->getOffset());
  //return recovered;
  std::vector<ZIntCuboid>boxes;
  std::vector<ZStack*> seeds;
  //get bound boxes of each each
  ZStack* edge_map=getBoundBoxes(sampled_watershed,boxes);

  //process each bound box
  for(uint i=0;i<boxes.size();++i)//deal with each edge
  {
      const ZIntCuboid& box=boxes[i];
      Cuboid_I range=getRange(box,src,_scale);

      generateSeedsInSampledSpace(seeds,box,edge_map,sampled_watershed,_scale);

      mapSeeds2OriginalSpace(seeds,_scale,src);

      localWaterShed(seeds,range,recovered,src,edge_map,_scale);

      for(uint j=0;j<seeds.size();++j)
      {
         delete seeds[j];
      }
      seeds.clear();
  }

  delete edge_map;
  return recovered;

}

void ZStackMultiScaleWatershed::getSeeds(std::vector<ZStack*>& seeds,QList<ZSwcTree*>& trees)
{
  if(trees.size()<2 || trees.size()>255)
  {
    return ;
  }
  uint seed_index=1;

  for(QList<ZSwcTree*>::iterator it=trees.begin();it!=trees.end();++it)
  {
    ZSwcTree* tree=*it;
    ZCuboid box=tree->getBoundBox();
    ZStack* seed=new ZStack(GREY,std::max(1,(int)(box.width()/_scale)),
                            std::max(1,(int)(box.height()/_scale)),
                            std::max(1,(int)(box.depth()/_scale)),1);
    ZPoint f=box.firstCorner();
    ZPoint l=box.lastCorner();
    ZIntPoint off((f.getX()+l.getX())/2/_scale,
                  (f.getY()+l.getY())/2/_scale,
                  (f.getZ()+l.getZ())/2/_scale);
    seed->setOffset(off);
    uint8_t* p=seed->array8();
    for(uint i=0;i<seed->getVoxelNumber();++i)
    {
      *p++=seed_index;
    }
    seed_index++;
    seeds.push_back(seed);
  }
}
