#ifndef ZACTIONFACTORY_H
#define ZACTIONFACTORY_H

#include <QAction>

class ZStackDoc;
class QWidget;
class ZActionActivator;

class ZActionFactory
{
public:
  ZActionFactory();


  enum EActionItem {
    ACTION_MEASURE_SWC_NODE_LENGTH, ACTION_MEASURE_SCALED_SWC_NODE_LENGTH,
    ACTION_SWC_SUMMARIZE,
    ACTION_CHNAGE_SWC_NODE_SIZE, ACTION_TRANSLATE_SWC_NODE,
    ACTION_SET_SWC_ROOT, ACTION_INSERT_SWC_NODE,
    ACTION_RESET_BRANCH_POINT, ACTION_SET_BRANCH_POINT,
    ACTION_CONNECTED_ISOLATED_SWC,
    ACTION_DELETE_SWC_NODE, ACTION_CONNECT_SWC_NODE,
    ACTION_MERGE_SWC_NODE, ACTION_BREAK_SWC_NODE,
    ACTION_SELECT_DOWNSTREAM, ACTION_SELECT_UPSTREAM,
    ACTION_SELECT_NEIGHBOR_SWC_NODE,
    ACTION_SELECT_SWC_BRANCH, ACTION_SELECT_CONNECTED_SWC_NODE,
    ACTION_SELECT_ALL_SWC_NODE,
    ACTION_CHANGE_SWC_TYPE, ACTION_CHANGE_SWC_SIZE, ACTION_REMOVE_TURN,
    ACTION_RESOLVE_CROSSOVER, ACTION_SWC_Z_INTERPOLATION,
    ACTION_SWC_RADIUS_INTERPOLATION, ACTION_SWC_POSITION_INTERPOLATION,
    ACTION_SWC_INTERPOLATION
  };

  static QAction* makeAction(
      EActionItem item, const ZStackDoc *doc, QWidget *parent,
      ZActionActivator *activator = NULL, bool positive = true);
};

#endif // ZACTIONFACTORY_H
