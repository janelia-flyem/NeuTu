#include "zdvidsynpasecommand.h"
#include "zintpoint.h"
#include "zdvidsynapse.h"
#include "zjsonobject.h"
#include "flyem/zflyemproofdoc.h"
#include "dvid/zdvidsynapseensenmble.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"
#include "zwidgetmessage.h"


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
  ZDvidReader reader;
  if (reader.open(m_doc->getDvidTarget())) {
    m_synapseBackup = reader.readSynapseJson(m_synapse);
    m_doc->removeSynapse(m_synapse, ZDvidSynapseEnsemble::DATA_GLOBAL);
    m_doc->notifySynapseEdited(m_synapse);
    QString msg = QString("Synapse removed at (%1, %2, %3)").
        arg(m_synapse.getX()).arg(m_synapse.getY()).arg(m_synapse.getZ());
    m_doc->notify(msg);
  }
  /*
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    ZDvidReader reader;
    if (reader.open(m_doc->getDvidTarget())) {
      m_synapseBackup = reader.readSynapseJson(m_synapse);
      se->removeSynapse(m_synapse, ZDvidSynapseEnsemble::DATA_GLOBAL);

    }
  }
  */
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapse::undo()
{
  if (m_synapseBackup.hasKey("Pos")) {
    ZDvidWriter writer;
    if (writer.open(m_doc->getDvidTarget())) {
      writer.writeSynapse(m_synapseBackup);
      if (writer.isStatusOk()) {
        ZDvidSynapse synapse;
        synapse.loadJsonObject(m_synapseBackup);
        m_doc->addSynapse(synapse, ZDvidSynapseEnsemble::DATA_LOCAL);
        m_doc->notifySynapseEdited(synapse);
        QString msg = QString("Synapse removal undone at (%1, %2, %3)").
            arg(m_synapse.getX()).arg(m_synapse.getY()).arg(m_synapse.getZ());
        m_doc->notify(msg);
      }
    }
  }
  /*
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
  */
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
  m_doc->addSynapse(m_synapse, ZDvidSynapseEnsemble::DATA_GLOBAL);
  m_doc->notifySynapseEdited(m_synapse);
  QString msg = QString("Synapse added at (%1, %2, %3)").
      arg(m_synapse.getX()).arg(m_synapse.getY()).arg(m_synapse.getZ());
  m_doc->notify(msg);
  /*
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    se->addSynapse(m_synapse, ZDvidSynapseEnsemble::DATA_GLOBAL);
    m_doc->processObjectModified(se);
    m_doc->notifyObjectModified();
  }
  */
}

void ZStackDocCommand::DvidSynapseEdit::AddSynapse::undo()
{
  m_doc->removeSynapse(
        m_synapse.getPosition(), ZDvidSynapseEnsemble::DATA_GLOBAL);
  m_doc->notifySynapseEdited(m_synapse);
  QString msg = QString("New synapse removed by undo at (%1, %2, %3)").
      arg(m_synapse.getX()).arg(m_synapse.getY()).arg(m_synapse.getZ());
  m_doc->notify(msg);
  /*
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    se->removeSynapse(
          m_synapse.getPosition(), ZDvidSynapseEnsemble::DATA_GLOBAL);
    m_doc->processObjectModified(se);
    m_doc->notifyObjectModified();
  }
  */
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
  m_doc->moveSynapse(m_from, m_to);
  m_doc->notifySynapseEdited(m_from);
  m_doc->notifySynapseEdited(m_to);
  QString msg = QString("Synapse moved from (%1, %2, %3) to (%1, %2, %3)").
      arg(m_from.getX()).arg(m_from.getY()).arg(m_from.getZ()).
      arg(m_to.getX()).arg(m_to.getY()).arg(m_to.getZ());
  m_doc->notify(msg);
  /*
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    se->moveSynapse(m_from, m_to);
    m_doc->processObjectModified(se);
    m_doc->notifyObjectModified();
  }
  */
}

