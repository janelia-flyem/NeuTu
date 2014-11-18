#ifndef ZFLYEMBODYMERGEDOC_H
#define ZFLYEMBODYMERGEDOC_H

#include "zstackdoc.h"
#include <QList>

#include "flyem/zflyembodymerger.h"
#include "zstackdoccommand.h"

class ZArray;

class ZFlyEmBodyMergeDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmBodyMergeDoc(ZStack *stack, QObject *parent = 0);
  ~ZFlyEmBodyMergeDoc();

public:
  void updateBodyObject();
  void mergeSelected();

  void setOriginalLabel(const ZStack *stack);

  ZFlyEmBodyMerger* getBodyMerger() {
    return &m_bodyMerger;
  }

private:
  QList<ZObject3dScan*> extractAllObject();

signals:

public slots:
  void updateOriginalLabel(ZArray *array);

private:
  ZArray *m_originalLabel;
  ZFlyEmBodyMerger m_bodyMerger;
};

namespace ZFlyEmBodyMergerDocCommand {
class MergeBody : public ZUndoCommand
{
public:
  MergeBody(ZStackDoc *doc, QUndoCommand *parent = NULL);
  void undo();
  void redo();

  ZFlyEmBodyMergeDoc* getCompleteDocument();

private:
  ZStackDoc *m_doc;
};
}

#endif // ZFLYEMBODYMERGEDOC_H
