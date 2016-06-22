#include "zdvidannotationcommand.h"
#include "flyem/zflyemproofdoc.h"
#include "zstackdoccommand.h"

ZStackDocCommand::DvidAnnotationEdit::CompositeCommand::CompositeCommand(
    ZFlyEmProofDoc *doc, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_isExecuted(false)
{
}

ZStackDocCommand::DvidAnnotationEdit::CompositeCommand::~CompositeCommand()
{
  qDebug() << "Composite command (" << this->text() << ") destroyed";
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
  /*
  ZDvidReader reader;
  if (reader.open(m_doc->getDvidTarget())) {
    m_backup = reader.readAnnotationJson(m_item);
    m_doc->removeSynapse(m_synapse, ZDvidSynapseEnsemble::DATA_GLOBAL);
    m_doc->notifySynapseEdited(m_synapse);
    QString msg = QString("Synapse removed at (%1, %2, %3)").
        arg(m_synapse.getX()).arg(m_synapse.getY()).arg(m_synapse.getZ());
    m_doc->notify(msg);
  }
  */
}

void ZStackDocCommand::FlyEmToDoItemEdit::RemoveItem::undo()
{

}



