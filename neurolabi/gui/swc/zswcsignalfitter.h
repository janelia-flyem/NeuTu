#ifndef ZSWCSIGNALFITTER_H
#define ZSWCSIGNALFITTER_H

#include "core/neutube_def.h"
#include "swctreenode.h"

class ZStack;
class ZSwcTree;

class ZSwcSignalFitter
{
public:
  ZSwcSignalFitter();

  void setBackground(neutube::EImageBackground bg) {
    m_background = bg;
  }

  void setFixingTerminal(bool on) {
    m_fixingTerminal = on;
  }

  neutube::EImageBackground getBackground() const {
    return m_background;
  }

  bool fitSignal(Swc_Tree_Node *tn, const ZStack *stack, int channel = 0);
  void fitSignal(ZSwcTree *tree, const ZStack *stack, int channel = 0);
  void fitSignal(Swc_Tree *tree, const ZStack *stack, int channel = 0);

private:
  void init();

private:
  neutube::EImageBackground m_background;
  bool m_fixingTerminal;

};

#endif // ZSWCSIGNALFITTER_H
