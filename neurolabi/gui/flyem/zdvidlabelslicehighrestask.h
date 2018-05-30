#ifndef ZDVIDLABELSLICEHIGHRESTASK_H
#define ZDVIDLABELSLICEHIGHRESTASK_H

#include "ztask.h"
#include "zstackviewparam.h"

class ZFlyEmProofMvc;

class ZDvidLabelSliceHighresTask : public ZTask
{
  Q_OBJECT
public:
  explicit ZDvidLabelSliceHighresTask(QObject *parent = nullptr);

  void execute();

signals:

public slots:

private:
  ZStackViewParam m_viewParam;
  int m_zoom = 0;
  int m_centerCutWidth = 0;
  int m_centerCutHeight = 0;
  ZFlyEmProofMvc *m_mvc = nullptr;
};

#endif // ZDVIDLABELSLICEHIGHRESTASK_H
