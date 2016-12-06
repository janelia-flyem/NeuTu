#include <QAction>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include "zgradientmagnitudemodule.h"
#include "mainwindow.h"
#include "zsandbox.h"
#include "zstackprocessor.h"

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
  reverse=new QCheckBox("reverse");
  gaussin=new QDoubleSpinBox();
  gaussin->setMaximum(1.0);
  gaussin->setMinimum(0.0);
  gaussin->setSingleStep(0.1);
  edge_enhance=new QDoubleSpinBox();
  edge_enhance->setMaximum(1.0);
  edge_enhance->setMinimum(0.0);
  edge_enhance->setSingleStep(0.1);
  lay->addWidget(strategies);
  lay->addWidget(reverse);
  lay->addWidget(new QLabel("gaussinSmoothSigma:"));
  lay->addWidget(gaussin);
  lay->addWidget(new QLabel("edgeEnhancementAlpha:"));
  lay->addWidget(edge_enhance);
  lay->addWidget(start_gradient_magnitude);
  this->setLayout(lay);
  this->resize(200,100);
  this->move(300,200);
  connect(start_gradient_magnitude,SIGNAL(clicked()),this,SLOT(onStart()));
  strategy_map["simple"]=GradientStrategyContext::SIMPLE;
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

  ZStack* out=new ZStack(kind,width,height,depth,channel);
  QString s=strategies->currentText();
  GradientStrategyContext context(strategy_map[s]);
  context.run(input_stack,out,edge_enhance->value(),reverse->isChecked());
  double gaussin_sigma=gaussin->value();
  if(gaussin_sigma!=0.0)
  {
    Stack* p=ZStackProcessor::GaussianSmooth(out->c_stack(),gaussin_sigma,gaussin_sigma,gaussin_sigma);
    out->consume(p);
  }
  ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(out);
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);

}


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
void GradientStrategy<T>::run(const T* in,T* out,uint width,uint height,uint depth,bool reverse,double edge_enhance)
{
  _width=width;
  _height=height;
  _depth=depth;
  _slice=width*height;
  _total=_slice*depth;
  _run(in,out);
  if(edge_enhance!=0.0)
  {
    this->edgeEnhance(in,out,edge_enhance);
  }
  if(reverse)
  {
    this->reverse(out,out+_total);
  }
}


template<typename T>
void GradientStrategy<T>::edgeEnhance(const T* in,T* out,double alpha)
{
  for(size_t i=0;i<this->_total;++i)
  {
    out[i]=(1-alpha)*out[i]+in[i]*(alpha);
  }
}


template<>
void GradientStrategy<color_t>::edgeEnhance(const color_t* in,color_t* out,double alpha)
{
  for(size_t i=0;i<this->_total;++i)
  {
    for(uint j=0;j<3;++j)
    {
      out[i][j]=(1-alpha)*out[i][j]+in[i][j]*alpha;
    }
  }
}


template<typename T>
void GradientStrategy<T>::reverse(T* begin,T* end)
{
  for(T* it=begin;it!=end;++it)
    *it=_max-*it;
}


template<>
void GradientStrategy<color_t>::reverse(color_t* begin,color_t* end)
{
  for(color_t* it=begin;it!=end;++it)
    for(uint i=0;i<3;++i)
      (*it)[i]=_max-(*it)[i];
}


GradientStrategyContext::GradientStrategyContext(StrategyType strategy_type)
{
  _type=strategy_type;
}


GradientStrategyContext::~GradientStrategyContext()
{

}


void GradientStrategyContext::run(const ZStack* in,ZStack* out,double edge_enhance,bool reverse)
{
  if(!in || !out)
    return ;

  switch(in->kind())
  {
    case GREY:
          _run<uint8>(in,out,edge_enhance,reverse);
          break;
    case GREY16:
          _run<uint16>(in,out,edge_enhance,reverse);
          break;
    case FLOAT32:
          _run<float32>(in,out,edge_enhance,reverse);
          break;
    case FLOAT64:
          _run<float64>(in,out,edge_enhance,reverse);
          break;
    case COLOR:
          _run<color_t>(in,out,edge_enhance,reverse);
          break;
    default:
          break;
  }
}


template<typename T>
void GradientStrategyContext:: _run(const ZStack* in,ZStack* out,double edge_enhance,bool reverse)
{
  GradientStrategy<T>* strategy=getStrategy<T>();
  if(strategy)
  {
    /*process each channel*/
    for(int i=0;i<in->channelNumber();++i)
    {
      const T*_in=(const T*)in->array8(i);
      T* _out=(T*)out->array8(i);
      strategy->run(_in,_out,in->width(),in->height(),in->depth(),reverse,edge_enhance);
    }
    delete strategy;
  }
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

#define process(m,n,p,o) \
for(uint z=0;z<depth;++z)\
  for(uint y=0;y<height;++y)\
    for(uint x=0;x<width;++x,++pi,++p)\
      if(m==0)*p=abs(double(*pi)-*(pi+o));\
      else if(m==n)*p=abs(double(*(pi-o))-*pi);\
      else*p=abs(double(*(pi+o))-*(pi-o))/2.0;

  const T* pi=in;
  if(width>1)
  {
    process(x,width-1,_px,1);
  }
  pi=in;
  if(height>1)
  {
    process(y,height-1,_py,width);
  }
  pi=in;
  if(depth>1)
  {
    process(z,depth-1,_pz,slice);
  }
#undef process
  for(uint i=0;i<total;++i)
  {
    out[i]=std::min(sqrt(static_cast<double>(px[i]*px[i]+py[i]*py[i]+pz[i]*pz[i])),max);
  }
  delete[] px;
  delete[] py;
  delete[] pz;
}


template<>
void GradientStrategySimple<color_t>::_run(const color_t* in,color_t* out)
{
  uint width=this->_width,height=this->_height,depth=this->_depth;
  size_t slice=this->_slice,total=this->_total;
  double max=this->_max;

  color_t *px=new color_t[total],*py=new color_t[total],*pz=new color_t[total];
  color_t *_px=px,*_py=py,*_pz=pz;
  memset(px,0,sizeof(color_t)*total);
  memset(py,0,sizeof(color_t)*total);
  memset(pz,0,sizeof(color_t)*total);

#define process(m,n,p,o) \
for(uint z=0;z<depth;++z)\
  for(uint y=0;y<height;++y)\
    for(uint x=0;x<width;++x,++pi,++p)\
      for(uint t=0;t<3;++t)\
        if(m==0)(*p)[t]=abs(double((*pi)[t])-(*(pi+o))[t]);\
        else if(m==n)(*p)[t]=abs(double((*(pi-o))[t])-(*pi)[t]);\
        else(*p)[t]=abs(double((*(pi+o))[t])-(*(pi-o))[t])/2.0;\

  const color_t* pi=in;
  if(width>1)
  {
    process(x,width-1,_px,1);
  }
  if(height>1)
  {
    pi=in;
    process(y,height-1,_py,width);
  }
  if(depth>1)
  {
    pi=in;
    process(z,depth-1,_pz,slice);
  }
#undef process
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
  delete[] pz;
}






