#include "zstackgradient.h"

#include <cfloat>
#include <cmath>
#include <typeinfo>
#include <string.h>

#include "zstack.hxx"
#include "imgproc/zstackprocessor.h"


template<typename T>
GradientStrategy<T>::GradientStrategy()
{
  if(typeid(T)==typeid(uint8_t) || typeid(T)==typeid(color_t))
    _max=255;
  else if(typeid(T)==typeid(uint16_t))
    _max=65535;
  else if(typeid(T)==typeid(float))
    _max=FLT_MAX;
  else if(typeid(T)==typeid(double))
    _max=DBL_MAX;
  else
    throw std::string("GradientStategy not support ")+typeid(T).name()+" yet";
}


template<typename T>
void GradientStrategy<T>::run(
    const T* in, T* out, uint32_t width, uint32_t height, uint32_t depth,
    bool ignore_background)
{
  _width=width;
  _height=height;
  _depth=depth;
  _slice=_width*_height;
  _total=_slice*_depth;
  _ignore_background=ignore_background;
  _run(in,out);
}


template<typename T>
void GradientStrategy<T>::edgeEnhance(const T* in,T* out,double alpha)
{
  for(size_t i=0;i<this->_total;++i)
  {
    out[i]=(_ignore_background&&in[i]==0)?0:(1-alpha)*out[i]+in[i]*(alpha);
  }
}


template<>
void GradientStrategy<color_t>::edgeEnhance(const color_t* /*in*/,color_t* /*out*/,double /*alpha*/)
{
  //not support yet
  /*for(size_t i=0;i<this->_total;++i)
  {
    for(uint j=0;j<3;++j)
    {
      out[i][j]=in[i][j]==0?0:(1-alpha)*out[i][j]+in[i][j]*alpha;
    }
  }*/
}


template<typename T>
void GradientStrategy<T>::reverse(T* begin,T* end)
{
  for(T* it=begin;it!=end;++it)
    *it=(_ignore_background && *it==0)?0:_max-*it;
}


template<>
void GradientStrategy<color_t>::reverse(color_t* /*begin*/,color_t* /*end*/)
{
  //not support yet
  /*
  for(color_t* it=begin;it!=end;++it)
    for(uint i=0;i<3;++i)
      (*it)[i]=_max-(*it)[i];*/
}


GradientStrategyContext::GradientStrategyContext(StrategyType strategy_type)
{
  _type=strategy_type;
}


GradientStrategyContext::~GradientStrategyContext()
{

}


void GradientStrategyContext::run
(
    const ZStack* in,
    ZStack* out,
    bool reverse,
    bool ignore_background,
    double edge_enhance_alpha,
    double gaussin_smooth_sigma_x,
    double gaussin_smooth_sigma_y,
    double gaussin_smooth_sigma_z
    )
{
  if(!in || !out)
    return ;

#define _run_(type)\
  _run<type>(in,out,edge_enhance_alpha,gaussin_smooth_sigma_x,\
  gaussin_smooth_sigma_y,gaussin_smooth_sigma_z,reverse,ignore_background)
  switch(in->kind())
  {
    case GREY:
          _run_(uint8);
          break;
    case GREY16:
           _run_(uint16);
          break;
    case FLOAT32:
          _run_(float32);
          break;
    case FLOAT64:
          _run_(float64);
          break;
    case COLOR:
           _run_(color_t);
          break;
    default:
          break;
  }
  out->setOffset(in->getOffset());
#undef _run_
}


template<typename T>
void GradientStrategyContext:: _run
(
    const ZStack* in,
    ZStack* out,
    double edge_enhance_alpha,
    double gaussin_smooth_sigma_x,
    double gaussin_smooth_sigma_y,
    double gaussin_smooth_sigma_z,
    bool reverse,
    bool ignore_background)
{
  GradientStrategy<T>* strategy=getStrategy<T>();
  size_t total=in->width()*in->height()*in->depth();

  if(strategy)
  {
    /*process each channel*/
    for(int i=0;i<in->channelNumber();++i)
    {
      const T*_in=(const T*)in->array8(i);
      T* _out=(T*)out->array8(i);
      strategy->run(_in,_out,in->width(),in->height(),in->depth(),ignore_background);
    }

    if(
       (gaussin_smooth_sigma_x!=0.0) ||
       (gaussin_smooth_sigma_y!=0.0) ||
       (gaussin_smooth_sigma_z!=0.0))
    {
      for(int i=0;i<out->channelNumber();++i)
      {
        Stack* p=ZStackProcessor::GaussianSmooth(
              out->c_stack(i),gaussin_smooth_sigma_x,
              gaussin_smooth_sigma_y,gaussin_smooth_sigma_z);
        memcpy(out->array8(i),p->array,sizeof(T)*total);
        Kill_Stack(p);
      }
    }

    if(reverse)
    {
      for(int i=0;i<in->channelNumber();++i)
      {
        T* _out=(T*)out->array8(i);
        strategy->reverse(_out,_out+total);
      }
    }

    if(edge_enhance_alpha!=0.0)
    {
      for(int i=0;i<in->channelNumber();++i)
      {
        const T*_in=(const T*)in->array8(i);
        T* _out=(T*)out->array8(i);
        strategy->edgeEnhance(_in,_out,edge_enhance_alpha);
      }
    }

    delete strategy;
  }
}


