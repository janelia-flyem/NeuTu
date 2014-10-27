#include "ztree.h"
#include <queue>

template <typename T>
ZTree<T>::ZTree(void)
{
  m_root = NULL;
}

template <typename T>
ZTree<T>::ZTree(ZTreeNode<T> *root)
{
  m_root = root;
}

template <typename T>
ZTree<T>::~ZTree(void)
{
  clear();
}


template <typename T>
void ZTree<T>::clear()
{
  ZTreeIterator<T> iterator(*this);
  while (iterator.hasNext()) {
    ZTreeNode<T> *node = iterator.nextNode();
    delete node;
  }
}


template <typename T>
bool ZTree<T>::isEmpty()
{
  return m_root == NULL;
}

template <typename T>
void ZTree<T>::setRoot(
    ZTreeNode<T>* root,   //New root
    bool clearOld         //Clear the old root or not
    )
{
  if (clearOld) {
    clear();
  }

  m_root = root;
}

/*********** External Iterator **************/

template <typename T>
ZTreeIterator<T>::ZTreeIterator(const ZTree<T> &tree, EIteratorOption option)
{
    m_filter = NO_FILTER;
    update(tree, option);
}

template <typename T>
void ZTreeIterator<T>::update(const ZTree<T> &tree, EIteratorOption option)
{
  m_nodeArray.clear();

  const ZTreeNode<T> *tn = tree.getRoot();
  const ZTreeNode<T> *lastNode = tree.getRoot();

  if (tn != NULL) {
    switch(option) {
    case DEPTH_FIRST:
    {
      //m_nodeArray.push_back(const_cast<ZTreeNode<T>*>(tree.getRoot()));

      tryAppendingNode(lastNode);

      while (tn != NULL) {
        if (tn->firstChild() != NULL) {
          lastNode = tn->firstChild();
          tryAppendingNode(lastNode);

          //m_nodeArray.push_back(tn->firstChild());
          //tn->setNext(tn->firstChild());
        } else {
          if (tn->nextSibling() != NULL) {
              lastNode = tn->nextSibling();
              tryAppendingNode(lastNode);
            //m_nodeArray.push_back(tn->nextSibling());
            //tn->setNext(tn->nextSibling());
          } else {
            ZTreeNode<T> *parent_tn = tn->parent();
            while (parent_tn != NULL) {
              if (parent_tn->nextSibling() != NULL) {
                  lastNode = parent_tn->nextSibling();
                  tryAppendingNode(lastNode);
                //m_nodeArray.push_back(parent_tn->nextSibling());
                //tn->setNext(parent_tn->nextSibling());
                break;
              } else {
                parent_tn = parent_tn->parent();
              }
            }
            if (parent_tn == NULL) {
              tn = NULL;
            }
          }
        }

        if (tn != NULL) {
          //tn = m_nodeArray.back();
            tn = lastNode;
        }
      }
    }
      break;
    case BREADTH_FIRST:
    {
      std::queue<const ZTreeNode<T>*> currentSiblingQueue;
      currentSiblingQueue.push(lastNode);
      while (!currentSiblingQueue.empty()) {
        tn = currentSiblingQueue.front();
        tryAppendingNode(tn);
        currentSiblingQueue.pop();

        const ZTreeNode<T> *child = tn->firstChild();
        while (child != NULL) {
          currentSiblingQueue.push(child);
          child = child->nextSibling();
        }
      }
    }
      break;
    default:
      break;
    }
  }

  m_currentIndex = 0;
}

template <typename T>
bool ZTreeIterator<T>::hasNext()
{
  return m_currentIndex < m_nodeArray.size();
}

template <typename T>
T& ZTreeIterator<T>::next()
{
  T& data =  m_nodeArray[m_currentIndex]->data();
  ++m_currentIndex;

  return data;
}

template <typename T>
ZTreeNode<T>* ZTreeIterator<T>::nextNode()
{
  ZTreeNode<T> *node =  m_nodeArray[m_currentIndex];
  ++m_currentIndex;

  return node;
}

template <typename T>
bool ZTreeIterator<T>::passedFilter(const ZTreeNode<T> *node) const
{
    if (node == NULL) {
        return false;
    }

    switch (m_filter) {
    case NO_FILTER:
      return true;
    case LEAF:
      return node->isLeaf();
    case BRANCH_POINT:
      return node->isBranchPoint();
    case CONTIUATION:
      return node->isContinuation();
    default:
        break;
    }

    return false;
}

template <typename T>
void ZTreeIterator<T>::tryAppendingNode(const ZTreeNode<T> *node)
{
    if (passedFilter(node)) {
        m_nodeArray.push_back(const_cast<ZTreeNode<T>*>(node));
    }
}
