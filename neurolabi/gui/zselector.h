#ifndef ZSELECTOR_H
#define ZSELECTOR_H

#include <set>
#include <vector>

template<typename T>
struct add_const_to_pointee
{
    typedef T type;
};

template <typename T>
struct add_const_to_pointee<T*>
{
    typedef const T* type;
};

template<class T> T& unconst(T const& t){return const_cast<T&>(t);}

/*!
 * \brief The template class of managing object selection
 *
 * The class select an object and meanwhile record the selection action. It
 * stores information which objects have been selected and which objects have
 * been deselected. The template parameter T has to be basic type or a pointer.
 */
template <typename T>
class ZSelector
{
public:
  ZSelector();

  /*!
   * \brief Reset to initial state
   *
   * This funciton only clears selection history without changing the selection
   * state of any object.
   */
  void reset();

  /*!
   * \brief Reset selections
   * \param selected Current selected set.
   * \param prevSelected Previous selected set.
   */
  void reset(const std::set<T> &selected, const std::set<T> &prevSelected);

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
  void selectObject(const T &obj);

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
  void deselectObject(const T &obj);

  void deselectAll();

  /*!
   * \brief The function of selecting or deselecting an object
   */
  void setSelection(T obj, bool selecting);

  bool isInSelectedSet(const T& obj) const;
  bool isInDeselectedSet(const T& obj) const;

  bool isEmpty() const;

  virtual void print() const;

  std::vector<T> getSelectedList() const;
  std::vector<T> getDeselectedList() const;

  const std::set<T>& getSelectedSet() const;
  const std::set<T>& getDeselectedSet() const;

protected:
  std::set<T> m_selectedSet;
  std::set<T> m_deselectedSet;
};

#include "zselector.cpp"

#endif // ZSELECTOR_H
