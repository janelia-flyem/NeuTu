#ifndef ZSTACKOBJECTSELECTOR_H
#define ZSTACKOBJECTSELECTOR_H

#include <set>
#include <vector>

#include "zstackobject.h"
#include "zselector.h"

/*!
 * \brief The class of managing object selection
 *
 * The class select an object and meanwhile record the selection action. It
 * stores information which objects have been selected and which objects have
 * been deselected.
 */
class ZStackObjectSelector : public ZSelector<const ZStackObject*>
{
public:
  ZStackObjectSelector();

  /*!
   * \brief Reset to initial state
   *
   * This funciton only clears selection history without changing the selection
   * state of any object.
   */
//  void reset();

  /*!
   * \brief Select an object
   *
   * Select an object and record the selection action:
   *   1. If \a obj has already been selected, nothing will done; or
   *   2. If \a obj is in the deselected set, it will be removed from the set.
   *      But it will not be added to the selected set; or
   *   3. \a obj will be added to the selected set.
   *
   * \param obj an object to be selected
   */
  void selectObject(ZStackObject *obj);

  /*!
   * \brief Deselect an object
   *
   * Deselect an object and record the selection action:
   *   1. If \a obj has already been deselected, nothing will done; or
   *   2. If \a obj is in the selected set, it will be removed from the set.
   *      But it will not be added to the deselected set; or
   *   3. \a obj will be added to the deselected set.
   *
   * \param obj an object to be selected
   */
  void deselectObject(ZStackObject *obj);

  /*!
   * \brief The function of selecting or deselecting an object
   */
  void setSelection(ZStackObject *obj, bool selecting);

//  bool isInSelectedSet(const ZStackObject *obj) const;
//  bool isInDeselectedSet(const ZStackObject *obj) const;

//  bool isEmpty() const;

  void print() const;

  std::vector<ZStackObject*> getSelectedList(ZStackObject::EType type) const;
  std::vector<ZStackObject*> getDeselectedList(ZStackObject::EType type) const;

  std::set<ZStackObject*> getSelectedSet(ZStackObject::EType type);
  std::set<ZStackObject*> getDeselectedSet(ZStackObject::EType type);
};

#endif // ZSTACKOBJECTSELECTOR_H