void ZStackDocCommand::DvidSynapseEdit::MoveSynapse::undo()
{
  m_doc->moveSynapse(m_to, m_from);
  m_doc->notifySynapseEdited(m_from);
  m_doc->notifySynapseEdited(m_to);
  QString msg = QString("Synapse moving undone: (%1, %2, %3) <- (%1, %2, %3)").
      arg(m_from.getX()).arg(m_from.getY()).arg(m_from.getZ()).
      arg(m_to.getX()).arg(m_to.getY()).arg(m_to.getZ());
  m_doc->notify(msg);
  /*
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    se->moveSynapse(m_to, m_from);
    m_doc->processObjectModified(se);
    m_doc->notifyObjectModified();
  }
  */
}

////////////////////////////////
ZStackDocCommand::DvidSynapseEdit::LinkSynapse::LinkSynapse(
    ZFlyEmProofDoc *doc, const ZIntPoint &from, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_doc = doc;
  m_from = from;
}

ZStackDocCommand::DvidSynapseEdit::LinkSynapse::LinkSynapse(
    ZFlyEmProofDoc *doc, const ZIntPoint &from, const ZIntPoint &to,
    const std::string &relation, QUndoCommand *parent) : ZUndoCommand(parent)
{
  m_doc = doc;
  m_from = from;
  if (!relation.empty()) {
    m_relJson.append(ZDvidSynapse::MakeRelJson(to, relation));
//    std::cout << this << std::endl;
//    m_relJson.print();
  }
//  m_to = to;
//  m_relation = relation;
}

ZStackDocCommand::DvidSynapseEdit::LinkSynapse::~LinkSynapse()
{

}

void ZStackDocCommand::DvidSynapseEdit::LinkSynapse::addRelation(
    const ZIntPoint &to, const std::string &relation)
{
  ZDvidSynapse::AddRelation(m_relJson, to, relation);
//  std::cout << this << std::endl;
//  m_relJson.print();
}

void ZStackDocCommand::DvidSynapseEdit::LinkSynapse::redo()
{
  ZDvidReader reader;
  if (reader.open(m_doc->getDvidTarget())) {
    ZJsonObject synapseJson = reader.readSynapseJson(m_from);

    if (synapseJson.hasKey("Pos")) {
      ZDvidWriter writer;
      if (writer.open(m_doc->getDvidTarget())) {
        m_synapseBackup = synapseJson.clone();

        ZDvidSynapse::AddRelation(synapseJson, m_relJson);
        writer.writeSynapse(synapseJson);

        m_doc->updateSynapsePartner(m_from);

        QString msg = QString("Synaptic elements linked");
        m_doc->notify(msg);
      }
    }
  }
#if 0
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    ZDvidReader reader;
    if (reader.open(m_doc->getDvidTarget())) {
      ZJsonObject synapseJson = reader.readSynapseJson(m_from);

      if (synapseJson.hasKey("Pos")) {
        ZDvidWriter writer;
        if (writer.open(m_doc->getDvidTarget())) {
          m_synapseBackup = synapseJson.clone();

//          std::cout << this << std::endl;
//          m_relJson.print();
          ZDvidSynapse::AddRelation(synapseJson, m_relJson);
//          synapseJson.print();
          writer.writeSynapse(synapseJson);

          se->updatePartner(
                se->getSynapse(m_from, ZDvidSynapseEnsemble::DATA_LOCAL));
          m_doc->processObjectModified(se);
          m_doc->notifyObjectModified();
        }
      }
    }
  }
#endif
}

void ZStackDocCommand::DvidSynapseEdit::LinkSynapse::undo()
{
  if (!m_synapseBackup.isEmpty()) {
    ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble(NeuTube::Z_AXIS);
    if (se != NULL) {
      ZDvidWriter writer;
      if (writer.open(m_doc->getDvidTarget())) {
        writer.writeSynapse(m_synapseBackup);
        m_doc->updateSynapsePartner(m_from);
        QString msg = QString("Undo synaptic elements link.");
        m_doc->notify(msg);
      }
    }
  }

#if 0
  if (!m_synapseBackup.isEmpty()) {
    ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
    if (se != NULL) {
      ZDvidWriter writer;
      if (writer.open(m_doc->getDvidTarget())) {
        writer.writeSynapse(m_synapseBackup);
        se->updatePartner(
              se->getSynapse(m_from, ZDvidSynapseEnsemble::DATA_LOCAL));

        m_doc->processObjectModified(se);
        m_doc->notifyObjectModified();
      }
    }
  }
#endif
}

