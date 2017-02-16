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
#include "imgproc/zstackprocessor.h"

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




