#ifndef ZImageInfoModule_H
#define ZImageInfoModule_H
#include<QWidget>
#include<QLabel>
#include<QSpinBox>
#include<QCheckBox>
#include"qcustomplot.h"
#include "mvc/zstackdoc.h"
#include "zsandboxmodule.h"

class ZImageShowWindow:public QWidget
{
  Q_OBJECT
  public:
    explicit ZImageShowWindow(QWidget *parent = 0);
  signals:
  public slots:
    void checkChange();
    void spinboxChange(int value);
  public:
    void updateInfo(ZStackDoc* _doc);
  private :
    void initPlot();
    void showBasicInfo();
    void showHistogram();
    void getBasicHist();
  private:
    ZStackDoc* doc;
    QCheckBox* checkbox_y_log;
    bool y_log;
    int size_of_bin;
    QSpinBox* spinbox;
    QCustomPlot *customPlot;
    QLabel *basic_info;
    QCPBars *bars;
    QVector<double> basic_hist;
};

class ZImageInfoModule : public ZSandboxModule
{
  Q_OBJECT
  public:
    explicit ZImageInfoModule(QObject *parent = 0);
    ~ZImageInfoModule();
  signals:
  public slots:
  private slots:
    void execute(); 
  private:
    void init();
  private:
    ZImageShowWindow* m_paint_window;
};

#endif // ZImageInfoModule_H