///////////////////////////////////////////
ZStackDocCommand::DvidSynapseEdit::UnlinkSynapse::UnlinkSynapse(
    ZFlyEmProofDoc *doc, const std::set<ZIntPoint> &ptSet, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_doc = doc;
  m_synapseSet = ptSet;
}

ZStackDocCommand::DvidSynapseEdit::UnlinkSynapse::~UnlinkSynapse()
{
}

void ZStackDocCommand::DvidSynapseEdit::UnlinkSynapse::redo()
{
  ZDvidReader reader;
  if (reader.open(m_doc->getDvidTarget())) {
    ZJsonArray synapseJsonArray = reader.readSynapseJson(
          m_synapseSet.begin(), m_synapseSet.end());

    m_synapseBackup.set(synapseJsonArray.clone());

    if (!synapseJsonArray.isEmpty()) {
      for (size_t i = 0; i < synapseJsonArray.size(); ++i) {
        ZJsonObject synapseJson(synapseJsonArray.value(i));
        for (std::set<ZIntPoint>::const_iterator iter = m_synapseSet.begin();
             iter != m_synapseSet.end(); ++iter) {
          const ZIntPoint &pt = *iter;
          ZDvidSynapse::RemoveRelation(synapseJson, pt);
        }

        ZDvidWriter writer;
        if (writer.open(m_doc->getDvidTarget())) {
          writer.writeSynapse(synapseJsonArray);
          m_doc->updateSynapsePartner(m_synapseSet);
        }
      }
      QString msg = QString("Synaptic elements unlinked");
      m_doc->notify(msg);
    }
  }


#if 0
  ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
  if (se != NULL) {
    ZDvidReader reader;
    if (reader.open(m_doc->getDvidTarget())) {
      ZJsonArray synapseJsonArray = reader.readSynapseJson(
            m_synapseSet.begin(), m_synapseSet.end());

      m_synapseBackup.set(synapseJsonArray.clone());

      if (!synapseJsonArray.isEmpty()) {
        for (size_t i = 0; i < synapseJsonArray.size(); ++i) {
          ZJsonObject synapseJson(synapseJsonArray.value(i));
          for (std::set<ZIntPoint>::const_iterator iter = m_synapseSet.begin();
               iter != m_synapseSet.end(); ++iter) {
            const ZIntPoint &pt = *iter;
            ZDvidSynapse::RemoveRelation(synapseJson, pt);
          }

          ZDvidWriter writer;
          if (writer.open(m_doc->getDvidTarget())) {
            writer.writeSynapse(synapseJsonArray);

            for (std::set<ZIntPoint>::const_iterator iter = m_synapseSet.begin();
                 iter != m_synapseSet.end(); ++iter) {
              const ZIntPoint &pt = *iter;
              se->updatePartner(
                    se->getSynapse(pt, ZDvidSynapseEnsemble::DATA_LOCAL));
            }
          }
        }
#ifdef _DEBUG_
        std::cout << synapseJsonArray.dumpString(2) << std::endl;
#endif

        m_doc->processObjectModified(se);
        m_doc->notifyObjectModified();
      }
    }
  }
#endif
}

void ZStackDocCommand::DvidSynapseEdit::UnlinkSynapse::undo()
{
  if (!m_synapseBackup.isEmpty()) {
    ZDvidWriter writer;
    if (writer.open(m_doc->getDvidTarget())) {
      writer.writeSynapse(m_synapseBackup);
      m_doc->updateSynapsePartner(m_synapseSet);
      QString msg = QString("Undo synaptic elements unlink.");
      m_doc->notify(msg);
    }
  }

#if 0
  if (!m_synapseBackup.isEmpty()) {
    ZDvidSynapseEnsemble *se = m_doc->getDvidSynapseEnsemble();
    if (se != NULL) {
      ZDvidWriter writer;
      if (writer.open(m_doc->getDvidTarget())) {
        writer.writeSynapse(m_synapseBackup);
        for (std::set<ZIntPoint>::const_iterator iter = m_synapseSet.begin();
             iter != m_synapseSet.end(); ++iter) {
          const ZIntPoint &pt = *iter;
          se->updatePartner(
                se->getSynapse(pt, ZDvidSynapseEnsemble::DATA_LOCAL));
        }

        m_doc->processObjectModified(se);
        m_doc->notifyObjectModified();
      }
    }
  }
#endif
}
