#ifndef ZFLYEMBODYMERGEFRAME_H
#define ZFLYEMBODYMERGEFRAME_H

#include "zstackframe.h"

class ZFlyEmBodyMergeDoc;

class ZFlyEmBodyMergeFrame : public ZStackFrame
{
  Q_OBJECT
public:
  explicit ZFlyEmBodyMergeFrame(QWidget *parent = 0);

  void createDocument();

  ZFlyEmBodyMergeDoc *getCompleteDocument();


signals:

public slots:

};

#endif // ZFLYEMBODYMERGEFRAME_H
