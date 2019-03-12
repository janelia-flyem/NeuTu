#ifndef ZDVIDGRAYSLICEHIGHRESTASK_H
#define ZDVIDGRAYSLICEHIGHRESTASK_H

#include "dvid/zdviddataslicetask.h"
#include "zstackviewparam.h"

class ZStackDoc;

class ZDvidGraySliceHighresTask : public ZDvidDataSliceTask
{
  Q_OBJECT
public:
  explicit ZDvidGraySliceHighresTask(QObject *parent = nullptr);

  void execute();

signals:

public slots:
};

#endif // ZDVIDGRAYSLICEHIGHRESTASK_H
