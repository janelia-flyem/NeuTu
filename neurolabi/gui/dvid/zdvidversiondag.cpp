#include "zdvidversiondag.h"
#include <iostream>
#include <queue>

#include "zjsonobject.h"
#include "zstring.h"
#include "zjsonparser.h"
#include "zerror.h"

ZDvidVersionDag::ZDvidVersionDag()
{
  m_data = ZSharedPointer<ZDvidVersionDagData>(new ZDvidVersionDagData);
//  m_tree =
//      ZSharedPointer<ZTree<ZDvidVersionNode> >(new ZTree<ZDvidVersionNode>);
}


void ZDvidVersionDag::clear()
{
  getTreeRef().clear();
  getVersionListRef().clear();
  getParentMapRef().clear();
}

bool ZDvidVersionDag::isLocked(const std::string &uuid) const
{
  return getNode(uuid).isLocked();
}

bool ZDvidVersionDag::isActive(const std::string &uuid) const
{
  return getNode(uuid).isActive();
}

bool ZDvidVersionDag::hasNode(const std::string &uuid) const
{
  return getVersionList().count(uuid) > 0;
}

ZDvidVersionNode ZDvidVersionDag::getNode(const std::string &uuid) const
{
  const ZTreeNode<ZDvidVersionNode> *tn = getDagNode(uuid);

  if (tn == NULL) {
    return ZDvidVersionNode();
  }

  return tn->data();
}

const ZTreeNode<ZDvidVersionNode>*
ZDvidVersionDag::getDagNode(const std::string &uuid) const
{
  if (getVersionList().count(uuid) == 0) {
    return NULL;
  }

  return getVersionList().at(uuid);
}

ZTreeNode<ZDvidVersionNode>*
ZDvidVersionDag::getDagNode(const std::string &uuid)
{
  return const_cast<ZTreeNode<ZDvidVersionNode>*>(
        static_cast<const ZDvidVersionDag&>(*this).getDagNode(uuid));
}

std::vector<std::string>
ZDvidVersionDag::getParentList(const std::string &uuid) const
{
  std::vector<std::string> uuidList;

  if (getParentMap().count(uuid) > 0) {
    std::list<std::string> parentList = getParentMap().at(uuid);
    uuidList.insert(uuidList.begin(), parentList.begin(), parentList.end());
  }

  return uuidList;
}

std::string ZDvidVersionDag::getFirstParent(const std::string &uuid) const
{
  std::string parentUuid;
  const ZTreeNode<ZDvidVersionNode> *tn = getDagNode(uuid);
  if (tn != NULL) {
    ZTreeNode<ZDvidVersionNode> *parentTn = tn->parent();
    if (parentTn != NULL) {
      parentUuid = parentTn->data().getUuid();
    }
  }

  return parentUuid;
}

std::vector<std::string> ZDvidVersionDag::getChildList(const std::string &uuid) const
{
  std::vector<std::string> uuidList;

  const ZTreeNode<ZDvidVersionNode> *tn = getDagNode(uuid);
  if (tn != NULL) {
    const ZTreeNode<ZDvidVersionNode> *child = tn->firstChild();
    while (child != NULL) {
      uuidList.push_back(child->data().getUuid());
      child = child->nextSibling();
    }
  }

  return uuidList;
}

int ZDvidVersionDag::getSiblingIndex(const std::string &uuid) const
{
  int index = -1;
  const ZTreeNode<ZDvidVersionNode> *tn = getDagNode(uuid);
  if (tn != NULL) {
    index = 0;
    if (!tn->isRoot()) {
      const ZTreeNode<ZDvidVersionNode> *parent = tn->parent();
      const ZTreeNode<ZDvidVersionNode> *child = parent->firstChild();
      while (child != NULL) {
        if (child == tn) {
          break;
        }
        child = child->nextSibling();
        ++index;
      }
    }
  }

  return index;
}

std::string ZDvidVersionDag::getRoot() const
{
  std::string uuid;
  if (!getTree().isEmpty()) {
    uuid = getTree().getRoot()->data().getUuid();
  }

  return uuid;
}

std::string ZDvidVersionDag::getChild(const std::string uuid, int index) const
{
  std::string childUuid;
  if (index >= 0) {
    const ZTreeNode<ZDvidVersionNode> *tn = getDagNode(uuid);
    if (tn != NULL) {
      ZTreeNode<ZDvidVersionNode> *childTn = tn->getChild(index);
      if (childTn != NULL) {
        childUuid = childTn->data().getUuid();
      }
    }
  }

  return childUuid;
}

void ZDvidVersionDag::setRoot(const std::string &uuid)
{
  if (!uuid.empty()) {
    clear();

    ZTreeNode<ZDvidVersionNode> *root = addNode(uuid);

    getTreeRef().setRoot(root, true);
  }
}

bool ZDvidVersionDag::isParent(
    const std::string &uuid, const std::string &parentUuid) const
{
  if (!uuid.empty() && !parentUuid.empty()) {
    if (getParentMap().count(uuid) > 0) {
      const std::list<std::string> &parentList = getParentMap().at(uuid);
      if (!parentList.empty()) {
        return std::find(parentList.begin(), parentList.end(), parentUuid) !=
            parentList.end();
      }
    }
  }

  return false;
}

ZTreeNode<ZDvidVersionNode>* ZDvidVersionDag::addNode(const std::string &uuid)
{
  ZTreeNode<ZDvidVersionNode> *tn = NULL;
  if (!uuid.empty() && getVersionList().count(uuid) == 0) {
    tn = new ZTreeNode<ZDvidVersionNode>;
    tn->data().setUuid(uuid);
    getVersionListRef()[uuid] = tn;
  }

  return tn;
}

