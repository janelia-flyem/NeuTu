#ifndef ZDVIDLABELSLICEHIGHRESTASK_H
#define ZDVIDLABELSLICEHIGHRESTASK_H

#include "dvid/zdviddataslicetask.h"
#include "zstackviewparam.h"

class ZStackDoc;

class ZDvidLabelSliceHighresTask : public ZDvidDataSliceTask
{
  Q_OBJECT
public:
  explicit ZDvidLabelSliceHighresTask(QObject *parent = nullptr);

  void execute();

signals:

public slots:
};

#endif // ZDVIDLABELSLICEHIGHRESTASK_H
