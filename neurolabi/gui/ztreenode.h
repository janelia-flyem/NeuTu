#ifndef ZTREENODE_H
#define ZTREENODE_H

#include <vector>
#include "tz_cdefs.h"
#include "zuncopyable.h"

template<typename T>
class ZTreeNode : public ZUncopyable
{
public:
    ZTreeNode();
    ZTreeNode(const ZTreeNode<T> &node);
    ZTreeNode(const T &data);
    virtual ~ZTreeNode(void);

public:
    inline ZTreeNode<T>* parent() const { return m_parent; }
    inline ZTreeNode<T>* firstChild() const { return m_firstChild; }
    inline ZTreeNode<T>* nextSibling() const { return m_nextSibling; }
    void setFirstChild(ZTreeNode<T> *child, bool updatingConsisitency = true);
    void setNextSibling(ZTreeNode<T> *sibling, bool updatingConsistency = true);
    ZTreeNode<T>* getNextAt(int index) const;

    ZTreeNode<T>* getChild(int index) const;

public:
    inline T& data() { return m_data; }
    inline const T& data() const { return m_data; }
    inline T* dataRef() { return &m_data; }
    inline const T* constDataRef() const { return &m_data; }

    inline void setData(const T &data) { m_data = data; }

    inline double weight() const { return m_weight; }
    inline void setWeight(double weight) { m_weight = weight; }

    inline int label() const { return m_label; }
    inline void setLabel(int label) { m_label = label; }

    inline int index() const { return m_index; }
    inline void setIndex(int index) { m_index = index; }

    ZTreeNode<T>* lastChild() const;

    bool isChildOf(const ZTreeNode<T> *parent) const;

    int childNumber()  const;

    inline bool hasChild()  const{ return (m_firstChild != NULL); }

    ZTreeNode<T>* previousSibling() const;

    int getSiblingIndex() const;

public:
    bool isRoot() const;
    bool isLastChild() const;
    bool isLeaf() const;
    bool isBranchPoint() const;
    bool isContinuation() const;
    bool isSpur() const;
    bool isSibling(const ZTreeNode<T> *node) const;

    /*!
     * \brief A node is an orphan if it has no linked node.
     */
    bool isOrphan() const;


    ZTreeNode<T>* addChild(ZTreeNode<T> *node);
    ZTreeNode<T>* addChild(const T& data);
    void removeChild(ZTreeNode<T> *child);
    void replaceChild(ZTreeNode<T> *oldChild, ZTreeNode<T> *newChild);

    /*!
     * \brief Remove all descendents of the node
     *
     * The children of the node and their descendents will be deleted from memory.
     */
    void killDownstream();

    ZTreeNode<T>* detachParent();
    void setParent(ZTreeNode<T> *p, bool updatingConsistency = true);

    void insert(ZTreeNode<T> *node);

    void becomeFirstChild();

    void mergeSibling(ZTreeNode<T> *node);

    virtual ZTreeNode<T>* mergeToParent();

    void becomeRoot(bool virtualCheck = true);

    double getBacktraceWeight(int n) const;

    void labelBranch(int label);

    virtual inline int id() const { return m_index; }
    int parentId() const;
    virtual inline void setId(int id) { m_index = id; }

protected:
    T m_data;                      //data
    ZTreeNode<T> *m_parent;        //parent
    ZTreeNode<T> *m_firstChild;    //first child
    ZTreeNode<T> *m_nextSibling;   //next sibling
    double m_weight;               //weight to connect to its parent
    int m_label;                   //label of identifying the node
    int m_index;                   //0-based index in iteration
    std::vector<double> m_featureVector;              //feature of the node
};

#include "ztreenode.cpp"

#endif // ZTREENODE_H
