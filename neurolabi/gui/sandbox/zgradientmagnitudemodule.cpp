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

  delete select_strategy_window;
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
  this->setWindowTitle("Image Gradient");

  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::WindowStaysOnTopHint;
  this->setWindowFlags(flags);
  QVBoxLayout* lay=new QVBoxLayout(this);
  start_gradient_magnitude=new QPushButton("start");
  QPushButton* reset=new QPushButton("reset");
  strategies=new QComboBox;
  strategies->addItem("simple");
  reverse=new QCheckBox("inverse gradient");

  gaussin_use_same_sigma=new QCheckBox("XYZ use same standard deviation sigma");
  gaussin_sigma_x=new QDoubleSpinBox();
  gaussin_sigma_y=new QDoubleSpinBox();
  gaussin_sigma_z=new QDoubleSpinBox();
  gaussin_sigma_x->setMaximum(3.0);
  gaussin_sigma_x->setMinimum(0.0);
  gaussin_sigma_x->setSingleStep(0.1);
  gaussin_sigma_y->setMaximum(3.0);
  gaussin_sigma_y->setMinimum(0.0);
  gaussin_sigma_y->setSingleStep(0.1);
  gaussin_sigma_z->setMaximum(3.0);
  gaussin_sigma_z->setMinimum(0.0);
  gaussin_sigma_z->setSingleStep(0.1);

  edge_enhance=new QDoubleSpinBox();
  edge_enhance->setToolTip(
    "g=alpha*F+(1-alpha)*E \n where F is original value ,E is gradient and g is new value"
    );
  edge_enhance->setMaximum(1.0);
  edge_enhance->setMinimum(0.0);
  edge_enhance->setSingleStep(0.1);

  QHBoxLayout* lay_s=new QHBoxLayout;
  lay_s->addWidget(new QLabel("gradient algorithm:"));
  lay_s->addWidget(strategies);
  lay->addLayout(lay_s);
  lay->addSpacing(15);

  QHBoxLayout* lay_g=new QHBoxLayout;
  lay_g->addWidget(new QLabel("gaussin smooth:"));
  lay_g->addWidget(gaussin_use_same_sigma);
  QHBoxLayout* lay_gs=new QHBoxLayout;
  lay_gs->addWidget(new QLabel("sigma x"));
  lay_gs->addWidget(gaussin_sigma_x);
  lay_gs->addWidget(new QLabel("sigma y"));
  lay_gs->addWidget(gaussin_sigma_y);
  lay_gs->addWidget(new QLabel("sigma z"));
  lay_gs->addWidget(gaussin_sigma_z);
  lay->addLayout(lay_g);
  lay->addLayout(lay_gs);
  lay->addSpacing(15);

  lay->addWidget(reverse);
  lay->addStretch();

  QHBoxLayout* lay_e=new QHBoxLayout;
  lay_e->addWidget(new QLabel("edge enhancement alpha:"));
  lay_e->addWidget(edge_enhance);
  lay->addLayout(lay_e);
  lay->addSpacing(15);

  QHBoxLayout* lay_st=new QHBoxLayout;
  lay_st->addWidget(reset);
  lay_st->addWidget(start_gradient_magnitude);
  lay->addLayout(lay_st);

  this->setLayout(lay);
  this->move(300,200);
  connect(reset,SIGNAL(clicked()),this,SLOT(onReset()));
  connect(start_gradient_magnitude,SIGNAL(clicked()),this,SLOT(onStart()));
  connect(gaussin_use_same_sigma,SIGNAL(stateChanged(int)),this,SLOT(onUseSameSigmaChanged(int)));
  strategy_map["simple"]=GradientStrategyContext::SIMPLE;
}


void ZSelectGradientStrategyWindow::onReset()
{
  strategies->setCurrentIndex(0);
  gaussin_use_same_sigma->setChecked(false);
  gaussin_sigma_x->setValue(0.0);
  gaussin_sigma_y->setValue(0.0);
  gaussin_sigma_z->setValue(0.0);
  reverse->setChecked(false);
  edge_enhance->setValue(0.0);
}


void ZSelectGradientStrategyWindow::onUseSameSigmaChanged(int state)
{
  if(state==Qt::Checked)
  {
    gaussin_sigma_y->setEnabled(false);
    gaussin_sigma_z->setEnabled(false);
  }
  else
  {
    gaussin_sigma_y->setEnabled(true);
    gaussin_sigma_z->setEnabled(true);
  }
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

  double sigma_x,sigma_y,sigma_z;
  sigma_x=gaussin_sigma_x->value();
  if(gaussin_use_same_sigma->isChecked())
  {
    sigma_y=sigma_z=sigma_x;
  }
  else
  {
    sigma_y=gaussin_sigma_y->value();
    sigma_z=gaussin_sigma_z->value();
  }

  QString s=strategies->currentText();
  GradientStrategyContext context(strategy_map[s]);
  context.run(input_stack,out,reverse->isChecked(),
              edge_enhance->value(),sigma_x,sigma_y,sigma_z);

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
void GradientStrategy<T>::run(const T* in,T* out,uint width,uint height,uint depth)
{
  _width=width;
  _height=height;
  _depth=depth;
  _slice=_width*_height;
  _total=_slice*_depth;
  _run(in,out);
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


void GradientStrategyContext::run
(
    const ZStack* in,
    ZStack* out,
    bool reverse,
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
  gaussin_smooth_sigma_y,gaussin_smooth_sigma_z,reverse)
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
    bool reverse)
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
      strategy->run(_in,_out,in->width(),in->height(),in->depth());
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
void GradientStrategySimple<T>::process(uint& x,uint&y ,uint&z,uint& w,const T* pi,T* p,uint offset,uint end)
{

  for(z=0;z<this->_depth;++z)
    for(y=0;y<this->_height;++y)
      for(x=0;x<this->_width;++x,++pi,++p)
      {
        if(w==0)*p=std::abs(double(*pi)-*(pi+offset));
        else if(w==end)*p=std::abs(double(*(pi-offset))-*pi);
        else*p=std::abs(double(*(pi+offset))-*(pi-offset))/2.0;
      }
}


template<>
void GradientStrategySimple<color_t>::process(uint& x,uint&y ,uint&z,uint& w,const color_t* pi,color_t* p,uint offset,uint end)
{
  for(z=0;z<this->_depth;++z)
    for(y=0;y<this->_height;++y)
      for(x=0;x<this->_width;++x,++pi,++p)
        for(uint t=0;t<3;++t)
        {
          if(w==0)(*p)[t]=std::abs(double((*pi)[t])-(*(pi+offset))[t]);
          else if(w==end)(*p)[t]=std::abs(double((*(pi-offset))[t])-(*pi)[t]);
          else(*p)[t]=std::abs(double((*(pi+offset))[t])-(*(pi-offset))[t])/2.0;
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
  delete[] pz;
}






