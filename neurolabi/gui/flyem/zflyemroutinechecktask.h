#ifndef ZFLYEMROUTINECHECKTASK_H
#define ZFLYEMROUTINECHECKTASK_H

#include "ztask.h"

class ZFlyEmProofDoc;

class ZFlyEmRoutineCheckTask : public ZTask
{
  Q_OBJECT
public:
  explicit ZFlyEmRoutineCheckTask(QObject *parent = nullptr);

  void setDoc(ZFlyEmProofDoc *doc);

  void execute();

signals:

public slots:

private:
  ZFlyEmProofDoc *m_doc = nullptr;
};

#endif // ZFLYEMROUTINECHECKTASK_H