template<typename T>
void GradientStrategySimple<T>::process(
    uint32_t &x, uint32_t &y , uint32_t &z, uint32_t &w, const T* pi, T* p, uint32_t offset, uint32_t end)
{
  bool ignore_background=this->_ignore_background;
  for(z=0;z<this->_depth;++z)
    for(y=0;y<this->_height;++y)
      for(x=0;x<this->_width;++x,++pi,++p)
      {
        if(ignore_background && *pi==0)
        {
          *p=0;
        }
        else
        {
          if(w==0)*p=std::abs(double(*pi)-*(pi+offset));
          else if(w==end)*p=std::abs(double(*(pi-offset))-*pi);
          else*p=std::abs(double(*(pi+offset))-*(pi-offset))/2.0;
        }
      }
}


template<>
void GradientStrategySimple<color_t>::process(uint& /*x*/,uint&/*y*/ ,uint&/*z*/,uint& /*w*/,const color_t* /*pi*/,color_t* /*p*/,uint /*offset*/,uint /*end*/)
{
  //not support yet
  /*
  for(z=0;z<this->_depth;++z)
    for(y=0;y<this->_height;++y)
      for(x=0;x<this->_width;++x,++pi,++p)
        for(uint t=0;t<3;++t)
        {
          if(w==0)(*p)[t]=std::abs(double((*pi)[t])-(*(pi+offset))[t]);
          else if(w==end)(*p)[t]=std::abs(double((*(pi-offset))[t])-(*pi)[t]);
          else(*p)[t]=std::abs(double((*(pi+offset))[t])-(*(pi-offset))[t])/2.0;
        }*/

}


template<typename T>
void GradientStrategySimple<T>::_run(const T* in,T* out)
{
  uint width=this->_width,height=this->_height,depth=this->_depth;
  size_t slice=this->_slice,total=this->_total;
  double max=this->_max;


  T *px=new T[total],*py=new T[total],*pz=new T[total];
  T *_px=px,*_py=py,*_pz=pz;
  memset(px,0,sizeof(T)*total);
  memset(py,0,sizeof(T)*total);
  memset(pz,0,sizeof(T)*total);
  const T* pi=in;
  uint x,y,z;
  if(width>1)
  {
    process(x,y,z,x,pi,_px,1,width-1);
  }
  pi=in;
  if(height>1)
  {
    process(x,y,z,y,pi,_py,width,height-1);
  }
  pi=in;
  if(depth>1)
  {
    process(x,y,z,z,pi,_pz,slice,depth-1);
  }
  for(uint i=0;i<total;++i)
  {
    out[i]=std::min(sqrt(static_cast<double>(px[i]*px[i]+py[i]*py[i]+pz[i]*pz[i])),max);
  }
  delete[] px;
  delete[] py;
  delete[] pz;
}


template<>
void GradientStrategySimple<color_t>::_run(const color_t* /*in*/,color_t* /*out*/)
{
  //not support yet
  /*
  uint width=this->_width,height=this->_height,depth=this->_depth;
  size_t slice=this->_slice,total=this->_total;
  double max=this->_max;

  color_t *px=new color_t[total],*py=new color_t[total],*pz=new color_t[total];
  color_t *_px=px,*_py=py,*_pz=pz;
  memset(px,0,sizeof(color_t)*total);
  memset(py,0,sizeof(color_t)*total);
  memset(pz,0,sizeof(color_t)*total);

  const color_t* pi=in;
  uint x,y,z;
  if(width>1)
  {
    process(x,y,z,x,pi,_px,1,width-1);
  }
  if(height>1)
  {
    pi=in;
    process(x,y,z,y,pi,_py,width,height-1);
  }
  if(depth>1)
  {
    pi=in;
    process(x,y,z,z,pi,_pz,slice,depth-1);
  }
  for(uint i=0;i<total;++i)
  {
    for(uint t=0;t<3;++t)
    {
      out[i][t]=std::min(sqrt(static_cast<double>
        (px[i][t]*px[i][t]+py[i][t]*py[i][t]+pz[i][t]*pz[i][t])),max);
    }
  }
  delete[] px;
  delete[] py;
  delete[] pz;*/
}
