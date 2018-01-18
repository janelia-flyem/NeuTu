#ifndef ZSHOWSEGRESULT_H
#define ZSHOWSEGRESULT_H
#include "zsandboxmodule.h"
class ZShowSegResultModule : public ZSandboxModule
{
  Q_OBJECT
  public:
    explicit ZShowSegResultModule(QObject *parent = 0);
  ~ZShowSegResultModule(){};
  signals:
  public slots:
  private slots:
    void execute();
  private:
    void init();
};
#endif // ZSHOWSEGRESULT_H
