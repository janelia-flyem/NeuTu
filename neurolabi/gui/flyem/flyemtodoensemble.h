#ifndef FLYEMTODOENSEMBLE_H
#define FLYEMTODOENSEMBLE_H

//#include <memory>
//#include <functional>

//#include "zstackobject.h"
#include "zflyemtodoitem.h"
#include "flyemtodochunk.h"
#include "flyempointannotationensemble.hpp"
//#include "zselector.h"

class FlyEmTodoBlockGrid;
class FlyEmTodoSource;
class ZDvidTarget;

class FlyEmTodoEnsemble :
    public FlyEmPointAnnotationEnsemble<ZFlyEmToDoItem, FlyEmTodoChunk>
{
public:
  FlyEmTodoEnsemble();
  ~FlyEmTodoEnsemble() override;

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::FLYEM_TODO_ENSEMBLE;
  }

//  bool display(
//      QPainter *painter, const DisplayConfig &config) const override;


  void setDvidTarget(const ZDvidTarget &target);

#if 0
  void addItem(const ZFlyEmToDoItem &item);
  void removeItem(const ZIntPoint &pos);

  /*!
   * \brief Remove selected todos
   *
   * It returns the number of todos that are removed.
   */
  int removeSelected(std::function<void(const std::string &)> errorProcessor);

  bool hit(double x, double y, double z) override;

  void processHit(ESelection s) override;
  void deselectSub() override;

  bool hasSelected() const;

  std::set<ZIntPoint> getSelectedPos() const;

  /*
  bool selectItem(int x, int y, int z);
  bool selectItem(const ZIntPoint &pt);
  */

  /*
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;
  void display(ZPainter &painter, const DisplayConfig &config) const override;
  */
#endif
public: //For testing
  void _setSource(std::shared_ptr<FlyEmTodoSource> source);
  ZFlyEmToDoItem _getHitItem() const;
  std::shared_ptr<FlyEmTodoBlockGrid> _getBlockGrid() const;

//private:
//  void selectAt(const ZIntPoint &pos);
//  void deselectAt(const ZIntPoint &pos);
//  void setSelectionAt(const ZIntPoint &pos, bool selecting);
//  void processDeselected();

//private:
//  std::shared_ptr<FlyEmTodoBlockGrid> m_blockGrid;
//  ZFlyEmToDoItem m_hitItem;
//  ZSelector<ZIntPoint> m_selector;
};

#endif // FLYEMTODOENSEMBLE_H
