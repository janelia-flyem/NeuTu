#include<QAction>
#include<QPushButton>
#include<QHBoxLayout>
#include<QMessageBox>
#include<QLineEdit>
#include<QLabel>
#include<vector>
#include<QIntValidator>
#include"zwatershedmodule.h"
#include"zstackwatershed.h"
#include"zstackdoc.h"
#include"zsandbox.h"
#include"mainwindow.h"
#include"zstackframe.h"
#include"zstackview.h"
#include "widgets/zimagewidget.h"


ZWaterShedGUI::ZWaterShedGUI(QWidget* parent):QWidget(parent)
{
  m_max_seed_index=10;
  m_cur_seed_index=1;
  initGui();
}


void ZWaterShedGUI::initGui()
{
  setWindowTitle("waterShed Segmentation");
  setWindowFlags(Qt::WindowStaysOnTopHint);
  m_lay=new QVBoxLayout;
  m_add_seed_btn=new QPushButton("startAddSeed");
  connect(m_add_seed_btn,SIGNAL(clicked()),this,SLOT(onAddSeed()));
  m_lay->addWidget(m_add_seed_btn);
  setLayout(m_lay);
  resize(300,100);
}


void ZWaterShedGUI::reset()
{
  m_max_seed_index=10;
  m_cur_seed_index=1;
  std::vector<QLineEdit*>::iterator it=m_line_edits.begin();
  for(;it!=m_line_edits.end();++it)
  {
    m_lay->removeWidget(*it);
    delete *it;
  }
  std::vector<QLabel*>::iterator ip=m_labels.begin();
  for(;ip!=m_labels.end();++ip)
  {
    m_lay->removeWidget(*ip);
    delete *ip;
  }
  m_line_edits.clear();
  m_labels.clear();
  m_add_seed_btn->setText("startAddSeed");

}

void ZWaterShedGUI::closeEvent(QCloseEvent* event)
{
  ZStackFrame* frame=ZSandbox::GetCurrentFrame();
  if(!frame)
  {
    return;
  }
  if(m_add_seed_btn->text()=="startWaterShed")
  {
    connect(frame->view()->imageWidget(),
            SIGNAL(mousePressed(QMouseEvent*)),
            ZSandbox::GetCurrentFrame()->view(),
            SLOT(mousePressedInImageWidget(QMouseEvent*)));
    connect(frame->view()->imageWidget(),
            SIGNAL(mouseReleased(QMouseEvent*)),
            ZSandbox::GetCurrentFrame()->view(),
            SLOT(mouseReleasedInImageWidget(QMouseEvent*)));
    disconnect(frame->view()->imageWidget(),
               SIGNAL(mousePressed(QMouseEvent*)),
               this,
                SLOT(onMousePressed(QMouseEvent*)));
  }
  reset();
  resize(300,100);
}

void ZWaterShedGUI::onMousePressed(QMouseEvent* event)
{

  if(m_cur_seed_index>m_max_seed_index)
  {
    return;
  }
  ZPoint pt=ZSandbox::GetCurrentFrame()->presenter()->getLastMousePosInStack();
  QHBoxLayout *lay=new QHBoxLayout;
  QLabel* label=new QLabel(QString("seed %1:").arg(m_cur_seed_index++));
  m_labels.push_back(label);
  lay->addWidget(label);
  QLineEdit *x[3];
  for(int i=0;i<3;++i)
  {
    x[i]=new QLineEdit;
    x[i]->setValidator(new QIntValidator());
    lay->addWidget(x[i]);
    m_line_edits.push_back(x[i]);
  }
  x[0]->setText(QString::number(static_cast<int>(0.5+pt.getX())));
  x[1]->setText(QString::number(static_cast<int>(0.5+pt.getY())));
  x[2]->setText(QString::number(static_cast<int>(0.5+pt.getZ())));
  m_lay->addLayout(lay);

}


