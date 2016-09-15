#include "zdvidsynapsecommand.h"
#include "zintpoint.h"
#include "zdvidsynapse.h"
#include "zjsonobject.h"
#include "flyem/zflyemproofdoc.h"
#include "dvid/zdvidsynapseensenmble.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"
#include "zwidgetmessage.h"
#include "neutubeconfig.h"

ZStackDocCommand::DvidSynapseEdit::CompositeCommand::CompositeCommand(
    ZFlyEmProofDoc *doc, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_isExecuted(false)
{
}

ZStackDocCommand::DvidSynapseEdit::CompositeCommand::~CompositeCommand()
{
  ZOUT(LTRACE(), 5)<< "Composite command (" << this->text() << ") destroyed";
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

//////////////////////////////////
ZStackDocCommand::DvidSynapseEdit::RemoveSynapseOp::RemoveSynapseOp(
    ZFlyEmProofDoc *doc, QUndoCommand *parent) : ZUndoCommand(parent)
{
  m_doc = doc;
}

ZStackDocCommand::DvidSynapseEdit::RemoveSynapseOp::~RemoveSynapseOp()
{

}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapseOp::addRemoval(
    const ZIntPoint &pt)
{
  m_synapseSet.insert(pt);

  ZDvidReader &reader = m_doc->getDvidReader();
  if (reader.isReady()) {
    ZDvidSynapse synapse =
        reader.readSynapse(pt, FlyEM::LOAD_PARTNER_LOCATION);
    if (synapse.getKind() == ZDvidAnnotation::KIND_PRE_SYN) {
      synapse.updatePartnerProperty(reader);
      std::vector<ZIntPoint> partnerArray = synapse.getPartners();
      for (size_t i = 0; i < partnerArray.size(); ++i) {
        const ZIntPoint &pt = partnerArray[i];
        if (synapse.getParterKind(i) == ZDvidAnnotation::KIND_POST_SYN) {
          m_synapseSet.insert(pt);
        }
      }
    }
  }
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapseOp::addRemoval(
    const QList<ZIntPoint> &ptArray)
{
  foreach (const ZIntPoint &pt, ptArray) {
    addRemoval(pt);
  }
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapseOp::backupSynapse()
{
  if (m_doc != NULL) {
    ZDvidReader &reader = m_doc->getDvidReader();
    if (reader.isReady()) {
      std::set<ZIntPoint> synapseSet;
      synapseSet.insert(m_synapseSet.begin(), m_synapseSet.end());

      ZJsonArray synapseArray = reader.readSynapseJson(m_synapseSet.begin(),
                                                       m_synapseSet.end());
      for (size_t i = 0; i < synapseArray.size(); ++i) {
        ZJsonObject synapseJson(synapseArray.value(i));
        std::vector<ZIntPoint> partnerArray =
            ZDvidAnnotation::GetPartners(synapseJson);
        synapseSet.insert(partnerArray.begin(), partnerArray.end());
      }

      m_synapseBackup = reader.readSynapseJson(
            synapseSet.begin(), synapseSet.end());
    }
  }
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapseOp::redo()
{
  if (!m_synapseSet.empty()) {
    ZDvidReader &reader = m_doc->getDvidReader();
//    ZDvidWriter &writer = m_doc->getDvidWriter();

    if (reader.isReady()) {
      backupSynapse();

      QString synapseString;
      for (std::set<ZIntPoint>::const_iterator iter = m_synapseSet.begin();
           iter != m_synapseSet.end(); ++iter) {
        const ZIntPoint &pt = *iter;
        m_doc->removeSynapse(pt, ZDvidSynapseEnsemble::DATA_GLOBAL);
        m_doc->notifySynapseEdited(pt);
        synapseString.append(pt.toString().c_str());
      }
#if 0

      for (size_t i = 0; i < m_synapseBackup.size(); ++i) {
        bool removed = false;
        ZJsonObject synapseJson(m_synapseBackup.value(i).clone());
        ZIntPoint currentPos = ZDvidAnnotation::GetPosition(synapseJson);
        if (m_synapseSet.count(currentPos) == 0) { //related synapses
          for (std::set<ZIntPoint>::const_iterator iter = m_synapseSet.begin();
               iter != m_synapseSet.end(); ++iter) {
            const ZIntPoint &pt = *iter;
            if (pt != currentPos) {
              removed =
                  ZDvidSynapse::RemoveRelation(synapseJson, pt) || removed;
            }
          }
          if (removed) {
            writer.writeSynapse(synapseJson);
          }
          m_doc->updateSynapsePartner(currentPos);
        } else { //host synapses
          m_doc->removeSynapse(currentPos, ZDvidSynapseEnsemble::DATA_GLOBAL);
        }
      }
#endif
      QString msg = QString("Synapse removed: %1").arg(synapseString);
      ZWidgetMessage message(
            msg, NeuTube::MSG_INFORMATION, ZWidgetMessage::TARGET_TEXT_APPENDING);
      m_doc->notify(message);
    }
  }
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapseOp::undo()
{
  if (!m_synapseBackup.isEmpty()) {
    ZDvidWriter &writer = m_doc->getDvidWriter();
    writer.writeSynapse(m_synapseBackup);

    for (size_t i = 0; i < m_synapseBackup.size(); ++i) {
      ZJsonObject synapseJson(m_synapseBackup.value(i));
      ZIntPoint currentPos = ZDvidAnnotation::GetPosition(synapseJson);
      if (m_synapseSet.count(currentPos) == 0) { //related synapses
        m_doc->updateSynapsePartner(ZDvidAnnotation::GetPosition(synapseJson));
      } else {
        m_doc->addSynapse(currentPos, ZDvidAnnotation::GetKind(synapseJson),
                          ZDvidSynapseEnsemble::DATA_LOCAL);
      }
      m_doc->notifySynapseEdited(currentPos);
    }

    QString msg = QString("Synapses grouped back.");
    m_doc->notify(msg);
  }
}

/////////////////////////////////////////////
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

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapse::backup()
{
  ZDvidReader &reader = m_doc->getDvidReader();

  m_synapseBackup =reader.readSynapseJson(m_synapse);
  m_partnerBackup.clear();
  std::vector<ZIntPoint> partnerArray =
      ZDvidAnnotation::GetPartners(m_synapseBackup);
  for (std::vector<ZIntPoint>::const_iterator iter = partnerArray.begin();
       iter != partnerArray.end(); ++iter) {
    const ZIntPoint &pt = *iter;
    m_partnerBackup.push_back(reader.readSynapseJson(pt));
  }
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapse::redo()
{
  if (m_doc->getDvidReader().good()) {
    backup();
    m_doc->removeSynapse(m_synapse, ZDvidSynapseEnsemble::DATA_GLOBAL);
    m_doc->notifySynapseEdited(m_synapse);
    QString msg = QString("Synapse removed at (%1, %2, %3)").
        arg(m_synapse.getX()).arg(m_synapse.getY()).arg(m_synapse.getZ());
    m_doc->notify(msg);
  } else {
    m_doc->notify(ZWidgetMessage("Invalid DVID reader", NeuTube::MSG_ERROR));
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
    ZDvidWriter &writer = m_doc->getDvidWriter();
    if (writer.good()) {
      writer.writeSynapse(m_synapseBackup);
      for (std::vector<ZJsonObject>::const_iterator
           iter = m_partnerBackup.begin(); iter != m_partnerBackup.end();
           ++iter) {
        writer.writeSynapse(*iter);
      }

      if (writer.isStatusOk()) {
        ZDvidSynapse synapse;
        synapse.loadJsonObject(
              m_synapseBackup, FlyEM::LOAD_PARTNER_LOCATION);
        m_doc->addSynapse(synapse, ZDvidSynapseEnsemble::DATA_LOCAL);
        m_doc->notifySynapseEdited(synapse);
        QString msg = QString("Synapse removal undone at (%1, %2, %3)").
            arg(m_synapse.getX()).arg(m_synapse.getY()).arg(m_synapse.getZ());
        m_doc->notify(msg);
      }
    } else {
      m_doc->notify(ZWidgetMessage("Invalid DVID writer", NeuTube::MSG_ERROR));
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

////////////////////////////////////////////////////
ZStackDocCommand::DvidSynapseEdit::RemoveSynapses::RemoveSynapses(
    ZFlyEmProofDoc *doc, QUndoCommand *parent) :
  ZUndoCommand(parent)
{
  m_doc = doc;
}

ZStackDocCommand::DvidSynapseEdit::RemoveSynapses::~RemoveSynapses()
{
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapses::addRemoval(
    int x, int y, int z)
{
  m_synapse.insert(ZIntPoint(x, y, z));
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapses::setRemoval(
    const std::set<ZIntPoint> &removal)
{
  m_synapse = removal;
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapses::backup()
{
  ZDvidReader &reader = m_doc->getDvidReader();

  m_partnerBackup.clear();

  std::set<ZIntPoint> partnerSet;

  for (std::set<ZIntPoint>::const_iterator iter = m_synapse.begin();
       iter != m_synapse.end(); ++iter) {
    const ZIntPoint &pos = *iter;
    ZJsonObject synapseJson = reader.readSynapseJson(pos);
    m_synapseBackup.push_back(synapseJson);
    std::vector<ZIntPoint> partnerArray =
        ZDvidAnnotation::GetPartners(synapseJson);
    partnerSet.insert(partnerArray.begin(), partnerArray.end());
  }

  //Backup partners
  for (std::set<ZIntPoint>::const_iterator iter = partnerSet.begin();
       iter != partnerSet.end(); ++iter) {
    const ZIntPoint &pt = *iter;
    m_partnerBackup.push_back(reader.readSynapseJson(pt));
  }
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapses::redo()
{
  if (m_doc->getDvidReader().good()) {
    backup();
    for (std::set<ZIntPoint>::const_iterator iter = m_synapse.begin();
         iter != m_synapse.end(); ++iter) {
      const ZIntPoint &synapse = *iter;
      m_doc->removeSynapse(synapse, ZDvidSynapseEnsemble::DATA_GLOBAL);
      m_doc->notifySynapseEdited(synapse);
      QString msg = QString("Synapse removed at (%1, %2, %3)").
          arg(synapse.getX()).arg(synapse.getY()).arg(synapse.getZ());
      m_doc->notify(msg);
    }
  } else {
    m_doc->notify(ZWidgetMessage("Invalid DVID reader", NeuTube::MSG_ERROR));
  }
}

void ZStackDocCommand::DvidSynapseEdit::RemoveSynapses::undo()
{
  ZDvidWriter &writer = m_doc->getDvidWriter();
  if (writer.good()) {
    for (std::vector<ZJsonObject>::const_iterator
         iter = m_synapseBackup.begin(); iter != m_synapseBackup.end();
         ++iter) {
      writer.writeSynapse(*iter);
    }

    for (std::vector<ZJsonObject>::const_iterator
         iter = m_partnerBackup.begin(); iter != m_partnerBackup.end();
         ++iter) {
      writer.writeSynapse(*iter);
    }

    if (writer.isStatusOk()) {
      ZDvidSynapse synapse;
      for (std::vector<ZJsonObject>::const_iterator
           iter = m_synapseBackup.begin(); iter != m_synapseBackup.end();
           ++iter) {
        synapse.loadJsonObject(
              *iter, FlyEM::LOAD_PARTNER_LOCATION);
        m_doc->addSynapse(synapse, ZDvidSynapseEnsemble::DATA_LOCAL);
        m_doc->notifySynapseEdited(synapse);

        QString msg = QString("Synapse removal undone at (%1, %2, %3)").
            arg(synapse.getX()).arg(synapse.getY()).arg(synapse.getZ());
        m_doc->notify(msg);
      }
    }
  } else {
    m_doc->notify(ZWidgetMessage("Invalid DVID writer", NeuTube::MSG_ERROR));
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
  ZDvidWriter &writer = m_doc->getDvidWriter();
  ZDvidReader &reader = m_doc->getDvidReader();
  ZJsonObject synapseJson = reader.readSynapseJson(m_to);
  ZJsonObject propJson(synapseJson.value("Prop"));
  m_propertyBackup = propJson.clone();

  if (!synapseJson.isEmpty()) {
    ZDvidSynapse::AddProperty(
          synapseJson, "user", NeuTube::GetCurrentUserName());
    ZDvidSynapse::SetConfidence(synapseJson, 1.0);
    writer.writeSynapse(synapseJson);
    m_doc->syncSynapse(m_to);
  }

  m_doc->notifySynapseMoved(m_from, m_to);
  /*
  m_doc->notifySynapseEdited(m_from);
  m_doc->notifySynapseEdited(m_to);
  */
  QString msg = QString("Synapse moved from (%1, %2, %3) to (%4, %5, %6)").
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
  ZDvidWriter &writer = m_doc->getDvidWriter();
  ZDvidReader &reader = m_doc->getDvidReader();
  ZJsonObject synapseJson = reader.readSynapseJson(m_from);
  if (!synapseJson.isEmpty()) {
    ZDvidAnnotation::SetProperty(synapseJson, m_propertyBackup);
    writer.writeSynapse(synapseJson);
    m_doc->syncSynapse(m_from);
    std::vector<ZIntPoint> ptArray = ZDvidAnnotation::GetPartners(synapseJson);
    for (std::vector<ZIntPoint>::const_iterator iter = ptArray.begin();
         iter != ptArray.end(); ++iter) {
      m_doc->updateSynapsePartner(*iter);
    }
  }

  m_doc->notifySynapseMoved(m_to, m_from);
  /*
  m_doc->notifySynapseEdited(m_from);
  m_doc->notifySynapseEdited(m_to);
  */
  QString msg = QString("Synapse moving undone: (%1, %2, %3) <- (%4, %5, %6)").
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

////////////////////////////////////
ZStackDocCommand::DvidSynapseEdit::GroupSynapse::GroupSynapse(
    ZFlyEmProofDoc *doc, QUndoCommand *parent) : ZUndoCommand(parent)
{
  m_doc = doc;
}

ZStackDocCommand::DvidSynapseEdit::GroupSynapse::~GroupSynapse()
{

}

void ZStackDocCommand::DvidSynapseEdit::GroupSynapse::addSynapse(const ZIntPoint &pt)
{
  m_synapseSet.insert(pt);
}

void ZStackDocCommand::DvidSynapseEdit::GroupSynapse::addSynapse(
    const QList<ZIntPoint> &ptArray)
{
  foreach (const ZIntPoint &pt, ptArray) {
    addSynapse(pt);
  }
}

void ZStackDocCommand::DvidSynapseEdit::GroupSynapse::backupSynapse()
{
  /*
  if (m_doc != NULL) {
    ZDvidReader &reader = m_doc->getDvidReader();
    if (reader.isReady()) {
      m_synapseBackup = reader.readSynapseJson(m_synapseSet.begin(),
                                               m_synapseSet.end());
    }
  }
  */
  if (m_doc != NULL) {
    ZDvidReader &reader = m_doc->getDvidReader();
    if (reader.isReady()) {
      std::set<ZIntPoint> synapseSet;
      synapseSet.insert(m_synapseSet.begin(), m_synapseSet.end());

      ZJsonArray synapseArray = reader.readSynapseJson(m_synapseSet.begin(),
                                                       m_synapseSet.end());
      for (size_t i = 0; i < synapseArray.size(); ++i) {
        ZJsonObject synapseJson(synapseArray.value(i));
        std::vector<ZIntPoint> partnerArray =
            ZDvidAnnotation::GetPartners(synapseJson, "GroupedWith");
        synapseSet.insert(partnerArray.begin(), partnerArray.end());
      }

      m_synapseBackup = reader.readSynapseJson(
            synapseSet.begin(), synapseSet.end());
    }
  }
}

void ZStackDocCommand::DvidSynapseEdit::GroupSynapse::redo()
{
  if (m_synapseSet.size() > 1) {
    ZDvidReader &reader = m_doc->getDvidReader();
    ZDvidWriter &writer = m_doc->getDvidWriter();

    if (reader.isReady()) {
      backupSynapse();
      QString synapseString;
      QList<ZIntPoint> synapseArray;
      for (size_t i = 0; i < m_synapseBackup.size(); ++i) {
        ZJsonObject synapseJson(m_synapseBackup.value(i));
        ZIntPoint currentPos = ZDvidAnnotation::GetPosition(synapseJson);
        synapseArray.append(currentPos);
      }

      for (size_t i = 0; i < m_synapseBackup.size(); ++i) {
        bool added = false;
        ZJsonObject synapseJson(m_synapseBackup.value(i).clone());
        ZIntPoint currentPos = ZDvidAnnotation::GetPosition(synapseJson);
        if (m_synapseSet.count(currentPos) == 0) { //related synapses
          for (std::set<ZIntPoint>::const_iterator iter = m_synapseSet.begin();
               iter != m_synapseSet.end(); ++iter) {
            const ZIntPoint &pt = *iter;
            if (pt != currentPos) {
              added =
                  ZDvidSynapse::AddRelation(synapseJson, pt, "GroupedWith") || added;
            }
          }
        } else { //host synapses: add all backed-up synapses
          foreach (const ZIntPoint &pt, synapseArray) {
            if (pt != currentPos) {
              added =
                  ZDvidSynapse::AddRelation(synapseJson, pt, "GroupedWith") || added;
            }
          }
        }
        if (added) {
          ZDvidSynapse::Annotate(synapseJson, "Multi");
          synapseString.append(currentPos.toString().c_str());
          writer.writeSynapse(synapseJson);
        }
        m_doc->updateSynapsePartner(currentPos);
      }

      QString msg = QString("Synapses grouped: %1").arg(synapseString);
      m_doc->notify(msg);
    }
  }
}

void ZStackDocCommand::DvidSynapseEdit::GroupSynapse::undo()
{
  if (!m_synapseBackup.isEmpty()) {
    ZDvidWriter &writer = m_doc->getDvidWriter();
    writer.writeSynapse(m_synapseBackup);

    for (size_t i = 0; i < m_synapseBackup.size(); ++i) {
      ZJsonObject synapseJson(m_synapseBackup.value(i));
      m_doc->updateSynapsePartner(ZDvidAnnotation::GetPosition(synapseJson));
    }

    QString msg = QString("Undo synapse grouping.");
    m_doc->notify(msg);
  }
}

////////////////////////////////
#if 1
ZStackDocCommand::DvidSynapseEdit::UngroupSynapse::UngroupSynapse(
    ZFlyEmProofDoc *doc, QUndoCommand *parent) : ZUndoCommand(parent)
{
  m_doc = doc;
}

ZStackDocCommand::DvidSynapseEdit::UngroupSynapse::~UngroupSynapse()
{

}

void ZStackDocCommand::DvidSynapseEdit::UngroupSynapse::addSynapse(
    const ZIntPoint &pt)
{
  m_synapseSet.insert(pt);
}

void ZStackDocCommand::DvidSynapseEdit::UngroupSynapse::addSynapse(
    const QList<ZIntPoint> &ptArray)
{
  foreach (const ZIntPoint &pt, ptArray) {
    addSynapse(pt);
  }
}

void ZStackDocCommand::DvidSynapseEdit::UngroupSynapse::backupSynapse()
{
  if (m_doc != NULL) {
    ZDvidReader &reader = m_doc->getDvidReader();
    if (reader.isReady()) {
      std::set<ZIntPoint> synapseSet;
      synapseSet.insert(m_synapseSet.begin(), m_synapseSet.end());

      ZJsonArray synapseArray = reader.readSynapseJson(m_synapseSet.begin(),
                                                       m_synapseSet.end());
      for (size_t i = 0; i < synapseArray.size(); ++i) {
        ZJsonObject synapseJson(synapseArray.value(i));
        std::vector<ZIntPoint> partnerArray =
            ZDvidAnnotation::GetPartners(synapseJson, "GroupedWith");
        synapseSet.insert(partnerArray.begin(), partnerArray.end());
      }

      m_synapseBackup = reader.readSynapseJson(
            synapseSet.begin(), synapseSet.end());
    }
  }
}

void ZStackDocCommand::DvidSynapseEdit::UngroupSynapse::redo()
{
  if (!m_synapseSet.empty()) {
    ZDvidReader &reader = m_doc->getDvidReader();
    ZDvidWriter &writer = m_doc->getDvidWriter();

    if (reader.isReady()) {
      backupSynapse();

      QString synapseString;
      for (size_t i = 0; i < m_synapseBackup.size(); ++i) {
        bool removed = false;
        ZJsonObject synapseJson(m_synapseBackup.value(i).clone());
        ZIntPoint currentPos = ZDvidAnnotation::GetPosition(synapseJson);
        if (m_synapseSet.count(currentPos) == 0) { //related synapses
          for (std::set<ZIntPoint>::const_iterator iter = m_synapseSet.begin();
               iter != m_synapseSet.end(); ++iter) {
            const ZIntPoint &pt = *iter;
            if (pt != currentPos) {
              removed =
                  ZDvidSynapse::RemoveRelation(synapseJson, pt) || removed;
            }
          }
        } else { //host synapses
          removed = ZDvidSynapse::RemoveRelation(synapseJson, "GroupedWith") ||
              removed;
          synapseString.append(currentPos.toString().c_str());
        }
        if (removed) {
          writer.writeSynapse(synapseJson);
        }
        m_doc->updateSynapsePartner(currentPos);
      }

      QString msg = QString("Synapse ungrouped: %1").arg(synapseString);
      ZWidgetMessage message(
            msg, NeuTube::MSG_INFORMATION, ZWidgetMessage::TARGET_TEXT_APPENDING);
      m_doc->notify(message);
    }
  }
}

void ZStackDocCommand::DvidSynapseEdit::UngroupSynapse::undo()
{
  if (!m_synapseBackup.isEmpty()) {
    ZDvidWriter &writer = m_doc->getDvidWriter();
    writer.writeSynapse(m_synapseBackup);

    for (size_t i = 0; i < m_synapseBackup.size(); ++i) {
      ZJsonObject synapseJson(m_synapseBackup.value(i));
      m_doc->updateSynapsePartner(ZDvidAnnotation::GetPosition(synapseJson));
    }

    QString msg = QString("Synapses grouped back.");
    m_doc->notify(msg);
  }
}
#endif

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
