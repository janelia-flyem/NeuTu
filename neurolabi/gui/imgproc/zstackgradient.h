#ifndef ZSTACKGRADIENT_H
#define ZSTACKGRADIENT_H
#include "zstack.hxx"

/* GradientStrategy
 * abstract base class of computing gradient
 * and subclasses should override _run method*/
template<typename T>
class GradientStrategy
{
public:
  GradientStrategy();
  //virtual ~GradientStrategy();
  void run(const T* in,T* out,uint width,uint height,uint depth,bool ignore_background);
  void reverse(T* begin,T* end);
  void edgeEnhance(const T* in,T* out,double alpha);
protected:
  virtual void _run(const T* in,T* out)=0;
protected:
  double _max;
  uint _width,_height,_depth;
  size_t _slice,_total;
  bool _ignore_background;
};


/* simple gradient strategy which uses df/dx=[f(x+1)-f(x-1)]/2*/
template<typename T>
class GradientStrategySimple:public GradientStrategy<T>
{
protected:
  void _run(const T* in,T* out);
private:
  void process(uint& x,uint&y ,uint&z,uint& w,const T* pi,T* p,uint offset,uint end);
};


/*GradientStrategyContext is the user interface to use gradient strategy classes*/
class GradientStrategyContext
{
public:
  typedef enum
  {
    SIMPLE=0
  }StrategyType;
public:
  GradientStrategyContext(StrategyType strategy_type);
  ~GradientStrategyContext();
  void run(const ZStack* in,ZStack* out,
           bool reverse=false,
           bool ignore_background=false,
           double edge_enhance_alpha=0.0,
           double gaussin_smooth_sigma_x=0.0,
           double gaussin_smooth_sigma_y=0.0,
           double gaussin_smooth_sigma_z=0.0);
private:
  template<typename T>
  GradientStrategy<T>* getStrategy()
  {
    GradientStrategy<T>* strategy=0;
    switch(_type)
    {
      case SIMPLE:
          strategy=new GradientStrategySimple<T>;
          break;
      default:
          strategy=0;
          break;
    }
    return strategy;
  }
  template<typename T>
  void _run(const ZStack* in,ZStack* out,double edge_enhance_alpha=0.0,
            double gaussin_smooth_sigma_x=0.0,
            double gaussin_smooth_sigma_y=0.0,
            double gaussin_smooth_sigma_z=0.0,
            bool reverse=false,
            bool ignore_background=false);
private:
  StrategyType _type;
};
#endif // ZSTACKGRADIENT_H
