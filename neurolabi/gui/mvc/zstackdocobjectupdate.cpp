#include "zstackdocobjectupdate.h"


#include <iostream>

#include "zstackobject.h"

ZStackDocObjectUpdate::ZStackDocObjectUpdate(ZStackObject *obj, EAction action)
{
  m_obj = obj;
  m_action = action;

  if (m_action == EAction::ADD_NONUNIQUE ||
      m_action == EAction::ADD_UNIQUE ||
      m_action == EAction::ADD_BUFFER) { //leftover objects
    m_dispose = [&]() {
      delete m_obj;
    };
  }
}

ZStackDocObjectUpdate::~ZStackDocObjectUpdate()
{
  cleanUp();
}

void ZStackDocObjectUpdate::setCallback(std::function<void (ZStackObject *)> f)
{
  m_callback = f;
}

void ZStackDocObjectUpdate::setCleanup(std::function<void ()> f)
{
  m_dispose = f;
}

void ZStackDocObjectUpdate::reset()
{
  m_obj = NULL;
  m_action = EAction::NONE;
  m_callback = nullptr;
  m_dispose = nullptr;
}

void ZStackDocObjectUpdate::cleanUp()
{
  if (m_dispose) {
    m_dispose();
  }
  reset();
}

void ZStackDocObjectUpdate::apply(
    std::function<void (ZStackObject *, EAction)> f)
{
  if (f) {
    f(getObject(), getAction());
    reset();
  } else {
    if (m_callback) {
      m_callback(getObject());
      reset();
    }
  }
}

void ZStackDocObjectUpdate::print() const
{
  switch (m_action) {
  case EAction::ADD_NONUNIQUE:
    std::cout << "Add nonunique";
    break;
  case EAction::ADD_UNIQUE:
    std::cout << "Add unique";
    break;
  case EAction::EXPEL:
    std::cout << "Expel";
    break;
  case EAction::KILL:
    std::cout << "Kill";
    break;
  case EAction::NONE:
    std::cout << "No action on";
    break;
  case EAction::RECYCLE:
    std::cout << "Recycle";
    break;
  case EAction::UPDATE:
    std::cout << "Update";
    break;
  case EAction::SELECT:
    std::cout << "Select";
    break;
  case EAction::DESELECT:
    std::cout << "Deselect";
    break;
  case EAction::ADD_BUFFER:
    std::cout << "Add to buffer";
    break;
  case EAction::CALLBACK:
    std::cout << "Callback";
    break;
  }

  std::cout << " " << ZStackObject::GetTypeName(m_obj->getType()) << " "
            << m_obj->getSource() << " " << m_obj << std::endl;
}

bool ZStackDocObjectUpdate::isMergable() const
{
  return  getAction() != EAction::CALLBACK;
}


//////////////////ZStackDocObjectUpdateFactory///////////////////////

ZStackDocObjectUpdate* ZStackDocObjectUpdateFactory::Make(
    std::function<void (ZStackObject *)> f)
{
  ZStackDocObjectUpdate *u = new ZStackDocObjectUpdate(
        nullptr, ZStackDocObjectUpdate::EAction::CALLBACK);
  u->setCallback(f);

  return u;
}

ZStackDocObjectUpdate* ZStackDocObjectUpdateFactory::Make(
    std::function<void()> f)
{
  ZStackDocObjectUpdate *u = new ZStackDocObjectUpdate(
        nullptr, ZStackDocObjectUpdate::EAction::CALLBACK);
  u->setCallback([f](ZStackObject*) {
    f();
  });

  return u;
}
