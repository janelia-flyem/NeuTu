#include "zdocplayer.h"
#include "zstack.hxx"
#include "zstroke2d.h"
#include "zsparseobject.h"
#include "zobject3d.h"
#include "zswcgenerator.h"
#include "zswctree.h"
#include "zjsonobject.h"
#include "zstackball.h"
#include "swctreenode.h"
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidsparsevolslice.h"

ZDocPlayer::~ZDocPlayer()
{
  m_data = NULL;
}

ZDocPlayer::ZDocPlayer(ZStackObject *data)
{
  m_data = data;
  m_enableUpdate = true;
}

bool ZDocPlayer::hasData(ZStackObject *data) const
{
  if (data == NULL) {
    return false;
  }

  return (m_data == data);
}

bool ZDocPlayer::hasRole(ZStackObjectRole::TRole role) const
{
  if (m_data != NULL) {
    return m_data->hasRole(role);
  }

  return false;
}

bool ZDocPlayer::isEmpty() const
{
  return m_data == NULL;
}

ZJsonObject ZDocPlayer::toJsonObject() const
{
  return ZJsonObject();
}

/*************************************/
ZDocPlayerList::~ZDocPlayerList()
{
#ifdef _DEBUG_2
  print();
#endif
  clear();
}

void ZDocPlayerList::clearUnsync()
{
  //foreach won't work here
  for (QList<ZDocPlayer*>::const_iterator iter = m_playerList.begin();
       iter != m_playerList.end(); ++iter) {
    ZDocPlayer *player = *iter;
    delete player;
  }

  m_playerList.clear();
}

void ZDocPlayerList::clear()
{
  QMutexLocker locker(&m_mutex);

  clearUnsync();
}

void ZDocPlayerList::moveTo(ZDocPlayerList &playerList)
{
  QMutexLocker locker(&m_mutex);
  QMutexLocker locker2(playerList.getMutex());

  playerList.getPlayerList().append(getPlayerList());

  getPlayerList().clear();
}

void ZDocPlayerList::addUnsync(ZDocPlayer *data)
{
  m_playerList.append(data);
}

void ZDocPlayerList::add(ZDocPlayer *data)
{
  QMutexLocker locker(&m_mutex);

  addUnsync(data);
}

QList<ZDocPlayer*> ZDocPlayerList::takePlayerUnsync(ZStackObject *data)
{
  QList<ZDocPlayer*> playerList;
  QList<ZDocPlayer*>::iterator iter = m_playerList.begin();
  while (iter != m_playerList.end()) {
    ZDocPlayer *player = *iter;
    if (player->hasData(data)) {
      //role |= player->getRole();
      iter = m_playerList.erase(iter);
      playerList.append(player);
    } else {
      ++iter;
    }
  }

  return playerList;
}

QList<ZDocPlayer*> ZDocPlayerList::takePlayer(ZStackObject *data)
{
  QMutexLocker locker(&m_mutex);

  return takePlayerUnsync(data);
}

ZStackObjectRole::TRole ZDocPlayerList::removePlayerUnsync(ZStackObject *data)
{
  ZStackObjectRole::TRole role = ZStackObjectRole::ROLE_NONE;
  if (data != NULL) {
    role = data->getRole().getRole();

    QList<ZDocPlayer*>::iterator iter = m_playerList.begin();
    while (iter != m_playerList.end()) {
      ZDocPlayer *player = *iter;
      if (player->hasData(data)) {
//        role |= player->getRole();
        iter = m_playerList.erase(iter);
        delete player;
      } else {
        ++iter;
      }
    }
  }

  return role;
}

ZStackObjectRole::TRole ZDocPlayerList::removePlayer(ZStackObject *data)
{
  QMutexLocker locker(&m_mutex);

  return removePlayerUnsync(data);
}

ZStackObjectRole::TRole ZDocPlayerList::removePlayerUnsync(
    ZStackObjectRole::TRole role)
{
  ZStackObjectRole roleObj;
  QList<ZDocPlayer*>::iterator iter = m_playerList.begin();
  while (iter != m_playerList.end()) {
    ZDocPlayer *player = *iter;
    if (player->hasRole(role)) {
      roleObj.addRole(player->getRole());
      iter = m_playerList.erase(iter);
      delete player;
    } else {
      ++iter;
    }
  }

  return roleObj.getRole();
}

