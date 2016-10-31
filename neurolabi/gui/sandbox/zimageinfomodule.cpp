#include<QMessageBox>
#include <QAction>
#include <QPainter>
#include <QPen>
#include<QString>
#include "zsandbox.h"
#include "zstackdoc.h"
#include "zstackprocessor.h"
#include "zimageinfomodule.h"


ZImageInfoModule::ZImageInfoModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}
ZImageInfoModule::~ZImageInfoModule()
{
  if(m_paint_window)
  {
    delete m_paint_window;
  }
//  if(customPlot)
//  {
//    delete customPlot;
//  }
//  if(info)
//  {
//    delete info;
//  }
//  if(bars)
//  {
//    delete bars;
//  }
}

void ZImageInfoModule::init()
{
  //init the show info window
  m_paint_window=new QWidget(NULL);
  m_paint_window->resize(800,600);
  m_paint_window->setWindowTitle("histogram of this image");
  m_paint_window->move(200,60);
  //init the qcustmPlot
  customPlot = new QCustomPlot(m_paint_window);
  customPlot->resize(800,600);
  customPlot->addLayer("abovemain", customPlot->layer("main"), QCustomPlot::limAbove);
  customPlot->addLayer("belowmain", customPlot->layer("main"), QCustomPlot::limBelow);
  customPlot->xAxis->grid()->setLayer("belowmain");
  customPlot->yAxis->grid()->setLayer("belowmain");
  customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
  customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
  customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
  customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
  customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
  customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
  customPlot->xAxis->setTickLabelColor(Qt::white);
  customPlot->yAxis->setTickLabelColor(Qt::white);
  customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  customPlot->xAxis->grid()->setSubGridVisible(true);
  customPlot->yAxis->grid()->setSubGridVisible(true);
  customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
  customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
  customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
  customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
  QLinearGradient plotGradient;
  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 350);
  plotGradient.setColorAt(0, QColor(80, 80, 80));
  plotGradient.setColorAt(1, QColor(50, 50, 50));
  customPlot->setBackground(plotGradient);
  QLinearGradient axisRectGradient;
  axisRectGradient.setStart(0, 0);
  axisRectGradient.setFinalStop(0, 350);
  axisRectGradient.setColorAt(0, QColor(80, 80, 80));
  axisRectGradient.setColorAt(1, QColor(30, 30, 30));
  customPlot->axisRect()->setBackground(axisRectGradient);
  customPlot->xAxis->setLabelColor(QColor(100,200,200));
  customPlot->yAxis->setLabelColor(QColor(100,200,200));
  customPlot->xAxis->setLabelFont(QFont("times",20));
  customPlot->yAxis->setLabelFont(QFont("times",20));
  customPlot->xAxis->setAutoTicks(false);
  customPlot->xAxis->setAutoTickLabels(false);
  customPlot->xAxis->setAutoTickStep(false);
  customPlot->xAxis->setLabel("value");
  customPlot->yAxis->setLabel("count");
  info = new QCPItemText(customPlot);
  customPlot->addItem(info);
  info->position->setType(QCPItemPosition::ptAxisRectRatio);
  info->position->setCoords(0.8,0.1);
  info->setColor(QColor(100,200,200));
  info->setFont(QFont("times",14));
  info->setTextAlignment(Qt::AlignLeft);
  //init bars
  bars = new QCPBars(customPlot->xAxis, customPlot->yAxis);
  bars->setPen(Qt::NoPen);
  bars->setBrush(QColor(10, 140, 70, 160));
  customPlot->addPlottable(bars);
  m_action = new QAction("imageInfo", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}
void ZImageInfoModule::showHistogram()
{
  ZStackDoc *doc = ZSandbox::GetCurrentDoc();
  ZIntHistogram* hist=C_Stack::hist(doc->getStack()->c_stack(),NULL);
  int minv=hist->getMinValue();
  int maxv=hist->getMaxValue();
  int t=0,groups=16,group_step=ceil((maxv-minv)/(double)groups);
  QVector<QString> labels(groups+1);
  QVector<double> index(groups),values(groups),coor(groups+1);
  double minc=MAX_INT32,maxc=0;
  int i=0;
  for(;i<groups;++i)
  {
    index[i]=i;
    t=0;
    for(int j=i*group_step;j<(i+1)*group_step;++j)
    {
      t+=hist->getCount(minv+j);
    }
    minc=t<minc?t:minc;
    maxc=t>maxc?t:maxc;
    values[i] = t;
    labels[i]=QString::number(minv+i*group_step+0.5*group_step);
  }
  labels[i]=QString::number(minv+i*group_step);
  bars->setData(index,values);
  customPlot->rescaleAxes();
  double upper=customPlot->xAxis->range().upper;
  double lower=customPlot->xAxis->range().lower;
  double wid=upper-lower;
  double width=bars->width();
  double cl=width+(1.0*wid-width*groups)/(groups-1);
  for(int i=0;i<groups+1;++i)
  {
    coor[i]=lower+i*cl+width/2.0;
  }
  customPlot->xAxis->setTickVector(coor);
  customPlot->xAxis->setTickVectorLabels(labels);
  customPlot->xAxis->setRange(lower,upper+1);
  customPlot->replot();
  m_paint_window->show();
  delete hist;
}

void ZImageInfoModule::showBasicInfo()
{
  ZStackDoc *doc = ZSandbox::GetCurrentDoc();
  QString  text="";
  int height=doc->getStackHeight();
  int width=doc->getStackWidth();
  int depth=doc->getStackDepth();
  text+="width :"+QString::number(width)+"\n";
  text+="height :"+QString::number(height)+"\n";
  text+="depth :"+QString::number(depth)+"\n";
  text+="total pixels :"+QString::number(width*height*depth);
  info->setText(text);
}

void ZImageInfoModule::execute()
{
  ZStackDoc *doc = ZSandbox::GetCurrentDoc();
  if (doc != NULL)
  {
    showBasicInfo();
    showHistogram();
  }
}

