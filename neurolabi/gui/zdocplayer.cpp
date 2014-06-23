#include "zdocplayer.h"
#include "zstack.hxx"
#include "zstroke2d.h"
#include "zsparseobject.h"
#include "zobject3d.h"

const ZDocPlayer::TRole ZDocPlayer::ROLE_NONE = 0;
const ZDocPlayer::TRole ZDocPlayer::ROLE_DISPLAY = 1;
const ZDocPlayer::TRole ZDocPlayer::ROLE_SEED = 2;
const ZDocPlayer::TRole ZDocPlayer::ROLE_TMP_RESULT = 4;
const ZDocPlayer::TRole ZDocPlayer::ROLE_3DPAINT = 8;
const ZDocPlayer::TRole ZDocPlayer::ROLE_MANAGED_OBJECT = 16;

ZDocPlayer::ZDocPlayer() : m_data(NULL), m_role(ZDocPlayer::ROLE_NONE)
{
}

ZDocPlayer::~ZDocPlayer()
{
  m_data = NULL;
  m_role = ZDocPlayer::ROLE_NONE;
}

ZDocPlayer::ZDocPlayer(ZDocumentable* data, TRole role)
{
  m_data = data;
  m_role = role;
}

bool ZDocPlayer::hasData(ZDocumentable *data) const
{
  if (data == NULL) {
    return false;
  }

  return (m_data == data);
}

bool ZDocPlayer::hasRole(TRole role) const
{
  if (role == ROLE_NONE) {
    return (m_role == ROLE_NONE);
  }

  return role == (m_role & role);
  //return (m_role == role) || ((m_role & role) > 0);
}

bool ZDocPlayer::isEmpty() const
{
  return m_data == NULL;
}

/*************************************/
ZDocPlayerList::~ZDocPlayerList()
{
#ifdef _DEBUG_2
  print();
#endif

  //foreach won't work here
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    ZDocPlayer *player = *iter;
    delete player;
  }
  clear();
}

ZDocPlayer::TRole ZDocPlayerList::removePlayer(ZDocumentable *data)
{
  ZDocPlayer::TRole role = ZDocPlayer::ROLE_NONE;

  ZDocPlayerList::iterator iter = begin();
  while (iter != end()) {
    ZDocPlayer *player = *iter;
    if (player->hasData(data)) {
      role |= player->getRole();
      iter = erase(iter);
      delete player;
    } else {
      ++iter;
    }
  }

  return role;
}

ZDocPlayer::TRole ZDocPlayerList::removePlayer(ZDocPlayer::TRole role)
{
  ZDocPlayer::TRole result = ZDocPlayer::ROLE_NONE;

  ZDocPlayerList::iterator iter = begin();
  while (iter != end()) {
    ZDocPlayer *player = *iter;
    if (player->hasRole(role)) {
      result |= player->getRole();
      iter = erase(iter);
      delete player;
    } else {
      ++iter;
    }
  }

  return result;
}

QList<ZDocPlayer*> ZDocPlayerList::getPlayerList(ZDocPlayer::TRole role)
{
  QList<ZDocPlayer*> playerList;

  for(ZDocPlayerList::iterator iter = begin(); iter != end(); ++iter) {
    ZDocPlayer *player = *iter;
    if (player->hasRole(role)) {
      playerList.append(player);
    }
  }

  return playerList;
}

QList<const ZDocPlayer*>
ZDocPlayerList::getPlayerList(ZDocPlayer::TRole role) const
{
  QList<const ZDocPlayer*> playerList;

  for(ZDocPlayerList::const_iterator iter = begin(); iter != end(); ++iter) {
    ZDocPlayer *player = *iter;
    if (player->hasRole(role)) {
      playerList.append(player);
    }
  }

  return playerList;
}

bool ZDocPlayerList::hasPlayer(ZDocPlayer::TRole role) const
{
  for(ZDocPlayerList::const_iterator iter = begin(); iter != end(); ++iter) {
    ZDocPlayer *player = *iter;
    if (player->hasRole(role)) {
      return true;
    }
  }

  return false;
}

void ZDocPlayerList::print() const
{
  std::cout << size() << " players." << std::endl;
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    ZDocPlayer *player = *iter;
    std::cout << player->getData()->className() << std::endl;
  }
}


/*************************************/

ZStroke2dPlayer::ZStroke2dPlayer() : ZDocPlayer()
{
}

ZStroke2dPlayer::ZStroke2dPlayer(ZDocumentable *data, TRole role) :
  ZDocPlayer(data, role)
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

ZStack* ZStroke2dPlayer::toStack() const
{
  ZStroke2d *stroke = getCompleteData();

  if (stroke != NULL) {
    return stroke->toStack();
  }

  return NULL;
}

/*************************************/

ZObject3dPlayer::ZObject3dPlayer() : ZDocPlayer()
{
}

ZObject3dPlayer::ZObject3dPlayer(ZDocumentable *data, TRole role) :
  ZDocPlayer(data, role)
{
}


const ZObject3d* ZObject3dPlayer::getCompleteData() const
{
  return dynamic_cast<const ZObject3d*>(m_data);
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

/*************************************/

ZSparseObjectPlayer::ZSparseObjectPlayer() : ZDocPlayer()
{
}

ZSparseObjectPlayer::ZSparseObjectPlayer(ZDocumentable *data, TRole role) :
  ZDocPlayer(data, role)
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
