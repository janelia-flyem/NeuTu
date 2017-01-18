#include "zdvidannotationcommand.h"
#include "flyem/zflyemproofdoc.h"
#include "zstackdoccommand.h"
#include "neutubeconfig.h"

ZStackDocCommand::DvidAnnotationEdit::CompositeCommand::CompositeCommand(
    ZFlyEmProofDoc *doc, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_isExecuted(false)
{
}

ZStackDocCommand::DvidAnnotationEdit::CompositeCommand::~CompositeCommand()
{
  ZOUT(LTRACE(), 5) << "Composite command (" << this->text() << ") destroyed";
}

void ZStackDocCommand::DvidAnnotationEdit::CompositeCommand::redo()
{
//  m_doc->blockSignals(true);
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  QUndoCommand::redo();
  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

  m_isExecuted = true;
}


void ZStackDocCommand::DvidAnnotationEdit::CompositeCommand::undo()
{
//  m_doc->blockSignals(true);

  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  QUndoCommand::undo();
  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

  m_isExecuted = false;
}

/////////////////////////////////////
ZStackDocCommand::FlyEmToDoItemEdit::RemoveItem::RemoveItem(
    ZFlyEmProofDoc *doc, int x, int y, int z, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_doc = doc;
  m_item.set(x, y, z);
}

ZStackDocCommand::FlyEmToDoItemEdit::RemoveItem::~RemoveItem()
{
}

void ZStackDocCommand::FlyEmToDoItemEdit::RemoveItem::redo()
{
  ZDvidReader reader;
  if (reader.open(m_doc->getDvidTarget())) {
    m_backup = reader.readToDoItemJson(m_item);
    m_doc->removeTodoItem(m_item, ZFlyEmToDoList::DATA_GLOBAL);
    m_doc->notifyTodoEdited(m_item);
    QString msg = QString("Todo removed at (%1, %2, %3)").
        arg(m_item.getX()).arg(m_item.getY()).arg(m_item.getZ());
    m_doc->notify(msg);
  }
}

void ZStackDocCommand::FlyEmToDoItemEdit::RemoveItem::undo()
{
  if (m_backup.hasKey("Pos")) {
    ZFlyEmToDoItem item;
    item.loadJsonObject(m_backup, FlyEM::LOAD_PARTNER_RELJSON);
    m_doc->addTodoItem(item, ZFlyEmToDoList::DATA_GLOBAL);
    m_doc->notifyTodoEdited(item.getPosition());
    QString msg = QString("Todo removal undone at (%1, %2, %3)").
        arg(m_item.getX()).arg(m_item.getY()).arg(m_item.getZ());
    m_doc->notify(msg);
  }
}

/////////////////AddItem//////////////////////
ZStackDocCommand::FlyEmToDoItemEdit::AddItem::AddItem(
    ZFlyEmProofDoc *doc, const ZFlyEmToDoItem &item)
{
  m_doc = doc;
  m_item = item;
}

ZStackDocCommand::FlyEmToDoItemEdit::AddItem::~AddItem()
{
}

void ZStackDocCommand::FlyEmToDoItemEdit::AddItem::redo()
{
  m_doc->addTodoItem(m_item, ZFlyEmToDoList::DATA_GLOBAL);
  m_doc->notifyTodoEdited(m_item.getPosition());
  QString msg = QString("Todo item added at (%1, %2, %3)").
      arg(m_item.getX()).arg(m_item.getY()).arg(m_item.getZ());
  m_doc->notify(msg);
}

void ZStackDocCommand::FlyEmToDoItemEdit::AddItem::undo()
{
  m_doc->removeTodoItem(
        m_item.getPosition(), ZFlyEmToDoList::DATA_GLOBAL);
  m_doc->notifyTodoEdited(m_item.getPosition());
  QString msg = QString("New todo item removed by undo at (%1, %2, %3)").
      arg(m_item.getX()).arg(m_item.getY()).arg(m_item.getZ());
  m_doc->notify(msg);
}


