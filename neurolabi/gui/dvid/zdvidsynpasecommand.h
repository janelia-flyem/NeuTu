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

}
}

#endif // ZDVIDSYNPASECOMMAND_H
