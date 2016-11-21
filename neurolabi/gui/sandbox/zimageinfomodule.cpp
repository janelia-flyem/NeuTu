#include<QMessageBox>
#include <QAction>
#include<QGridLayout>
#include <QPainter>
#include <QPen>
#include<QString>
#include "zsandbox.h"
#include "zstackprocessor.h"
#include "zimageinfomodule.h"

ZImageShowWindow::ZImageShowWindow(QWidget *parent) : QWidget(parent)
{
  //init this window
  resize(1000,600);
  setWindowTitle("histogram of this image");
  move(200,60);
  //add a QLabel to show basic info of the image
  basic_info=new QLabel();
  //add y Axis options
  checkbox_y_log = new QCheckBox("yAxisLog",this);
  checkbox_y_log->setChecked(false);
  y_log=false;
  connect(checkbox_y_log, SIGNAL(clicked()), this, SLOT(checkChange()));
  //add a spinbox for choice of bins
  size_of_bin=1;
  spinbox=new QSpinBox(this);
  spinbox->setValue(size_of_bin);
  spinbox->setRange(1,256);
  connect(spinbox, SIGNAL(valueChanged(int)),this, SLOT(spinboxChange(int)));
  //init the plot object
  initPlot();
  //at last add a layout for this window
  QGridLayout* layout=new QGridLayout;
  layout->addWidget(customPlot,0,0,10,9);
  layout->addWidget(basic_info,0,9,5,1);
  layout->addWidget(checkbox_y_log,5,9,1,1);
  layout->addWidget(new QLabel("size of bin:"),7,9,1,1);
  layout->addWidget(spinbox,8,9,1,1);
  setLayout(layout);

}


void ZImageShowWindow::showHisogram()
{
  int t=0;
  double sum=0.0,average=0.0;
  int groups=ceil(256.0/size_of_bin);
  QVector<double> index(groups),values(groups);
  int i=0;
  double maxc=0;
  for(;i<groups;++i)
  {
    index[i]=i;
    t=0;
    for(int j=i*size_of_bin;j<(i+1)*size_of_bin&&j<256;++j)
    {
      t+=basic_hist[j];
    }
    sum+=t;
    t=y_log?log(t+1):t;//avoid the log(0) exception
    values[i] = t;
    maxc=t>maxc?t:maxc;
  }
  customPlot->yAxis->setLabel(y_log?"count/log":"count");
  for(int i=0;i<256;++i)
  {
    average+=i*basic_hist[i]/sum;
  }
  QVector<double>x(2),y(2);
  x[0]=x[1]=average/size_of_bin;
  y[0]=0,y[1]=maxc;
  customPlot->graph(0)->setData(x,y);
  bars->setData(index,values);
  QVector<double> coor;
  QVector<QString>labels;
  int step=ceil(16.0/size_of_bin);//we show at most 16 labels at x axis
  for(int i=0;i<groups;i+=step)
  {
    coor.append(i);
    labels.append(QString::number(i*size_of_bin));
  }
  coor.append(average/size_of_bin);
  labels.append(QString::number(ceil(average)));
  customPlot->xAxis->setTickVector(coor);
  customPlot->xAxis->setTickVectorLabels(labels);
  customPlot->rescaleAxes();
  customPlot->replot();
}

void ZImageShowWindow::updateInfo(ZStackDoc* _doc)
{
  doc=_doc;
  //show hisogram
  getBasicHist();
  showHisogram();
  //show basic information of the image
  showBasicInfo();
  //finally show the window
  show();
}


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
}


void ZImageInfoModule::init()
{
  m_paint_window=new ZImageShowWindow();
  m_action = new QAction("imageInfo", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}

void ZImageInfoModule::execute()
{
  ZStackDoc *doc = ZSandbox::GetCurrentDoc();
  if (doc != NULL)
  {
    m_paint_window->updateInfo(doc);
  }
}

void ZImageShowWindow::checkChange()
{
  if(sender()==checkbox_y_log)
  {
    y_log=checkbox_y_log->isChecked();
  }
  showHisogram();
  show();
}

void ZImageShowWindow::spinboxChange(int value)
{
  size_of_bin=value;
  showHisogram();
  show();
}

void ZImageShowWindow::showBasicInfo()
{
  QString  text="basic information:\n\n";
  int img_height=doc->getStackHeight();
  int img_width=doc->getStackWidth();
  int img_depth=doc->getStackDepth();
  text+="width :"+QString::number(img_width)+"\n\n";
  text+="height :"+QString::number(img_height)+"\n\n";
  text+="depth :"+QString::number(img_depth)+"\n\n";
  text+="total pixels :"+QString::number(img_width*img_height*img_depth);
  basic_info->setText(text);
}

void ZImageShowWindow::getBasicHist()
{
  basic_hist.clear();
  basic_hist.resize(256);
  ZIntHistogram* hist=C_Stack::hist(doc->getStack()->c_stack(),NULL);
  for(int i=0;i<256;++i)
  {
    basic_hist[i] = hist->getCount(i);
  }
  delete hist;
}

void ZImageShowWindow::initPlot()
{
  //init the qcustmPlot
  customPlot = new QCustomPlot(this);
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
  //init bars
  bars = new QCPBars(customPlot->xAxis, customPlot->yAxis);
  bars->setPen(Qt::NoPen);
  bars->setBrush(QColor(10, 140, 70, 160));
  customPlot->addPlottable(bars);
//  customPlot->addGraph();
  customPlot->addGraph();
  QPen pen;
  pen.setColor(QColor(255,170,100));
  pen.setWidth(2);
  pen.setStyle(Qt::DotLine);
  customPlot->graph(0)->setPen(pen);
  customPlot->graph(0)->setName("  average intensity");
  customPlot->legend->setVisible(true);
  customPlot->legend->setBrush(QBrush(QColor(255,255,255,150)));
}
