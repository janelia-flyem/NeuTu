#ifndef ZSTACKOBJECTSELECTOR_H
#define ZSTACKOBJECTSELECTOR_H

#include <set>
#include <vector>

#include "zstackobject.h"

class ZStackObjectSelector
{
public:
  ZStackObjectSelector();

  void reset();
  void selectObject(ZStackObject *obj);
  void deselectObject(ZStackObject *obj);
  void setSelection(ZStackObject *obj, bool selecting);

  bool isInSelectedSet(const ZStackObject *obj) const;
  bool isInDeselectedSet(const ZStackObject *obj) const;

  bool isEmpty() const;

  void print() const;

  std::vector<ZStackObject*> getSelectedList(ZStackObject::EType type);
  std::vector<ZStackObject*> getDeselectedList(ZStackObject::EType type);

  std::set<ZStackObject*> getSelectedSet(ZStackObject::EType type);
  std::set<ZStackObject*> getDeselectedSet(ZStackObject::EType type);


private:
  std::set<ZStackObject*> m_selectedSet;
  std::set<ZStackObject*> m_deselectedSet;
};

#endif // ZSTACKOBJECTSELECTOR_H
