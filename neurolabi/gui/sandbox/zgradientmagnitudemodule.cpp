#include <QAction>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QRadioButton>
#include "zgradientmagnitudemodule.h"
#include "mainwindow.h"
#include "zsandbox.h"


ZGradientMagnitudeModule::ZGradientMagnitudeModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}


ZGradientMagnitudeModule::~ZGradientMagnitudeModule()
{
  if(select_strategy_window)
  {
    delete select_strategy_window;
  }
}


void ZGradientMagnitudeModule::init()
{
  QAction *action = new QAction("GradientMagnitude", this);
  connect(action, SIGNAL(triggered()), this, SLOT(execute()));
  select_strategy_window=new ZSelectGradientStrategyWindow();
  setAction(action);
}


void ZGradientMagnitudeModule::execute()
{
  select_strategy_window->show();
}


ZSelectGradientStrategyWindow::ZSelectGradientStrategyWindow(QWidget *parent)
  :QWidget(parent)
{
  this->setWindowTitle("Select Gradient Strategy");
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::WindowStaysOnTopHint;
  this->setWindowFlags(flags);
  QHBoxLayout* lay=new QHBoxLayout(this);
  start_gradient_magnitude=new QPushButton("start");
  strategies=new QComboBox;
  strategies->addItem("simple");
  inverse=new QRadioButton("inverse");
  lay->addWidget(strategies);
  lay->addWidget(inverse);
  lay->addWidget(start_gradient_magnitude);
  this->setLayout(lay);
  this->resize(200,100);
  this->move(300,200);
  connect(start_gradient_magnitude,SIGNAL(clicked()),this,SLOT(onStart()));
  strategy_map["simple"]=GradientStrategyType::SIMPLE;
}


void ZSelectGradientStrategyWindow::onStart()
{
  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  ZStack  *input_stack=doc->getStack();
  if(!doc || ! input_stack)
  {
    return;
  }

  int kind=input_stack->kind();
  int width=input_stack->width();
  int height=input_stack->height();
  int depth=input_stack->depth();
  int channel=input_stack->channelNumber();

  ZStack* mag=new ZStack(kind,width,height,depth,channel);

  QString s=strategies->currentText();
  GradientStrategyContext context(input_stack,mag,inverse->isChecked(),strategy_map[s]);
  context.run();

  ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(mag);
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);

}


GradientStrategyContext::GradientStrategyContext(
    const ZStack* input_stack,ZStack* output_x,ZStack* output_y,
    ZStack* output_z,ZStack* mag,bool inverse,GradientStrategyType::StrategyType strategy_type)
{
  _in=input_stack;
  _outx=output_x;
  _outy=output_y;
  _outz=output_z;
  _mag=mag;
  _type=strategy_type;
  _inverse=inverse;
  delete_out_xyz=false;
}


GradientStrategyContext::GradientStrategyContext(
    const ZStack* in,ZStack* out,bool inverse,GradientStrategyType::StrategyType strategy_type)
{
  _in=in;
  _mag=out;
  _inverse=inverse;
  _outx=new ZStack(in->kind(),in->width(),in->height(),in->depth(),in->channelNumber());
  _outy=new ZStack(in->kind(),in->width(),in->height(),in->depth(),in->channelNumber());
  _outz=new ZStack(in->kind(),in->width(),in->height(),in->depth(),in->channelNumber());
  _type=strategy_type;
  delete_out_xyz=true;
}


GradientStrategyContext::~GradientStrategyContext()
{
  if(delete_out_xyz)
  {
    delete _outx;
    delete _outy;
    delete _outz;
  }
}


void GradientStrategyContext::run()
{
  if(!_in || !_outx || !_outy || !_outz)
    return ;

  switch(_in->kind())
  {
    case GREY:
          _run<uint8>();
          break;
    case GREY16:
          _run<uint16>();
          break;
    case FLOAT32:
          _run<float32>();
          break;
    case FLOAT64:
          _run<float64>();
          break;
    case COLOR:
          _run<color_t>();
          break;
    default:
          break;
  }
}


template<typename T>
void GradientStrategyContext:: _run()
{
  GradientStrategy<T>* strategy=0;
  switch(_type)
  {
    case GradientStrategyType::SIMPLE:
        strategy=new GradientStrategySimple<T>;
        break;
    default:
        strategy=0;
        break;
  }
  if(strategy)
  {
    GradientStrategyParam p(_in->width(),_in->height(),_in->depth(),_inverse);
    /*process each channel*/
    for(int i=0;i<_in->channelNumber();++i)
    {
      const T*in=(const T*)_in->array8(i);
      T* out_x=(T*)_outx->array8(i);
      T* out_y=(T*)_outy->array8(i);
      T* out_z=(T*)_outz->array8(i);
      T* mag=(T*)_mag->array8(i);
      strategy->run(in,out_x,out_y,out_z,mag,p);
    }
    delete strategy;
  }
}


