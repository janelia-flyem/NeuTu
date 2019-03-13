#include "flyembodyselectionmanager.h"

#include "common/utilities.h"
//#include "zflyembodymanager.h"

FlyEmBodySelectionManager::FlyEmBodySelectionManager()
{
}

/* For handling potential ID encoding */
uint64_t FlyEmBodySelectionManager::getNormalizedId(uint64_t bodyId) const
{
  return bodyId;
}

void FlyEmBodySelectionManager::selectBody(uint64_t bodyId)
{
  m_selected.insert(getNormalizedId(bodyId));

  m_lastSelected = bodyId;
}

void FlyEmBodySelectionManager::deselectBody(uint64_t bodyId)
{
  m_selected.erase(getNormalizedId(bodyId));
  m_lastSelected = 0;
}

bool FlyEmBodySelectionManager::isSelected(uint64_t bodyId) const
{
  return m_selected.count(getNormalizedId(bodyId)) > 0;
}

std::set<uint64_t> FlyEmBodySelectionManager::getSelected() const
{
  return m_selected;
}

void FlyEmBodySelectionManager::deselectAll()
{
  m_selected.clear();
  m_lastSelected = 0;
}

uint64_t FlyEmBodySelectionManager::getLastSelected() const
{
  return m_lastSelected;
}

std::pair<std::set<uint64_t>,std::set<uint64_t>>
FlyEmBodySelectionManager::flushSeletionChange()
{
  std::set<uint64_t> selected = neutu::setdiff(m_selected, m_prevSelected);
  std::set<uint64_t> deselected = neutu::setdiff(m_prevSelected, m_selected);

  m_prevSelected = m_selected;

  return std::pair<std::set<uint64_t>,std::set<uint64_t>>(selected, deselected);
}
