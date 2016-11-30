#include <QAction>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
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
  select_strategy_window=new ZSelectGradientStrategyWindow;
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
  QVBoxLayout* lay=new QVBoxLayout(this);
  start_gradient_magnitude=new QPushButton("start");
  strategies=new QComboBox;
  strategies->addItem("normal");
  lay->addWidget(strategies);
  lay->addWidget(start_gradient_magnitude);
  this->setLayout(lay);
  this->resize(200,200);
  this->move(300,200);
  connect(start_gradient_magnitude,SIGNAL(clicked()),this,SLOT(onStart()));
  strategy_map["normal"]=GradientStrategyType::NORMAL;
}


void ZSelectGradientStrategyWindow::onStart()
{
  this->hide();
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

  ZStack* output_x=new ZStack(kind,width,height,depth,channel);
  ZStack* output_y=new ZStack(kind,width,height,depth,channel);
  ZStack* output_z=new ZStack(kind,width,height,depth,channel);
  ZStack* out=new ZStack(kind,width,height,depth,channel);

  QString s=strategies->currentText();
  GradientStrategyContext context(input_stack,output_x,output_y,output_z,strategy_map[s]);
  context.run();

  /*ZStackFrame *framex=ZSandbox::GetMainWindow()->createStackFrame(output_x);
  ZSandbox::GetMainWindow()->addStackFrame(framex);
  ZSandbox::GetMainWindow()->presentStackFrame(framex);

  ZStackFrame *framey=ZSandbox::GetMainWindow()->createStackFrame(output_y);
  ZSandbox::GetMainWindow()->addStackFrame(framey);
  ZSandbox::GetMainWindow()->presentStackFrame(framey);

  ZStackFrame *framez=ZSandbox::GetMainWindow()->createStackFrame(output_z);
  ZSandbox::GetMainWindow()->addStackFrame(framez);
  ZSandbox::GetMainWindow()->presentStackFrame(framez);*/

  GradientMagnitude<uint8>::run(output_x,output_y,output_z,out);
  ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(out);
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);

  delete output_x;
  delete output_y;
  delete output_z;
}


void GradientStrategyContext::run()
{
  for(int i=0;i<_in->channelNumber();++i)
  {
    switch(_in->kind())
    {
      case GREY:
          _run<uint8>(_in->array8(i),_outx->array8(i),_outy->array8(i),_outz->array8(i));
          break;
      case GREY16:
          _run<uint16>(_in->array16(i),_outx->array16(i),_outy->array16(i),_outz->array16(i));
          break;
      case FLOAT32:
          _run<float32>(_in->array32(i),_outx->array32(i),_outy->array32(i),_outz->array32(i));
          break;
      case FLOAT64:
          _run<float64>(_in->array64(i),_outx->array64(i),_outy->array64(i),_outz->array64(i));
          break;
      case COLOR:
          _run<gradient_color_t>
              (
                (gradient_color_t*)_in->array8(i),
                (gradient_color_t*)_outx->array8(i),
                (gradient_color_t*)_outy->array8(i),
                (gradient_color_t*)_outz->array8(i)
              );
          break;
      default:
          break;
    }
  }
}


template<typename T>
void GradientStrategyContext:: _run(const T* in,T* out_x,T* out_y,T* out_z)
{
  GradientStrategy<T>* strategy=0;
  switch(_type)
  {
    case GradientStrategyType::NORMAL:
        strategy=new GradientStrategyNormal<T>;
        break;
    default:
        strategy=0;
        break;
  }
  strategy->run(in,out_x,out_y,out_z,_in->width(),_in->height(),_in->depth());
}


template<typename T>
void GradientStrategyNormal<T>::run(const T* in,T* out_x,T* out_y,T* out_z,uint width,uint height,uint depth)
{
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
        if(x==0)
          *px=*(pi+1)>*pi ? *(pi+1)-*pi : *pi-*(pi+1);
        else if(x==width-1)
          *px=*(pi-1)>*pi ? *(pi-1)-*pi : *pi-*(pi-1);
        else
          *px=*(pi+1)>*(pi-1) ? (*(pi+1)-*(pi-1))/2.0 : (*(pi-1)-*(pi+1))/2.0;

        if(y==0)
          *py=*(pi+width)>*pi ? *(pi+width)-*pi : *pi-*(pi+width);
        else if(y==height-1)
          *py=*pi>*(pi-width) ? *pi-*(pi-width) : *(pi-width)-*pi;
        else
          *py=*(pi+width)>*(pi-width) ? (*(pi+width)-*(pi-width))/2.0 : (*(pi-width)-*(pi+width))/2.0;

        if(z==0)
          *pz=*(pi+t)>*(pi) ? *(pi+t)-*(pi) : *(pi)-*(pi+t);
        else if(z==depth-1)
          *pz=*(pi)>*(pi-t) ? *(pi)-*(pi-t) : *(pi-t)-*(pi);
        else
          *pz=*(pi+t)>*(pi-t) ? (*(pi+t)-*(pi-t))/2.0 : (*(pi-t)-*(pi+t))/2.0;
      }
    }
  }
}


template<typename T>
void GradientMagnitude<T>::run(const ZStack* in_x,const ZStack* in_y,const ZStack* in_z,ZStack* out)
{
  for(int c=0;c<in_x->channelNumber();++c)
  {
    uint num=in_x->getVoxelNumber();
    const T* p_x=(const T*)(void*)(in_x->array8(c));
    const T* end_x=p_x+num;
    const T* p_y=(const T*)(void*)(in_y->array8(c));
    const T* p_z=(const T*)(void*)(in_z->array8(c));
    T* p_o=(T*)(void*)(out->array8(c));

    T x,y,z;
    for(;p_x!=end_x;++p_x,++p_y,++p_z,++p_o)
    {
      x=*p_x;
      y=*p_y;
      z=*p_z;
      *p_o=sqrt(static_cast<double>(x*x+y*y+z*z));
    }
  }
}





