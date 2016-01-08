#ifndef ZFLYEMBOOKMARKCOMMAND_H
#define ZFLYEMBOOKMARKCOMMAND_H

#include "zstackdoccommand.h"

class ZFlyEmProofDoc;
class ZFlyEmProofMvc;
class ZFlyEmBookmark;

namespace ZStackDocCommand {
namespace FlyEmBookmarkEdit {

class CompositeCommand : public ZUndoCommand
{
public:
  CompositeCommand(ZFlyEmProofDoc *doc, QUndoCommand *parent = NULL);
  virtual ~CompositeCommand();

  void redo();
  void undo();

protected:
  ZFlyEmProofDoc *m_doc;
  bool m_isExecuted;
};

class RemoveRemoteBookmark : public ZUndoCommand
{
public:
  RemoveRemoteBookmark(ZFlyEmProofDoc *doc, int x, int y, int z,
                QUndoCommand *parent = NULL);
  virtual ~RemoveRemoteBookmark();
  void undo();
  void redo();

private:
  ZFlyEmProofDoc *m_doc;
  ZIntPoint m_location;
  ZJsonObject m_backup;
};


class RemoveBookmark : public ZUndoCommand
{
public:
  RemoveBookmark(ZFlyEmProofMvc *frame, ZFlyEmBookmark *bookmark,
                QUndoCommand *parent = NULL);
  virtual ~RemoveBookmark();
  void undo();
  void redo();

private:
  ZFlyEmProofMvc *m_frame;
  ZFlyEmBookmark *m_bookmark;
  ZJsonObject m_backup;
  bool m_isInDoc;
};

}

}

#endif // ZFLYEMBOOKMARKCOMMAND_H
