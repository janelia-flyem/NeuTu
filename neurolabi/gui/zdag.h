#ifndef ZDAG_H
#define ZDAG_H

#include <vector>
#include "zsharedpointer.h"

//class
template <typename T>
class ZDagNodePtr /*: public ZSharedPointer<ZDagNode<T> >*/ {

};

template <typename T>
class ZDagNode
{
public:
  ZDagNode();

  static ZSharedPointer<ZDagNode<T> > makeNode();

  const std::vector<ZSharedPointer<ZDagNode<T> > >& getParentList() const {
    return m_parentList;
  }
  std::vector<ZSharedPointer<ZDagNode<T> > >& getParentList() {
    return m_parentList;
  }

  const std::vector<ZSharedPointer<ZDagNode<T> > >& getChildList() const {
    return m_childList;
  }
  std::vector<ZSharedPointer<ZDagNode<T> > >& getChildList() {
    return m_childList;
  }

private:
  std::vector<ZSharedPointer<ZDagNode<T> > > m_parentList;
  std::vector<ZSharedPointer<ZDagNode<T> > > m_childList;
  bool m_isVisited;
};

template <typename T>
class ZDag
{
public:
  ZDag();
  ~ZDag();

  std::vector<ZSharedPointer<ZDagNode<T> > > getNodeArray() const;

private:
  ZSharedPointer<ZDagNode<T> > m_root;
};



#endif // ZDAG_H
