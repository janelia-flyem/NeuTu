#include "zflyembookmarkcommand.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyemproofmvc.h"
#include "zwidgetmessage.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "flyem/zflyembookmark.h"

ZStackDocCommand::FlyEmBookmarkEdit::CompositeCommand::CompositeCommand(
    ZFlyEmProofDoc *doc, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_isExecuted(false)
{
}

ZStackDocCommand::FlyEmBookmarkEdit::CompositeCommand::~CompositeCommand()
{
  qDebug() << "Composite command (" << this->text() << ") destroyed";
}

void ZStackDocCommand::FlyEmBookmarkEdit::CompositeCommand::redo()
{
//  m_doc->blockSignals(true);
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  QUndoCommand::redo();
  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

  m_isExecuted = true;
}

void ZStackDocCommand::FlyEmBookmarkEdit::CompositeCommand::undo()
{
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  QUndoCommand::undo();
  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

  m_isExecuted = false;
}

/////////////////////////////////////////////////
ZStackDocCommand::FlyEmBookmarkEdit::RemoveRemoteBookmark::RemoveRemoteBookmark(
    ZFlyEmProofDoc *doc, int x, int y, int z, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_doc = doc;
  m_location.set(x, y, z);
}

ZStackDocCommand::FlyEmBookmarkEdit::RemoveRemoteBookmark::~RemoveRemoteBookmark()
{
}

void ZStackDocCommand::FlyEmBookmarkEdit::RemoveRemoteBookmark::redo()
{
  ZDvidReader reader;
  if (reader.open(m_doc->getDvidTarget())) {
    m_backup = reader.readBookmarkJson(m_location);
    ZDvidWriter writer;
    if (writer.open(m_doc->getDvidTarget())) {
      writer.deleteBookmark(m_location);
    }
  }
}

void ZStackDocCommand::FlyEmBookmarkEdit::RemoveRemoteBookmark::undo()
{
  if (m_backup.hasKey("Pos")) {
    ZDvidWriter writer;
    if (writer.open(m_doc->getDvidTarget())) {
      writer.writeBookmark(m_backup);
    }
  }
}

//////////////////////////////////////////////////////////
ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark::RemoveBookmark(
    ZFlyEmProofMvc *frame, ZFlyEmBookmark *bookmark, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_frame = frame;
  m_bookmark = bookmark;
  m_isInDoc = true;
}

ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark::~RemoveBookmark()
{
  if (!m_isInDoc) {
    delete m_bookmark;
  }
}

void ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark::redo()
{
  if (m_bookmark != NULL) {
    ZDvidReader reader;
    ZFlyEmProofDoc *doc = m_frame->getCompleteDocument();
    if (reader.open(doc->getDvidTarget())) {
      m_backup = reader.readBookmarkJson(m_bookmark->getCenter().toIntPoint());
      ZDvidWriter writer;
      if (writer.open(doc->getDvidTarget())) {
        writer.deleteBookmark(m_bookmark->getCenter().toIntPoint());
      }

      m_frame->removeLocalBookmark(m_bookmark);
      m_isInDoc = false;
    }
  }
}

void ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark::undo()
{
  if (m_backup.hasKey("Pos")) {
    ZDvidWriter writer;
    ZFlyEmProofDoc *doc = m_frame->getCompleteDocument();
    if (writer.open(doc->getDvidTarget())) {
      writer.writeBookmark(m_backup);
      if (writer.isStatusOk()) {
        m_frame->addLocalBookmark(m_bookmark);
        m_isInDoc = true;
      } else {
        doc->notify(
              ZWidgetMessage("Failed to undo bookmark deletion", NeuTube::MSG_WARNING));
      }
    }
  }
}