template<typename T>
void GradientStrategySimple<T>::run(const T* in,T* out_x,T* out_y,T* out_z,T* mag,GradientStrategyParam p)
{
  uint width=p.width;
  uint height=p.height;
  uint depth=p.depth;
  uint t=width*height;

  for(uint z=0;z<depth;++z)
  {
    uint offset_s=t*z;
    for(uint y=0;y<height;++y)
    {
      uint offset=offset_s+width*y;
      const T* pi=in+offset;
      T* px=out_x+offset;
      T* py=out_y+offset;
      T* pz=out_z+offset;
      for(uint x=0;x<width;++x,++px,++py,++pz,++pi)
      {
        /*convert to double to avoid substraction operation overflow*/
        if(x==0)
          *px=abs(double(*px)-*(px+1));
        else if(x==width-1)
          *px=abs(double(*(pi-1))-*pi);
        else
          *px=abs(double(*(pi+1))-*(pi-1))/2.0;
        if(y==0)
          *py=abs(double(*(pi+width))-*pi);
        else if(y==height-1)
          *py=abs(double(*pi)-*(pi-width));
        else
          *py=abs(double(*(pi+width))-*(pi-width))/2.0;
        if(z==0)
          *pz=abs(double(*(pi+t))-*(pi));
        else if(z==depth-1)
          *pz=abs(double(*(pi))-*(pi-t));
        else
          *pz=abs(double(*(pi+t))-*(pi-t))/2.0;
      }
    }
  }
  if(mag)
  {
    int sign=p.inverse?-1:1;
    double max=DBL_MAX;
    if(typeid(uint8)==typeid(T))
    {
      max=255.0;
    }
    else if(typeid(uint16)==typeid(T))
    {
      max=65535.0;
    }
    else if(typeid(float32)==typeid(T))
    {
      max=FLT_MAX;
    }
    double inver_max=p.inverse?max:0;
    T *p_x=out_x,*p_y=out_y,*p_z=out_z,*p_o=mag;
    uint num=width*height*depth;
    const T* end_x=p_x+num;
    T x,y,z;
    for(;p_x!=end_x;++p_x,++p_y,++p_z,++p_o)
    {
      x=*p_x;
      y=*p_y;
      z=*p_z;
      *p_o=inver_max+sign*std::min(sqrt(static_cast<double>(x*x+y*y+z*z)),max);
    }
  }
}


template<>
void GradientStrategySimple<color_t>::run(const color_t* in,color_t* out_x,
                                          color_t* out_y,color_t* out_z,color_t* mag,
                                          GradientStrategyParam p)
{
  uint width=p.width;
  uint height=p.height;
  uint depth=p.depth;
  uint t=width*height;
  for(uint z=0;z<depth;++z)
  {
    uint offset_s=t*z;
    for(uint y=0;y<height;++y)
    {
      uint offset=offset_s+width*y;
      const color_t* pi=in+offset;
      color_t* px=out_x+offset;
      color_t* py=out_y+offset;
      color_t* pz=out_z+offset;
      for(uint x=0;x<width;++x,++px,++py,++pz,++pi)
      {
        //convert to double to avoid substraction operation overflow
        for(uint y=0;y<3;++y)
        {
          if(x==0)
            *px[y]=abs(double((*px)[y]))-*(px+1)[y];
          else if(x==width-1)
            *px[y]=abs(double((*(pi-1))[y])-*pi[y]);
          else
            *px[y]=abs(double((*(pi+1))[y])-*(pi-1)[y])/2.0;
          if(y==0)
            *py[y]=abs(double((*(pi+width))[y])-*pi[y]);
          else if(y==height-1)
            *py[y]=abs(double((*pi)[y])-*(pi-width)[y]);
          else
            *py[y]=abs(double((*(pi+width))[y])-*(pi-width)[y])/2.0;
          if(z==0)
            *pz[y]=abs(double((*(pi+t))[y])-*(pi)[y]);
          else if(z==depth-1)
            *pz[y]=abs(double((*(pi))[y])-*(pi-t)[y]);
          else
            *pz[y]=abs(double((*(pi+t))[y])-*(pi-t)[y])/2.0;
        }
      }
    }
  }
  if(mag)
  {
    int sign=p.inverse?-1:1;
    uint8_t max=255;
    uint8_t inver_max=p.inverse?max:0;
    color_t* p_x=out_x,*p_y=out_y,*p_z=out_z,*p_o=mag;
    uint num=width*height*depth;
    const color_t* end_x=p_x+num;
    for(;p_x!=end_x;++p_x,++p_y,++p_z,++p_o)
    {
      for(int i=0;i<3;++i)
      {
        *p_o[i]=inver_max+sign*std::min(255.0,
              sqrt(static_cast<double>(*p_x[i]**p_x[i]+*p_y[i]**p_y[i]+*p_z[i]**p_z[i])));
      }
    }
  }
}