bool ZDvidVersionDag::addNode(
    const std::string &uuid, const std::string &parentUuid)
{
  bool succ = false;

  if (!uuid.empty() && !parentUuid.empty()) {
    if (!isParent(uuid, parentUuid) && !isParent(parentUuid, uuid)) {
      ZTreeNode<ZDvidVersionNode> *parent = getDagNode(parentUuid);
      if (parent != NULL) {
        ZTreeNode<ZDvidVersionNode> *tn = getDagNode(uuid);
        if (tn == NULL) {
          tn = addNode(uuid);
        }

        if (tn->isRoot()) {
          tn->setParent(parent);
          std::list<std::string> newParentList;
          newParentList.push_back(parentUuid);
          getParentMapRef()[uuid] = newParentList;
        } else {
          std::list<std::string> &parentList = getParentMapRef()[uuid];
          parentList.push_back(parentUuid);
        }
        succ = true;
      }
    }
  }

  return succ;
}

void ZDvidVersionDag::print() const
{
  const std::map<std::string, std::list<std::string> > &parentMap =
      getParentMap();
  for (std::map<std::string, std::list<std::string> >::const_iterator
       iter = parentMap.begin(); iter != parentMap.end(); ++iter) {
    const std::string &uuid = iter->first;
    const std::list<std::string> &parentList = iter->second;
    std::cout << uuid << ":";
    for (std::list<std::string>::const_iterator iter = parentList.begin();
         iter != parentList.end(); ++iter) {
      std::cout << *iter << " ";
    }
    std::cout << std::endl;
  }
}

void ZDvidVersionDag::load(const ZJsonObject &obj, const std::string &uuid)
{
  clear();

  std::vector<std::string> keyList = obj.getAllKey();
  std::string fullUuid;
  for (std::vector<std::string>::const_iterator iter = keyList.begin();
       iter != keyList.end(); ++iter) {
    ZString key = *iter;
    if (key.startsWith(uuid)) {
      fullUuid = key;
      break;
    }
  }

  ZJsonObject dagJson = ZJsonObject(ZJsonObject(obj.value(fullUuid.c_str())).value("DAG"));
  if (!dagJson.isEmpty()) {
    std::queue<std::string> uuidQueue;

    std::string rootUuid = ZJsonParser::stringValue(dagJson["Root"]);

    RECORD_WARNING(rootUuid.empty(), "empty root uuid");

    uuidQueue.push(rootUuid);
    setRoot(rootUuid.substr(0, 4));

    ZJsonObject allNodeJson(dagJson.value("Nodes"));

    std::vector<ZDvidVersionNode> nodeList; //sorted uuid list

    const char* key;
    json_t *value;
    ZJsonObject_foreach (allNodeJson, key, value) {
      ZJsonObject nodeJson(value, ZJsonValue::SET_INCREASE_REF_COUNT);
      int versionId = ZJsonParser::integerValue(nodeJson["VersionID"]);
      if (versionId >= (int) nodeList.size()) {
        nodeList.resize(versionId + 1);
      }
      ZDvidVersionNode node;
      node.setUuid(key);
      if (ZJsonParser::booleanValue(nodeJson["Locked"])) {
        node.lock();
      }
      nodeList[versionId] = node;
    }

    while (!uuidQueue.empty()) {
      std::string nextUuid = uuidQueue.front();
      uuidQueue.pop();

      ZJsonObject uuidJson(allNodeJson.value(nextUuid.c_str()));
      std::vector<int> versionIdArray =
          ZJsonParser::integerArray(uuidJson["Children"]);
      for (size_t i = 0; i < versionIdArray.size(); ++i) {
        int versionId = versionIdArray[i];
        std::string childUuid = nodeList[versionId].getUuid();
        if (!hasNode(childUuid)) {
          uuidQueue.push(childUuid);
        }
        addNode(childUuid.substr(0, 4), nextUuid.substr(0, 4));
      }
    }

    for (std::vector<ZDvidVersionNode>::const_iterator iter = nodeList.begin();
         iter != nodeList.end(); ++iter) {
      const ZDvidVersionNode &node = *iter;
      if (node.isLocked()) {
        lock(node.getUuid().substr(0, 4));
      }
    }
  }
}

void ZDvidVersionDag::lock(const std::string &uuid)
{
  ZTreeNode<ZDvidVersionNode> *tn = getDagNode(uuid);
  if (tn != NULL) {
    tn->data().lock();
  }
}

void ZDvidVersionDag::unlock(const std::string &uuid)
{
  ZTreeNode<ZDvidVersionNode> *tn = getDagNode(uuid);
  if (tn != NULL) {
    tn->data().unlock();
  }
}

void ZDvidVersionDag::activateNode(const std::string &uuid)
{
  ZTreeNode<ZDvidVersionNode> *tn = getDagNode(uuid);
  if (tn != NULL) {
    tn->data().activate();
  }
}

void ZDvidVersionDag::deactivateNode(const std::string &uuid)
{
  ZTreeNode<ZDvidVersionNode> *tn = getDagNode(uuid);
  if (tn != NULL) {
    tn->data().deactivate();
  }
}


std::vector<std::string> ZDvidVersionDag::getBreadthFirstList() const
{
  std::vector<std::string> uuidList;

  ZTree<ZDvidVersionNode> &tree =
      const_cast<ZDvidVersionDag&>(*this).getTreeRef();

  ZTreeIterator<ZDvidVersionNode> iterator(
        tree, ZTreeIterator<ZDvidVersionNode>::BREADTH_FIRST);
  while (iterator.hasNext()) {
    ZDvidVersionNode& node = iterator.next();
    uuidList.push_back(node.getUuid());
  }

  return uuidList;
}

bool ZDvidVersionDag::isEmpty() const
{
  return getVersionList().empty();
}