void ZWaterShedGUI::onAddSeed()
{
  ZStackFrame* frame=ZSandbox::GetCurrentFrame();
  if(!frame)
  {
    return;
  }
  if(m_add_seed_btn->text()=="startAddSeed")
  {
    m_add_seed_btn->setText("startWaterShed");
    connect(frame->view()->imageWidget(),
            SIGNAL(mousePressed(QMouseEvent*)),
            this,
            SLOT(onMousePressed(QMouseEvent*)));
    disconnect(frame->view()->imageWidget(),
               SIGNAL(mousePressed(QMouseEvent*)),
               ZSandbox::GetCurrentFrame()->view(),
               SLOT(mousePressedInImageWidget(QMouseEvent*)));
    disconnect(frame->view()->imageWidget(),
               SIGNAL(mouseReleased(QMouseEvent*)),
               ZSandbox::GetCurrentFrame()->view(),
               SLOT(mouseReleasedInImageWidget(QMouseEvent*)));
  }
  else
  {
    m_add_seed_btn->setText("startAddSeed");
    disconnect(frame->view()->imageWidget(),
               SIGNAL(mousePressed(QMouseEvent*)),
               this,
               SLOT(onMousePressed(QMouseEvent*)));
    connect(frame->view()->imageWidget(),
            SIGNAL(mousePressed(QMouseEvent*)),
            ZSandbox::GetCurrentFrame()->view(),
            SLOT(mousePressedInImageWidget(QMouseEvent*)));
    connect(frame->view()->imageWidget(),
               SIGNAL(mouseReleased(QMouseEvent*)),
               ZSandbox::GetCurrentFrame()->view(),
               SLOT(mouseReleasedInImageWidget(QMouseEvent*)));
    //start Segment
    onStartSegment();
    reset();
    resize(300,100);
  }
}


void ZWaterShedGUI::onStartSegment()
{
  ZStackWatershed watershed;
  ZStackDoc *doc;
  std::vector<ZStack*> seeds;
  ZStack    *signal,*seed,*result;
  std::vector<QLineEdit*>::iterator it;
  bool ok[3];
  ZIntPoint p;
  uint i,seed_index;
  uint8_t* pdata;
  ZStackFrame *frame;
/*  Cuboid_I range;
  Cuboid_I_Set_S(&range,0,0,47,1024,1024,1);
  watershed.setRange(range);*/
  doc=ZSandbox::GetCurrentDoc();
  if((!doc) || !(signal=doc->getStack()))
  {
    return;
  }
  it=m_line_edits.begin();
  seed_index=1;
  for(;it!=m_line_edits.end();)
  {
    seed=new ZStack(GREY,1,1,1,1);
    seeds.push_back(seed);
    p.m_x=(*it++)->text().toInt(&ok[0]);
    p.m_y=(*it++)->text().toInt(&ok[1]);
    p.m_z=(*it++)->text().toInt(&ok[2]);
    if(!(ok[0]&&ok[1]&&ok[2]))
    {
      goto release;
    }
    seed->setOffset(p);
    seed->array8()[0]=seed_index++;
  }
  result=watershed.run(signal,seeds);
  if(result)
  {
    pdata=result->array8();
    for(i=0;i<result->getVoxelNumber();++i)
    {
      pdata[i]=pdata[i]*20;
    }
    frame =ZSandbox::GetMainWindow()->createStackFrame(result);
    ZSandbox::GetMainWindow()->addStackFrame(frame);
    ZSandbox::GetMainWindow()->presentStackFrame(frame);
  }
  else
  {
    QMessageBox::information(0,"error","no result created");
  }

release:
  //release memory
  for(i=0;i<seeds.size();++i)
  {
    delete seeds[i];
  }
}


ZWaterShedGUI::~ZWaterShedGUI()
{

}


ZWaterShedModule::ZWaterShedModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}


ZWaterShedModule::~ZWaterShedModule()
{
  if(m_gui)
  {
    delete m_gui;
  }
}


void ZWaterShedModule::init()
{
  m_gui=new ZWaterShedGUI();
  m_action = new QAction("WaterShedSegmentation", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}


void ZWaterShedModule::execute()
{
  m_gui->show();
}
