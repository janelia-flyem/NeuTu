#ifndef FLYEMTODOENSEMBLE_H
#define FLYEMTODOENSEMBLE_H

#include <memory>

#include "zstackobject.h"

class FlyEmTodoBlockGrid;

class FlyEmTodoEnsemble : public ZStackObject
{
public:
  FlyEmTodoEnsemble();
  virtual ~FlyEmTodoEnsemble();

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::FLYEM_TODO_ENSEMBLE;
  }

  bool display(
      QPainter *painter, const DisplayConfig &config) const {
    return false;
  }

  /*
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;
  void display(ZPainter &painter, const DisplayConfig &config) const override;
  */

private:
  std::shared_ptr<FlyEmTodoBlockGrid> m_blockGrid;
};

#endif // FLYEMTODOENSEMBLE_H