ZStackObjectRole::TRole ZDocPlayerList::removePlayer(
    ZStackObjectRole::TRole role)
{
  QMutexLocker locker(&m_mutex);

  return removePlayerUnsync(role);
}

ZStackObjectRole::TRole ZDocPlayerList::removeAllUnsync()
{
  ZStackObjectRole roleObj;
  QList<ZDocPlayer*>::iterator iter = m_playerList.begin();
  while (iter != m_playerList.end()) {
    ZDocPlayer *player = *iter;
    roleObj.addRole(player->getRole());
    delete player;
    ++iter;
  }

  m_playerList.clear();

  return roleObj.getRole();
}


ZStackObjectRole::TRole ZDocPlayerList::removeAll()
{
  QMutexLocker locker(&m_mutex);

  return removeAllUnsync();
}

QList<ZDocPlayer*> ZDocPlayerList::getPlayerListUnsync(
    ZStackObjectRole::TRole role)
{
  QList<ZDocPlayer*> playerList;

  for(QList<ZDocPlayer*>::iterator iter = m_playerList.begin();
      iter != m_playerList.end(); ++iter) {
    ZDocPlayer *player = *iter;
    if (player->hasRole(role)) {
      playerList.append(player);
    }
  }

  return playerList;
}

QList<ZDocPlayer*> ZDocPlayerList::getPlayerList(ZStackObjectRole::TRole role)
{
  QMutexLocker locker(&m_mutex);

  return getPlayerListUnsync(role);
}

QList<const ZDocPlayer*>
ZDocPlayerList::getPlayerListUnsync(ZStackObjectRole::TRole role) const
{
  QList<const ZDocPlayer*> playerList;

  for(QList<ZDocPlayer*>::const_iterator iter = m_playerList.begin();
      iter != m_playerList.end(); ++iter) {
    ZDocPlayer *player = *iter;
    if (player->hasRole(role)) {
      playerList.append(player);
    }
  }

  return playerList;
}

QList<const ZDocPlayer*>
ZDocPlayerList::getPlayerList(ZStackObjectRole::TRole role) const
{
  QMutexLocker locker(&m_mutex);

  return getPlayerListUnsync(role);
}

bool ZDocPlayerList::hasPlayerUnsync(ZStackObjectRole::TRole role) const
{
  for(QList<ZDocPlayer*>::const_iterator iter = m_playerList.begin();
      iter != m_playerList.end(); ++iter) {
    ZDocPlayer *player = *iter;
    if (player->hasRole(role)) {
      return true;
    }
  }

  return false;
}

bool ZDocPlayerList::hasPlayer(ZStackObjectRole::TRole role) const
{
  QMutexLocker locker(&m_mutex);

  return hasPlayerUnsync(role);
}

void ZDocPlayerList::printUnsync() const
{
  std::cout << size() << " players." << std::endl;
  for (QList<ZDocPlayer*>::const_iterator iter = m_playerList.begin();
       iter != m_playerList.end(); ++iter) {
    ZDocPlayer *player = *iter;
    std::cout << player->getData()->className() << std::endl;
  }
}

void ZDocPlayerList::print() const
{
  QMutexLocker locker(&m_mutex);

  printUnsync();
}


/*************************************/
ZStroke2dPlayer::ZStroke2dPlayer(ZStackObject *data) :
  ZDocPlayer(data)
{
}


ZStroke2d* ZStroke2dPlayer::getCompleteData() const
{
  return dynamic_cast<ZStroke2d*>(m_data);
}

void ZStroke2dPlayer::labelStack(ZStack* stack) const
{
  ZStroke2d *stroke = getCompleteData();

  if (stroke != NULL) {
    stroke->labelStack(stack);
  }
}

int ZStroke2dPlayer::getLabel() const
{
  ZStroke2d *stroke = getCompleteData();

  if (stroke != NULL) {
    return stroke->getLabel();
  }

  return 0;
}

void ZStroke2dPlayer::setLabel(int label)
{
  ZStroke2d *stroke = getCompleteData();

  if (stroke != NULL) {
    stroke->setLabel(label);
  }
}

QString ZStroke2dPlayer::getTypeName() const
{
  return "Plane Stroke";
}

