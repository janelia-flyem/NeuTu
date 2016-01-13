#ifndef ZFLYEMBOOKMARKCOMMAND_H
#define ZFLYEMBOOKMARKCOMMAND_H

#include "zstackdoccommand.h"

#include "flyem/zflyembookmark.h"

class ZFlyEmProofDoc;
class ZFlyEmProofMvc;

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
  RemoveBookmark(ZFlyEmProofDoc *doc, ZFlyEmBookmark *bookmark,
                QUndoCommand *parent = NULL);
  virtual ~RemoveBookmark();
  void undo();
  void redo();

  bool hasRemoving() const {
    return !m_bookmarkArray.empty();
  }
  void addRemoving(ZFlyEmBookmark *bookmark);

private:
  ZFlyEmProofDoc *m_doc;
  std::vector<ZFlyEmBookmark*> m_bookmarkArray;
//  ZJsonObject m_backup;
  bool m_isInDoc;
};

class AddBookmark : public ZUndoCommand
{
public:
  AddBookmark(ZFlyEmProofDoc *doc, ZFlyEmBookmark *bookmark,
                QUndoCommand *parent = NULL);
  virtual ~AddBookmark();

  void addBookmark(ZFlyEmBookmark *bookmark);

  void undo();
  void redo();

private:
  ZFlyEmProofDoc *m_doc;
  std::vector<ZFlyEmBookmark*> m_bookmarkArray;
  bool m_isInDoc;
};

class ChangeBookmark : public ZUndoCommand
{
public:
  ChangeBookmark(ZFlyEmProofDoc *doc, ZFlyEmBookmark *bookmark,
                 const ZFlyEmBookmark &newBookmark,
                 QUndoCommand *parent = NULL);
  virtual ~ChangeBookmark();
  void undo();
  void redo();


private:
  ZFlyEmProofDoc *m_doc;
  ZFlyEmBookmark *m_bookmark;
  ZFlyEmBookmark m_newBookmark;
  ZFlyEmBookmark m_backup;
};

}

}

#endif // ZFLYEMBOOKMARKCOMMAND_H
