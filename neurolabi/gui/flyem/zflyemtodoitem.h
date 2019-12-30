#ifndef ZFLYEMTODOITEM_H
#define ZFLYEMTODOITEM_H

#include <map>
#include "dvid/zdvidannotation.h"

class ZFlyEmToDoItem : public ZDvidAnnotation
{
public:
  /*!
   * \brief Default constructor
   *
   * The object is always set to invalid kind by default.
   */
  ZFlyEmToDoItem();

  /*!
   * \brief Constructor with initial position
   *
   * With a position given, the object is set to a valid type.
   */
  ZFlyEmToDoItem(const ZIntPoint &pos);
  ZFlyEmToDoItem(int x, int y, int z);

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const;

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::FLYEM_TODO_ITEM;
  }

  friend std::ostream& operator<< (
      std::ostream &stream, const ZFlyEmToDoItem &synapse);

  bool isChecked() const;
  void setChecked(bool checked);
  int getPriority() const;
  std::string getPriorityName() const;

  static std::string GetPriorityName(int p);
//  bool isCheckable() const;

  void setAction(neutu::EToDoAction action);
  neutu::EToDoAction getAction() const;
  std::string getActionName() const;

  void setAction(const std::string &action);
  bool hasSomaAction() const;

  QColor getDisplayColor() const;

  void removeActionTag();

  void setPriority(int p);

  static std::string GetActionTag(neutu::EToDoAction action);

private:
  void syncActionTag();
  QList<std::vector<QPointF>> makeOutlineDecoration(
      double x, double y, double radius) const;
  static void PaintOutline(
      ZPainter &painter, const QList<std::vector<QPointF>> &outline);

public:
  static const char *KEY_ACTION;
  static const char *ACTION_GENERAL;
  static const char *ACTION_SPLIT;
  static const char *ACTION_SUPERVOXEL_SPLIT;
  static const char *ACTION_IRRELEVANT;
  static const char *ACTION_MERGE;
  static const char *ACTION_TRACE_TO_SOMA;
  static const char *ACTION_NO_SOMA;
  static const char *ACTION_DIAGNOSTIC;
  static const char *ACTION_SEGMENTATION_DIAGNOSTIC;
  static const char *ACTION_SEGMENTATION_DIAGNOSTIC_TAG;
  static const char *ACTION_TIP_DETECTOR;
  static const char *ACTION_TIP_DETECTOR_TAG;
  static const char *ACTION_SPLIT_TAG;
  static const char *ACTION_SUPERVOXEL_SPLIT_TAG;
  static const char *ACTION_IRRELEVANT_TAG;
  static const char *ACTION_MERGE_TAG;
  static const char *ACTION_TRACE_TO_SOMA_TAG;
  static const char *ACTION_NO_SOMA_TAG;
  static const char *ACTION_DIAGNOSTIC_TAG;

  static const std::map<std::string, neutu::EToDoAction> m_actionMap;

private:
  void init();
  void init(EKind kind);
};

#endif // ZFLYEMTODOITEM_H
