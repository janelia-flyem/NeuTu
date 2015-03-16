#include "zdag.h"

template <typename T>
ZDagNode<T>::ZDagNode() : m_isVisited(false)
{

}

template <typename T>
ZDag<T>::ZDag()
{
}

template <typename T>
std::vector<ZSharedPointer<ZDagNode<T> > > ZDag<T>::getNodeArray() const
{
  std::vector<ZSharedPointer<ZDagNode<T> > > nodeArray;

  nodeArray.push_back(m_root);

  nodeArray.push_back(m_root.getChildArray());

  return nodeArray;
}

