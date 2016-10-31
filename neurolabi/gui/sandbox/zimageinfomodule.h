#ifndef ZImageInfoModule_H
#define ZImageInfoModule_H
#include<QWidget>
#include"qcustomplot.h"
#include "zsandboxmodule.h"


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
    void showHistogram();
    void showBasicInfo();
  private:
    QWidget* m_paint_window;
    QCustomPlot *customPlot;
    QCPItemText *info;
    QCPBars *bars;
};

#endif // ZImageInfoModule_H
