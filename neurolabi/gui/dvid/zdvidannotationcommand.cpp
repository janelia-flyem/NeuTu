#include "zdvidannotationcommand.h"
#include "flyem/zflyemproofdoc.h"

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
