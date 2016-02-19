#ifndef ZTREE_H
#define ZTREE_H

#include <vector>
#include <fstream>
#include "ztreenode.h"
#include "ztreeiterator.h"

template <typename T>
class ZTree
{
public:
  ZTree(void);
  explicit ZTree(ZTreeNode<T> *root);
  virtual ~ZTree(void);

  inline ZTreeNode<T>* getRoot() { return m_root; }
  inline const ZTreeNode<T>* getRoot() const { return m_root; }
  void setRoot(ZTreeNode<T>* root, bool clearOld = true);

  bool isEmpty() const;
  void clear();

protected:
  ZTreeNode<T> *m_root;         /**< root of the tree */
};

/*!
 * The class of external tree iterator.
 */
template<typename T>
class ZTreeIterator
{
public:
  /* Options for iterating. */
  enum EIteratorOption{
    DEPTH_FIRST,  //depth first search
    BREADTH_FIRST //breadth first search
  };

  enum EIteratorFilter {
    NO_FILTER, LEAF, BRANCH_POINT, CONTIUATION
  };

  ZTreeIterator(const ZTreeNode<T>* startNode,
                EIteratorOption option = DEPTH_FIRST);
  ZTreeIterator(const ZTree<T> &tree,
                EIteratorOption option = DEPTH_FIRST);
  void update(const ZTreeNode<T>* startNode,
              EIteratorOption option = DEPTH_FIRST);
  void update(const ZTree<T> &tree,
              EIteratorOption option = DEPTH_FIRST);
  bool hasNext();
  T &next();
  ZTreeNode<T>* nextNode();
  inline size_t size() const { return m_nodeArray.size(); }
  inline void reset() { m_currentIndex = 0; }
  void reverse();

private:
  bool passedFilter(const ZTreeNode<T> *node) const;
  void tryAppendingNode(const ZTreeNode<T> *node);

private:
  std::vector<ZTreeNode<T>*> m_nodeArray;
  size_t m_currentIndex;
  EIteratorFilter m_filter;
};

#include "ztree.cpp"





#endif // ZTREE_H
