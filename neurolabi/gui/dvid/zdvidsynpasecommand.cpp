#include "zdvidsynpasecommand.h"
#include "zintpoint.h"
#include "zdvidsynapse.h"
#include "zjsonobject.h"
#include "flyem/zflyemproofdoc.h"
#include "dvid/zdvidsynapseensenmble.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"


ZStackDocCommand::DvidSynapseEdit::CompositeCommand::CompositeCommand(
    ZFlyEmProofDoc *doc, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_isExecuted(false)
{
}

ZStackDocCommand::DvidSynapseEdit::CompositeCommand::~CompositeCommand()
{
  qDebug() << "Composite command (" << this->text() << ") destroyed";
}

void ZStackDocCommand::DvidSynapseEdit::CompositeCommand::redo()
{
//  m_doc->blockSignals(true);
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  QUndoCommand::redo();
  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

  m_isExecuted = true;
}


void ZStackDocCommand::DvidSynapseEdit::CompositeCommand::undo()
{
//  m_doc->blockSignals(true);

  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  QUndoCommand::undo();
  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

  m_isExecuted = false;
}

ZStackDocCommand::DvidSynapseEdit::RemoveSynapse::RemoveSynapse(
    ZFlyEmProofDoc *doc, int x, int y, int z, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_doc = doc;
  m_synapse.set(x, y, z);
}

ZStackDocCommand::DvidSynapseEdit::RemoveSynapse::~RemoveSynapse()
{

}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapse::redo()
{
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    ZDvidReader reader;
    if (reader.open(m_doc->getDvidTarget())) {
      m_synapseBackup = reader.readSynapseJson(m_synapse);
      se->removeSynapse(m_synapse, ZDvidSynapseEnsemble::DATA_GLOBAL);
      m_doc->processObjectModified(se);
      m_doc->notifyObjectModified();
    }
  }
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapse::undo()
{
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    if (m_synapseBackup.hasKey("Pos")) {
      ZDvidWriter writer;
      if (writer.open(m_doc->getDvidTarget())) {
        writer.writeSynapse(m_synapseBackup);
        if (writer.isStatusOk()) {
          ZDvidSynapse synapse;
          synapse.loadJsonObject(m_synapseBackup);
          se->addSynapse(synapse, ZDvidSynapseEnsemble::DATA_LOCAL);
          m_doc->processObjectModified(se);
          m_doc->notifyObjectModified();
        }
      }
    }
  }
}


///////////////////////////////////////
ZStackDocCommand::DvidSynapseEdit::AddSynapse::AddSynapse(
    ZFlyEmProofDoc *doc, const ZDvidSynapse &synapse, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_doc = doc;
  m_synapse = synapse;
}

ZStackDocCommand::DvidSynapseEdit::AddSynapse::~AddSynapse()
{
}

void ZStackDocCommand::DvidSynapseEdit::AddSynapse::redo()
{
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    se->addSynapse(m_synapse, ZDvidSynapseEnsemble::DATA_GLOBAL);
    m_doc->processObjectModified(se);
    m_doc->notifyObjectModified();
  }
}

void ZStackDocCommand::DvidSynapseEdit::AddSynapse::undo()
{
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    se->removeSynapse(
          m_synapse.getPosition(), ZDvidSynapseEnsemble::DATA_GLOBAL);
    m_doc->processObjectModified(se);
    m_doc->notifyObjectModified();
  }
}

/////////////////////////////////////////////////////
ZStackDocCommand::DvidSynapseEdit::MoveSynapse::MoveSynapse(
    ZFlyEmProofDoc *doc, const ZIntPoint &from, const ZIntPoint &to,
    QUndoCommand *parent) : ZUndoCommand(parent)
{
  m_doc = doc;
  m_from = from;
  m_to = to;
}

ZStackDocCommand::DvidSynapseEdit::MoveSynapse::~MoveSynapse()
{

}

void ZStackDocCommand::DvidSynapseEdit::MoveSynapse::redo()
{
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    se->moveSynapse(m_from, m_to);
    m_doc->processObjectModified(se);
    m_doc->notifyObjectModified();
  }
}

void ZStackDocCommand::DvidSynapseEdit::MoveSynapse::undo()
{
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    se->moveSynapse(m_to, m_from);
    m_doc->processObjectModified(se);
    m_doc->notifyObjectModified();
  }
}



