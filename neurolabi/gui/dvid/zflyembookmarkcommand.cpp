#include "zflyembookmarkcommand.h"

#include <algorithm>

#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyemproofmvc.h"
#include "zwidgetmessage.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"

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
    ZFlyEmProofDoc *doc, ZFlyEmBookmark *bookmark, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_doc = doc;
  addRemoving(bookmark);
  m_isInDoc = true;
}

ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark::~RemoveBookmark()
{
  if (!m_isInDoc) {
    for (std::vector<ZFlyEmBookmark*>::iterator iter = m_bookmarkArray.begin();
         iter != m_bookmarkArray.end(); ++iter) {
      delete *iter;
    }
  }
}

void ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark::addRemoving(
    ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    m_bookmarkArray.push_back(bookmark);
  }
}

void ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark::redo()
{
  if (!m_bookmarkArray.empty() && m_doc != NULL) {
    ZDvidReader reader;
    if (reader.open(m_doc->getDvidTarget())) {
//      m_backup = reader.readBookmarkJson(m_bookmark->getCenter().toIntPoint());
      ZDvidWriter writer;
      if (writer.open(m_doc->getDvidTarget())) {
        for (std::vector<ZFlyEmBookmark*>::iterator
             iter = m_bookmarkArray.begin(); iter != m_bookmarkArray.end();
             ++iter) {
          ZFlyEmBookmark *bookmark = *iter;
          writer.deleteBookmark(bookmark->getCenter().toIntPoint());
        }
      }

      m_doc->removeLocalBookmark(m_bookmarkArray);
      m_doc->notifyBookmarkEdited(m_bookmarkArray);
      m_isInDoc = false;
    }
  }
}

void ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark::undo()
{
  if (!m_bookmarkArray.empty() && m_doc != NULL) {
    ZDvidWriter writer;
    if (writer.open(m_doc->getDvidTarget())) {
      writer.writeBookmark(m_bookmarkArray);
      if (writer.isStatusOk()) {
        m_doc->addLocalBookmark(m_bookmarkArray);
        m_doc->notifyBookmarkEdited(m_bookmarkArray);
        m_isInDoc = true;
      } else {
        m_doc->notify(ZWidgetMessage("Failed to undo bookmark deletion",
                                     NeuTube::MSG_WARNING));
      }
    }
  }
}

////////////////////////////////////////////////////////////
ZStackDocCommand::FlyEmBookmarkEdit::AddBookmark::AddBookmark(
    ZFlyEmProofDoc *doc, ZFlyEmBookmark *bookmark, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_doc = doc;
  if (bookmark != NULL) {
    m_bookmarkArray.push_back(bookmark);
  }
  m_isInDoc = false;
}

ZStackDocCommand::FlyEmBookmarkEdit::AddBookmark::~AddBookmark()
{
  if (!m_isInDoc) {
    for (std::vector<ZFlyEmBookmark*>::iterator iter = m_bookmarkArray.begin();
         iter != m_bookmarkArray.end(); ++iter) {
      delete *iter;
    }
  }
}

void ZStackDocCommand::FlyEmBookmarkEdit::AddBookmark::addBookmark(
    ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    m_bookmarkArray.push_back(bookmark);
  }
}

void ZStackDocCommand::FlyEmBookmarkEdit::AddBookmark::redo()
{
  if (!m_bookmarkArray.empty() && m_doc != NULL) {
    ZDvidWriter writer;
    if (writer.open(m_doc->getDvidTarget())) {
      writer.writeBookmark(m_bookmarkArray);
      if (writer.isStatusOk()) {
        m_doc->addLocalBookmark(m_bookmarkArray);
        m_doc->notifyBookmarkEdited(m_bookmarkArray);
        m_isInDoc = true;
      } else {
        m_doc->notify(ZWidgetMessage("Failed to save bookmark to DVID",
                                     NeuTube::MSG_WARNING));
      }
    }
  }
}

void ZStackDocCommand::FlyEmBookmarkEdit::AddBookmark::undo()
{
  if (!m_bookmarkArray.empty() && m_doc != NULL) {
    ZDvidReader reader;
    if (reader.open(m_doc->getDvidTarget())) {
      ZDvidWriter writer;
      if (writer.open(m_doc->getDvidTarget())) {
        writer.deleteBookmark(m_bookmarkArray);
      }

      m_doc->removeLocalBookmark(m_bookmarkArray);
      m_doc->notifyBookmarkEdited(m_bookmarkArray);
      m_isInDoc = false;
    }
  }
}

////////////////////////////////////////////////////////////
ZStackDocCommand::FlyEmBookmarkEdit::ChangeBookmark::ChangeBookmark(
    ZFlyEmProofDoc *doc, ZFlyEmBookmark *bookmark,
    const ZFlyEmBookmark &newBookmark, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_doc = doc;
  m_bookmark = bookmark;
  m_newBookmark = newBookmark;
}

ZStackDocCommand::FlyEmBookmarkEdit::ChangeBookmark::~ChangeBookmark()
{
}

void ZStackDocCommand::FlyEmBookmarkEdit::ChangeBookmark::redo()
{
  if (m_bookmark != NULL && m_doc != NULL) {
    ZDvidWriter writer;
    if (writer.open(m_doc->getDvidTarget())) {
      writer.writeBookmark(m_newBookmark.toDvidAnnotationJson());
      if (writer.isStatusOk()) {
        m_backup = *m_bookmark;
        *m_bookmark = m_newBookmark;
        m_doc->updateLocalBookmark(m_bookmark);
        m_doc->notifyBookmarkEdited(m_bookmark);
      } else {
        m_doc->notify(ZWidgetMessage("Failed to save bookmark to DVID",
                                     NeuTube::MSG_WARNING));
      }
    }
  }
}

void ZStackDocCommand::FlyEmBookmarkEdit::ChangeBookmark::undo()
{
  if (m_bookmark != NULL && m_doc != NULL) {
    ZDvidWriter writer;
    if (writer.open(m_doc->getDvidTarget())) {
      writer.writeBookmark(m_backup.toDvidAnnotationJson());
      if (writer.isStatusOk()) {
        *m_bookmark = m_backup;
        m_doc->updateLocalBookmark(m_bookmark);
        m_doc->notifyBookmarkEdited(m_bookmark);
      } else {
        m_doc->notify(ZWidgetMessage("Failed to save bookmark to DVID",
                                     NeuTube::MSG_WARNING));
      }
    }
  }
}



