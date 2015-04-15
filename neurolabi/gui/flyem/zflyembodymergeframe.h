#ifndef ZFLYEMBODYMERGEFRAME_H
#define ZFLYEMBODYMERGEFRAME_H

#include "zstackframe.h"

class ZFlyEmBodyMergeDoc;
class ZDvidTarget;

class ZFlyEmBodyMergeFrame : public ZStackFrame
{
  Q_OBJECT
protected:
  explicit ZFlyEmBodyMergeFrame(QWidget *parent = 0);

public:
  void createDocument();
  void enableMessageManager();
  //void customizeWidget();

  static ZStackFrame* Make(QMdiArea *parent);
  static ZStackFrame* Make(
      QMdiArea *parent, ZSharedPointer<ZFlyEmBodyMergeDoc> doc);

  ZFlyEmBodyMergeDoc *getCompleteDocument();

  class MessageProcessor : public ZMessageProcessor {
  public:
    void processMessage(ZMessage *message, QWidget *host) const;
  };

  void setDvidTarget(const ZDvidTarget &target);

signals:

public slots:

};

#endif // ZFLYEMBODYMERGEFRAME_H
