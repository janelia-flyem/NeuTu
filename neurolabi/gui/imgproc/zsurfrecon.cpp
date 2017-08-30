//#include "zstackdoc.h"
#include <set>
#include "zsurfrecon.h"

#if defined(_ENABLE_SURFRECON_)
void ZSurfRecon::SurfRecon(VoxelSet& in,VoxelSet& out)
{
  Surf s;
  s.surfrecon(in,out,26,1);
}


void ZSurfRecon::LabelStack(VoxelSet& surface,ZStack* stack,int value)
{
  uint8_t* p=stack->array8();
  int width=stack->width(),height=stack->height(),area=width*height;
  size_t _off=stack->getOffset().m_x+stack->getOffset().m_y*width+stack->getOffset().m_z*area;
  size_t max_off=stack->getVoxelNumber();
  for(uint i=0;i<surface.size();++i)
  {
    const VoxelType& v=surface[i];
    for(int x=std::floor(v.x);x<=std::ceil(v.x);++x)
    {
      for(int y=std::floor(v.y);y<=std::ceil(v.y);++y)
      {
        for(int z=std::floor(v.z);z<=std::ceil(v.z);++z)
        {
          size_t offset=x+y*width+z*area-_off;
          if(offset<max_off)
          {
            p[offset]=value;
          }
        }
      }
    }
  }
}


int gaussianBlur2(ZStack* src,int x,int y,int z,int r,double sigma)
{
  const static double tpi=std::pow(std::sqrt(2*3.1415926),3);
  double sigma_2=sigma*sigma;
  double sigma_3=sigma_2*sigma;

  int width=src->width(),height=src->height(),depth=src->depth(),area=width*height;
  size_t _off=src->getOffset().m_x+src->getOffset().m_y*width+src->getOffset().m_z*area;
  int x_min=std::max(0,x-r),x_max=std::min(width,x+r);
  int y_min=std::max(0,y-r),y_max=std::min(height,y+r);
  int z_min=std::max(0,z-r),z_max=std::min(depth,z+r);

  int size=(x_max-x_min)*(y_max-y_min)*(z_max-z_min);
  double* weights=new double[size];
  memset(weights,0,sizeof(double)*size);
  double total=0.0,weight=0.0;
  int index=0;

  for(int zz=z_min;zz<z_max;++zz)
  {
    for(int yy=y_min;yy<y_max;++yy)
    {
      for(int xx=x_min;xx<x_max;++xx)
      {
        double p=-1*((xx-x)*(xx-x)+(yy-y)*(yy-y)+(zz-z)*(zz-z));
        weight=1.0/(tpi*sigma_3)*std::exp(p/(2*sigma_2));
        weights[index++]=weight;
        total+=weight;
      }
    }
  }


  double value=0.0;
  index=0;
  for(int zz=z_min;zz<z_max;++zz)
  {
    for(int yy=y_min;yy<y_max;++yy)
    {
      for(int xx=x_min;xx<x_max;++xx)
      {
         value+=weights[index++]/total*src->array8()[xx+yy*width+zz*area-_off];
      }
    }
  }

  delete[] weights;
  return int(value);
}


void ZSurfRecon::GaussianBlur(VoxelSet& surface,ZStack* stack,int r,double sigma)
{
  int width=stack->width(),height=stack->height(),area=width*height;
  size_t _off=stack->getOffset().m_x+stack->getOffset().m_y*width+stack->getOffset().m_z*area;
  size_t max_off=stack->getVoxelNumber();
  uint8_t* p=stack->array8();
  for(uint i=0;i<surface.size();++i)
  {
    const VoxelType& v=surface[i];
    std::vector<int> gv;
    for(int x=std::floor(v.x);x<=std::ceil(v.x);++x)
    {
      for(int y=std::floor(v.y);y<=std::ceil(v.y);++y)
      {
        for(int z=std::floor(v.z);z<=std::ceil(v.z);++z)
        {
          size_t offset=x+y*width+z*area-_off;
          if(offset<max_off)
            gv.push_back(gaussianBlur2(stack,x,y,z,r,sigma));
        }
      }
    }
    int index=0;
    for(int x=std::floor(v.x);x<=std::ceil(v.x);++x)
    {
      for(int y=std::floor(v.y);y<=std::ceil(v.y);++y)
      {
        for(int z=std::floor(v.z);z<=std::ceil(v.z);++z)
        {
          size_t offset=x+y*width+z*area-_off;
          if(offset<max_off)
            p[offset]=gv[index++];
        }
      }
    }
  }
}


void ZSurfRecon::PruneSkeleton(VoxelSet& surface,ZSwcTree* tree)
{

  static double x64=std::pow(2.0,2.0*8*sizeof(int));
  static double x32=std::pow(2.0,8.0*sizeof(int));

  std::set<double> surf_map;
  for(uint i=0;i<surface.size();++i)
  {
    const VoxelType& v=surface[i];
    for(int x=std::floor(v.x);x<=std::ceil(v.x);++x)
    {
      for(int y=std::floor(v.y);y<=std::ceil(v.y);++y)
      {
        for(int z=std::floor(v.z);z<=std::ceil(v.z);++z)
        {
          surf_map.insert(x*x64+y*x32+z);
        }
      }
    }
  }

  std::vector<Swc_Tree_Node*> cnn;

  Swc_Tree_Node* root=tree->forceVirtualRoot();
  ZSwcTree::DepthFirstIterator it(tree);
  it.next();//skip virtual root
  while(it.hasNext())
  {
    Swc_Tree_Node* n=it.next();
    for(Swc_Tree_Node* p=n->first_child;p;p=p->next_sibling)
    {
      double px=p->node.x,py=p->node.y,pz=p->node.z;
      double qx=n->node.x,qy=n->node.y,qz=n->node.z;

      double x, y,z, xincre, yincre,zincre;
      int k = std::max(abs(px -qx),abs(py - qy));
      k=  std::max(double(k),abs(pz-qz));

      xincre = (qx - px)/(double)k;
      yincre = (qy - py)/(double)k;
      zincre = (qz - pz)/(double)k;

      x = px;
      y = py;
      z = pz;

      for (int i = 1; i < k; i++)
      {
          for(int zz=std::floor(z);zz<=std::ceil(z);++zz)
          {
            for(int yy=std::floor(y);yy<=std::ceil(y);++yy)
            {
              for(int xx=std::floor(x);xx<=std::ceil(x);++xx)
              {
                if(surf_map.find(xx*x64+yy*x32+zz)!=surf_map.end())
                {
                  cnn.push_back(p);
                  goto next;
                }
              }
            }
          }
          x += xincre;
          y += yincre;
          z += zincre;
      }
      next:;
    }
  }
  for(auto x:cnn)
  {
    SwcTreeNode::setParent(x,root);
  }

}
#endif