ZJsonObject ZStroke2dPlayer::toJsonObject() const
{
  return getCompleteData()->toJsonObject();
}

Z3DGraph ZStroke2dPlayer::get3DGraph() const
{
  Z3DGraph graph;

  ZStroke2d *stroke = getCompleteData();
  if (stroke != NULL) {
    if (!stroke->isEmpty()) {
      double z = stroke->getZ();
      double radius = stroke->getWidth() / 2.0;
      for (size_t i = 0; i < stroke->getPointNumber(); ++i) {
        double x = 0;
        double y = 0;
        stroke->getPoint(&x, &y, i);
        ZStackBall stackBall(x, y, z, radius);
        stackBall.setColor(stroke->getColor());
        graph.addNode(stackBall);
//        graph.addNode(x, y, z, radius);
      }
    }
  }

  return graph;
}

ZSwcTree* ZStroke2dPlayer::getSwcDecoration() const
{
  ZSwcTree *tree = NULL;
  ZStroke2d *stroke = getCompleteData();
  if (stroke != NULL) {
    if (!stroke->isEmpty()) {
      tree->forceVirtualRoot();
      Swc_Tree_Node *parent = tree->root();
      double z = stroke->getZ();
      double radius = stroke->getWidth() / 2.0;
      for (size_t i = 0; i < stroke->getPointNumber(); ++i) {
        double x = 0;
        double y = 0;
        stroke->getPoint(&x, &y, i);
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(x, y, z, radius, parent);
        parent = tn;
      }
    }
  }
  return tree;
}

ZStack* ZStroke2dPlayer::toStack() const
{
  ZStroke2d *stroke = getCompleteData();

  if (stroke != NULL) {
    return stroke->toStack();
  }

  return NULL;
}

/*************************************/
ZObject3dPlayer::ZObject3dPlayer(ZStackObject *data) :
  ZDocPlayer(data)
{
}


const ZObject3d* ZObject3dPlayer::getCompleteData() const
{
  return dynamic_cast<const ZObject3d*>(m_data);
}

ZObject3d* ZObject3dPlayer::getCompleteData()
{
  return dynamic_cast<ZObject3d*>(m_data);
}

void ZObject3dPlayer::labelStack(ZStack *stack, int value) const
{
  const ZObject3d *obj = getCompleteData();

  if (obj != NULL) {
    obj->labelStack(stack, value);
  }
}

void ZObject3dPlayer::labelStack(Stack *stack, int *offset, int value) const
{
  const ZObject3d *obj = getCompleteData();

  if (obj != NULL) {
    if (offset == NULL) {
      obj->labelStack(stack, value);
    } else {
      obj->labelStack(stack, value, -offset[0], -offset[1], -offset[2]);
    }
  }
}

void ZObject3dPlayer::labelStack(Stack *stack, int *offset, int value,
                                 int xIntv, int yIntv, int zIntv) const
{
  const ZObject3d *obj = getCompleteData();

  if (obj != NULL) {
    if (offset == NULL) {
      obj->labelStack(stack, value, 0, 0, 0, xIntv, yIntv, zIntv);
    } else {
      obj->labelStack(stack, value, offset[0], offset[1], offset[2],
          xIntv, yIntv, zIntv);
    }
  }
}


void ZObject3dPlayer::labelStack(ZStack* stack) const
{
  const ZObject3d *obj = getCompleteData();

  if (obj != NULL) {
    obj->labelStack(stack);
  }
}

ZStack* ZObject3dPlayer::toStack() const
{
  const ZObject3d *obj = getCompleteData();

  if (obj != NULL) {
    return obj->toLabelStack();
  }

  return NULL;
}

void ZObject3dPlayer::paintStack(ZStack *stack) const
{
  const ZObject3d *obj = getCompleteData();

  if (obj != NULL) {
    obj->drawStack(stack);
  }
}

void ZObject3dPlayer::paintStack(
    const std::vector<Stack*> &stackArray, const int *offset,
    int xIntv, int yIntv, int zIntv) const
{
  const ZObject3d *obj = getCompleteData();

  if (obj != NULL) {
    obj->drawStack(stackArray, offset, xIntv, yIntv, zIntv);
  }
}

