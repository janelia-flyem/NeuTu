#ifndef ZDVIDSYNPASECOMMAND_H
#define ZDVIDSYNPASECOMMAND_H

#include "zstackdoccommand.h"
#include "dvid/zdvidsynapse.h"

class ZFlyEmProofDoc;

namespace ZStackDocCommand {
namespace DvidSynapseEdit {

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

class RemoveSynapse : public ZUndoCommand
{
public:
  RemoveSynapse(ZFlyEmProofDoc *doc, int x, int y, int z,
                QUndoCommand *parent = NULL);
  virtual ~RemoveSynapse();
  void undo();
  void redo();

private:
  ZFlyEmProofDoc *m_doc;
  ZIntPoint m_synapse;
  ZJsonObject m_synapseBackup;
};

class AddSynapse : public ZUndoCommand
{
public:
  AddSynapse(ZFlyEmProofDoc *doc, const ZDvidSynapse &synapse,
             QUndoCommand *parent = NULL);
  virtual ~AddSynapse();
  void undo();
  void redo();

private:
  ZFlyEmProofDoc *m_doc;
  ZDvidSynapse m_synapse;
};

class MoveSynapse : public ZUndoCommand
{
public:
  MoveSynapse(ZFlyEmProofDoc *doc, const ZIntPoint &from, const ZIntPoint &to,
              QUndoCommand *parent = NULL);
  virtual ~MoveSynapse();
  void undo();
  void redo();

private:
  ZFlyEmProofDoc *m_doc;
  ZIntPoint m_from;
  ZIntPoint m_to;
};

class LinkSynapse : public ZUndoCommand
{
public:
  LinkSynapse(ZFlyEmProofDoc *doc, const ZIntPoint &from,
              QUndoCommand *parent = NULL);
  LinkSynapse(ZFlyEmProofDoc *doc, const ZIntPoint &from, const ZIntPoint &to,
              const std::string &relation, QUndoCommand *parent = NULL);
  virtual ~LinkSynapse();
  void undo();
  void redo();
  void addRelation(const ZIntPoint &to, const std::string &relation);

private:
  ZFlyEmProofDoc *m_doc;
  ZIntPoint m_from;
  ZJsonArray m_relJson;
//  ZIntPoint m_to;
//  std::string m_relation;
  ZJsonObject m_synapseBackup;
};

class UnlinkSynapse : public ZUndoCommand
{
public:
  UnlinkSynapse(ZFlyEmProofDoc *doc, const std::set<ZIntPoint> &ptSet,
              QUndoCommand *parent = NULL);
  virtual ~UnlinkSynapse();
  void undo();
  void redo();

private:
  ZFlyEmProofDoc *m_doc;
  std::set<ZIntPoint> m_synapseSet;
  ZJsonArray m_synapseBackup;
};


}
}

#endif // ZDVIDSYNPASECOMMAND_H
