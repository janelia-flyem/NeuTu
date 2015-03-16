#ifndef ZDVIDVERSIONDAG_H
#define ZDVIDVERSIONDAG_H

#include <vector>
#include <string>
#include <map>
#include <list>

#include "zgraph.h"
#include "zsharedpointer.h"
#include "zdvidversionnode.h"
#include "ztree.h"

class ZJsonObject;

struct ZDvidVersionDagData {
  ZTree<ZDvidVersionNode> m_tree;
  std::map<std::string, ZTreeNode<ZDvidVersionNode>* > m_versionList;
  std::map<std::string, std::list<std::string> > m_parentMap;
};

class ZDvidVersionDag
{
public:
  ZDvidVersionDag();

  void clear();

  bool isEmpty() const;

  ZDvidVersionNode getNode(const std::string &uuid) const;
  bool hasNode(const std::string &uuid) const;

  std::vector<std::string> getParentList(const std::string &uuid) const;
  std::vector<std::string> getChildList(const std::string &uuid) const;
  std::string getFirstParent(const std::string &uuid) const;
  int getSiblingIndex(const std::string &uuid) const;

  bool isLocked(const std::string &uuid) const;
  bool isActive(const std::string &uuid) const;

  std::string getRoot() const;
  std::string getChild(const std::string uuid, int index) const;

  void print() const;

  /*!
   * \brief Set the root of the DAG.
   *
   * All existing nodes will be removed.
   *
   * \param uuid the uuid of the root
   */
  void setRoot(const std::string &uuid);

  /*!
   * \brief Add a node as a child of an existing node
   *
   * No node will be added if \a parentUuid does not exist in the current DAG.
   * If \a uuid already exists but \a parentUuid is not the parent of \a uuid,
   * the function will try to set \a parentUuid as a parent \a uuid unless there
   * is a conflict. Note that the function only detects conflict within one
   * level, so the user need to make sure that \a parentUuid is not a
   * grandparent of \a uuid or vice versa.
   *
   * \param uuid the uuid of the new node
   * \param parentUuid the uuid of the parent node
   * \return true iff a new node or a new link is created.
   */
  bool addNode(const std::string &uuid, const std::string &parentUuid);

  bool isParent(const std::string &uuid, const std::string &parentUuid) const;

  void load(const ZJsonObject &obj, const std::string &uuid);

  void lock(const std::string &uuid);
  void unlock(const std::string &uuid);

  void activateNode(const std::string &uuid);
  void deactivateNode(const std::string &uuid);

  std::vector<std::string> getBreadthFirstList() const;

private:
  const ZTreeNode<ZDvidVersionNode>* getDagNode(const std::string &uuid) const;
  ZTreeNode<ZDvidVersionNode>* getDagNode(const std::string &uuid);
  ZTreeNode<ZDvidVersionNode>* addNode(const std::string &uuid);

  const std::map<std::string, ZTreeNode<ZDvidVersionNode>* >&
  getVersionList() const {
    return const_cast<ZDvidVersionDag&>(*this).getVersionListRef();
  }

  const std::map<std::string, std::list<std::string> >& getParentMap() const {
    return const_cast<ZDvidVersionDag&>(*this).getParentMapRef();
  }

  const ZTree<ZDvidVersionNode>& getTree() const {
    return const_cast<ZDvidVersionDag&>(*this).getTreeRef();
  }

  std::map<std::string, ZTreeNode<ZDvidVersionNode>* >& getVersionListRef() {
    return m_data->m_versionList;
  }
  std::map<std::string, std::list<std::string> >& getParentMapRef() {
    return m_data->m_parentMap;
  }
  ZTree<ZDvidVersionNode>& getTreeRef() { return m_data->m_tree; }

private:
  ZSharedPointer<ZDvidVersionDagData> m_data;
//  ZSharedPointer<ZTree<ZDvidVersionNode> > m_tree;
//  std::map<std::string, ZTreeNode<ZDvidVersionNode>* > m_versionList;
//  std::map<std::string, std::list<std::string> > m_parentMap;
};

#endif // ZDVIDVERSIONDAG_H
