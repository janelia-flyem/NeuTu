#ifndef FLYEMBODYSELECTIONMANAGER_H
#define FLYEMBODYSELECTIONMANAGER_H

#include <cstdint>
#include <utility>
#include <set>

//#include "zselector.h"

/*!
 * \brief The manager class for body selection
 *
 */
class FlyEmBodySelectionManager
{
public:
  FlyEmBodySelectionManager();

public:
  void selectBody(uint64_t bodyId);
  void deselectBody(uint64_t bodyId);
  bool isSelected(uint64_t bodyId) const;
  uint64_t getLastSelected() const;
  void deselectAll();
  std::pair<std::set<uint64_t>,std::set<uint64_t>> flushSeletionChange();

  std::set<uint64_t> getSelected() const;

private:
  uint64_t getNormalizedId(uint64_t bodyId) const;

private:
  std::set<uint64_t> m_selected;
  std::set<uint64_t> m_prevSelected;
  uint64_t m_lastSelected = 0;
};

#endif // FLYEMBODYSELECTIONMANAGER_H
