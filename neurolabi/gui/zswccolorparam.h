#ifndef ZSWCCOLORPARAM_H
#define ZSWCCOLORPARAM_H

#include <map>
#include <memory>
#include <utility>
#include <QString>
#include <QMutex>

#include "zswccolorscheme.h"
#include "zutils.h"
#include "widgets/znumericparameter.h"

class ZSwcTree;
class Z3DSwcFilter;

class ZSwcColorParam
{
public:
  ZSwcColorParam();

  ZVec4Parameter& getColorParameter(void *obj);
  ZVec4Parameter* getColorParameterPtr(void *obj);
  void remove(void *obj);

  void setColorScheme(const ZColorScheme::EColorScheme colorScheme);
  void setPrefix(const QString &prefix);

  bool contains(void *obj) const;

  template<typename InputIterator>
  std::pair<std::vector<ZVec4Parameter*>, std::vector<ZVec4Parameter*>>
  update(const InputIterator &begin, const InputIterator &end);

  std::map<void*, ZVec4Parameter*> getColorParamMap() const;

  /*
  void setSyncMutex(QMutex *m) {
    m_syncMutex = m;
  }
  */
  
private:
  void makeColorPamater(void *obj);

private:
  std::map<void*, size_t> m_mapper;
  std::vector<std::shared_ptr<ZVec4Parameter>> m_paramList;
  std::vector<bool> m_paramTaken;
  ZSwcColorScheme m_scheme;
  QString m_prefix;
//  QMutex *m_syncMutex = nullptr;
//  Z3DSwcFilter *m_host = nullptr;
};

template<typename InputIterator>
std::pair<std::vector<ZVec4Parameter*>, std::vector<ZVec4Parameter*>>
ZSwcColorParam::update(const InputIterator &begin, const InputIterator &end)
{
  std::pair<std::vector<ZVec4Parameter*>, std::vector<ZVec4Parameter*>> updated;
  std::vector<ZVec4Parameter*> &added = updated.first;
  std::vector<ZVec4Parameter*> &removed = updated.second;

  std::set<void*> newObjectSet;
  for (InputIterator iter = begin; iter != end; ++iter) {
    newObjectSet.insert(*iter);
#ifdef _DEBUG_
    std::cout << "***new object: " << *iter << std::endl;
#endif
  }

#ifdef _DEBUG_
  for (auto s : m_mapper) {
    std::cout << "***old object: " << s.first << std::endl;
  }
#endif
  
  std::map<void*, size_t> obsoleteSet;
  std::set_difference(m_mapper.begin(), m_mapper.end(),
                      newObjectSet.begin(), newObjectSet.end(),
                      std::inserter(obsoleteSet, obsoleteSet.end()),
                      _KeyLess());

  for (auto s : obsoleteSet) {
    removed.push_back(m_paramList[s.second].get());
    remove(s.first);
  }

//  std::vector<ZVec4Parameter*> paramList;
  for (InputIterator iter = begin; iter != end; ++iter) {
    if (m_mapper.count(*iter) == 0) {
      added.push_back(getColorParameterPtr(*iter));
    }
  }

  return updated;
}

#endif // ZSWCCOLORPARAM_H