int ZObject3dPlayer::getLabel() const
{
  const ZObject3d *obj = getCompleteData();

  if (obj != NULL) {
    return obj->getLabel();
  }

  return 0;
}

void ZObject3dPlayer::setLabel(int label)
{
  ZObject3d *obj = getCompleteData();

  if (obj != NULL) {
    obj->setLabel(label);
  }
}

ZSwcTree* ZObject3dPlayer::getSwcDecoration() const
{
  const ZObject3d *obj = getCompleteData();

  ZSwcTree *tree = NULL;
  if (obj != NULL) {
    if (!obj->isEmpty()) {
      tree = ZSwcGenerator::createSwc(*obj, 1.0, 3);
      tree->setColor(obj->getColor());
    }
  }

  return tree;
}

Z3DGraph ZObject3dPlayer::get3DGraph() const
{
  Z3DGraph graph;

  const ZObject3d *obj = getCompleteData();
  if (obj != NULL) {
    graph.importObject3d(*obj, 1.0);
  }

  return graph;
}

ZJsonObject ZObject3dPlayer::toJsonObject() const
{
  return getCompleteData()->toJsonObject();
}

/*************************************/
ZObject3dScanPlayer::ZObject3dScanPlayer(ZStackObject *data) :
  ZDocPlayer(data)
{
}


const ZObject3dScan* ZObject3dScanPlayer::getCompleteData() const
{
  return dynamic_cast<const ZObject3dScan*>(m_data);
}

/*************************************/
ZSparseObjectPlayer::ZSparseObjectPlayer(ZStackObject *data) :
  ZDocPlayer(data)
{
}


ZSparseObject* ZSparseObjectPlayer::getCompleteData() const
{
  return dynamic_cast<ZSparseObject*>(m_data);
}

void ZSparseObjectPlayer::labelStack(ZStack* stack) const
{
  ZSparseObject *obj = getCompleteData();

  if (obj != NULL) {
    obj->labelStack(stack);
  }
}

ZStack* ZSparseObjectPlayer::toStack() const
{
  const ZSparseObject *obj = getCompleteData();

  if (obj != NULL) {
    return obj->toStackObject();
  }

  return NULL;
}

/*******************************/
ZStackBallPlayer::ZStackBallPlayer(ZStackObject *data) :
  ZDocPlayer(data)
{
}

ZStackBall* ZStackBallPlayer::getCompleteData() const
{
  return dynamic_cast<ZStackBall*>(m_data);
}

Z3DGraph ZStackBallPlayer::get3DGraph() const
{
  Z3DGraph graph;

  const ZStackBall *obj = getCompleteData();
  if (obj != NULL) {
    graph.addNode(*obj);
  }

  return graph;
}

ZDvidLabelSlicePlayer::ZDvidLabelSlicePlayer(ZStackObject *data) :
  ZDocPlayer(data)
{
}

ZDvidLabelSlice* ZDvidLabelSlicePlayer::getCompleteData() const
{
  return dynamic_cast<ZDvidLabelSlice*>(m_data);
}

bool ZDvidLabelSlicePlayer::updateData(const ZStackViewParam &viewParam) const
{
  bool updated = false;
  if (m_enableUpdate) {
    ZDvidLabelSlice *obj = getCompleteData();
    if (obj != NULL) {
      if (obj->isVisible()) {
        updated = obj->update(viewParam);
      }
    }
  }

  return updated;
}

/////////////////////////////

ZDvidSparsevolSlicePlayer::ZDvidSparsevolSlicePlayer(ZStackObject *data) :
  ZDocPlayer(data)
{
}

ZDvidSparsevolSlice* ZDvidSparsevolSlicePlayer::getCompleteData() const
{
  return dynamic_cast<ZDvidSparsevolSlice*>(m_data);
}

bool ZDvidSparsevolSlicePlayer::updateData(const ZStackViewParam &viewParam) const
{
  bool updated = false;
  if (m_enableUpdate) {
    ZDvidSparsevolSlice *obj = getCompleteData();
    if (obj != NULL) {
      updated = obj->update(viewParam.getZ());
    }
  }

  return updated;
}

////////////////////////////

Z3DGraph ZCuboidRoiPlayer::get3DGraph() const
{
  Z3DGraph graph;

  if (getData() != NULL) {

  }

  return graph;
}


