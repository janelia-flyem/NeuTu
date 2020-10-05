#include "zmousecursorglyph.h"

#include <iostream>

#include "geometry/zpoint.h"
#include "zweightedpoint.h"
#include "zswctree.h"
#include "zstroke2d.h"
#include "zstackball.h"

ZMouseCursorGlyph::ZMouseCursorGlyph()
{
  init();
}

ZMouseCursorGlyph::~ZMouseCursorGlyph()
{
  m_prepare.clear();
  foreach (auto g, m_glyphMap) {
    delete g;
  }
  m_activeGlyph = nullptr;
  m_prevActiveGlyph = nullptr;
  m_glyphMap.clear();
}


void ZMouseCursorGlyph::init()
{
  m_defaultGlyphSize[ROLE_STROKE] = 3.0;
  m_defaultGlyphSize[ROLE_SWC] = 5.0;
  m_defaultGlyphSize[ROLE_SYNAPSE] = 5.0;
  m_defaultGlyphSize[ROLE_BOOKMARK] = 5.0;
  m_defaultGlyphSize[ROLE_TODO_ITEM] = 5.0;

  {
    ZStroke2d *stroke = new ZStroke2d;
    stroke->setVisible(false);
    stroke->setFilled(true);
    stroke->setPenetrating(true);
    stroke->useCosmeticPen(false);
    stroke->hideStart(false);
    stroke->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addGlyph(ROLE_STROKE, stroke);
  }

  {
    ZSwcTree *obj = new ZSwcTree;
    obj->setVisible(false);
    obj->useCosmeticPen(true);
    Swc_Tree_Node *tn = SwcTreeNode::MakePointer(
          0, 0, 0, m_defaultGlyphSize[ROLE_SWC]);
    obj->forceVirtualRoot();
    obj->addRegularRoot(tn);
    obj->setColor(0, 0, 255);
    obj->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addGlyph(ROLE_SWC, obj);
  }


  {
    ZStackBall *obj = new ZStackBall;
    obj->setVisible(false);
    obj->useCosmeticPen(true);
    obj->setRadius(m_defaultGlyphSize[ROLE_SYNAPSE]);
    obj->setColor(0, 255, 0);
    obj->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addGlyph(ROLE_SYNAPSE, obj);
  }

  {
    ZStackBall *obj = new ZStackBall;
    obj->setVisible(false);
    obj->useCosmeticPen(true);
    obj->setRadius(m_defaultGlyphSize[ROLE_BOOKMARK]);
    obj->setColor(255, 0, 0);
    obj->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addGlyph(ROLE_BOOKMARK, obj);
  }

  {
    ZStackBall *obj = new ZStackBall;
    obj->setVisible(false);
    obj->useCosmeticPen(true);
    obj->setRadius(m_defaultGlyphSize[ROLE_TODO_ITEM]);
    obj->setColor(255, 0, 0);
    obj->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addGlyph(ROLE_TODO_ITEM, obj);
  }
}

void ZMouseCursorGlyph::addGlyph(ERole role, ZStackObject *obj)
{
  if (obj) {
    if (m_glyphMap.contains(role)) {
      if (obj != m_glyphMap[role]) {
        if (obj == m_activeGlyph) {
          m_activeGlyph = nullptr;
        }
        if (obj == m_prevActiveGlyph) {
          m_prevActiveGlyph = nullptr;
        }
        delete m_glyphMap[role];
      }
    }

    m_glyphMap[role] = obj;
  }
}

std::string ZMouseCursorGlyph::GetRoleName(ERole role)
{
  switch (role) {
  case ROLE_STROKE:
    return "ROLE_STROKE";
  case ROLE_SWC:
    return "ROLE_SWC";
  case ROLE_SYNAPSE:
    return "ROLE_SYNAPSE";
  case ROLE_BOOKMARK:
    return "ROLE_BOOKMARK";
  case ROLE_TODO_ITEM:
    return "ROLE_TODO_ITEM";
  }

  return "";
}

QList<ZStackObject*> ZMouseCursorGlyph::getGlyphList() const
{
  QList<ZStackObject*> glyphList;
  foreach (auto g, m_glyphMap) {
    glyphList.append(g);
  }

  return glyphList;
}

ZStackObject* ZMouseCursorGlyph::getGlyph(ERole role) const
{
  return m_glyphMap.value(role, nullptr);
}

ZStackObject* ZMouseCursorGlyph::getActiveGlyph() const
{
  return m_activeGlyph;
}

bool ZMouseCursorGlyph::isActivated() const
{
  return m_activeGlyph;
}

bool ZMouseCursorGlyph::isActivated(ERole role) const
{
  if (m_activeGlyph) {
    return m_activeGlyph == getGlyph(role);
  }

  return false;
}

void ZMouseCursorGlyph::deactivate()
{
  if (m_activeGlyph) {
    m_prevActiveGlyph = m_activeGlyph;
    m_activeGlyph->setVisible(false);
    m_activeGlyph = nullptr;
  }
}

void ZMouseCursorGlyph::activate(ERole role)
{
  activate(role, m_prepare.value(role));
}

void ZMouseCursorGlyph::activate(
    ERole role, std::function<void(ZStackObject*)> prepare)
{
  deactivate();
  ZStackObject *obj = getGlyph(role);
  if (obj) {
    m_activeGlyph = obj;
    obj->setVisible(true);
    if (prepare) {
      prepare(obj);
    }
  }
}

void ZMouseCursorGlyph::activate(
    ERole role, std::function<void(ZStackObject*, ERole)> prepare)
{
  activate(role, [&](ZStackObject *obj) {
    prepare(obj, role);
  });
}

void ZMouseCursorGlyph::processActiveChange(
    std::function<void (ZStackObject *)> proc)
{
  proc(m_prevActiveGlyph);
  proc(m_activeGlyph);
}

void ZMouseCursorGlyph::processActiveChange(
    std::function<void (ZStackObject*, ZStackObject *)> proc)
{
  proc(m_prevActiveGlyph, m_activeGlyph);
}

void ZMouseCursorGlyph::setPrepareFunc(
    ERole role, std::function<void(ZStackObject *)> prepare)
{
  m_prepare[role] = prepare;
}

ZPoint ZMouseCursorGlyph::getActiveGlyphPosition() const
{
  if (m_activeGlyph) {
    switch (m_activeGlyph->getType()) {
    case ZStackObject::EType::STROKE:
      return dynamic_cast<ZStroke2d*>(m_activeGlyph)->getLastPoint();
    case ZStackObject::EType::STACK_BALL:
      return dynamic_cast<ZStackBall*>(m_activeGlyph)->getCenter();
    case ZStackObject::EType::SWC:
    {
      ZSwcTree *tree = dynamic_cast<ZSwcTree*>(m_activeGlyph);
      if (tree) {
        Swc_Tree_Node *tn = tree->firstRegularRoot();
        if (tn) {
          if (SwcTreeNode::hasChild(tn)) {
            tn = SwcTreeNode::firstChild(tn);
          }
          return SwcTreeNode::center(tn);
        }
      }
    }
      break;
    default:
      break;
    }
  }

  return ZPoint::INVALID_POINT;
}

double ZMouseCursorGlyph::getActiveGlyphSize() const
{
  if (m_activeGlyph) {
    switch (m_activeGlyph->getType()) {
    case ZStackObject::EType::STROKE:
      return dynamic_cast<ZStroke2d*>(m_activeGlyph)->getWidth();
    case ZStackObject::EType::STACK_BALL:
      return dynamic_cast<ZStackBall*>(m_activeGlyph)->getRadius();
    case ZStackObject::EType::SWC:
    {
      ZSwcTree *tree = dynamic_cast<ZSwcTree*>(m_activeGlyph);
      if (tree) {
        Swc_Tree_Node *tn = tree->firstRegularRoot();
        if (tn) {
          if (SwcTreeNode::hasChild(tn)) {
            tn = SwcTreeNode::firstChild(tn);
          }
          return SwcTreeNode::radius(tn);
        }
      }
    }
      break;
    default:
      break;
    }
  }

  return 0.0;
}

void ZMouseCursorGlyph::setActiveGlyphSize(
    double r, std::function<void(ZStackObject*)> postProc)
{
  if (m_activeGlyph) {
    ZStackObject *obj = m_activeGlyph;
    switch (obj->getType()) {
    case ZStackObject::EType::STROKE:
    {
      ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(obj);
      stroke->setWidth(r);
    }
      break;
    case ZStackObject::EType::STACK_BALL:
    {
      ZStackBall *ball = dynamic_cast<ZStackBall*>(obj);
      ball->setRadius(r);
    }
      break;
    case ZStackObject::EType::SWC:
    {
      ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
      if (tree) {
        Swc_Tree_Node *tn = tree->firstRegularRoot();
        if (tn) {
          if (SwcTreeNode::hasChild(tn)) {
            tn = SwcTreeNode::firstChild(tn);
          }
          SwcTreeNode::setRadius(tn, r);
          tree->deprecate(ZSwcTree::BOUND_BOX);
        }
      }
    }
      break;
    default:
      obj = nullptr;
      break;
    }

    if (postProc) {
      postProc(obj);
    }
  }
}

void ZMouseCursorGlyph::addActiveGlyphSize(
    double dr, std::function<void(ZStackObject*)> postProc)
{
  if (m_activeGlyph) {
    setActiveGlyphSize(getActiveGlyphSize() + dr, postProc);
  }
}

void ZMouseCursorGlyph::setActiveGlyphPosition(
    const ZPoint &pos, std::function<void(ZStackObject*)> postProc)
{
  if (m_activeGlyph) {
    ZStackObject *obj = m_activeGlyph;
    switch (obj->getType()) {
    case ZStackObject::EType::STROKE:
    {
      ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(obj);
      if (stroke) {
        stroke->updateWithLast(pos);
      }
    }
      break;
    case ZStackObject::EType::STACK_BALL:
    {
      ZStackBall *ball = dynamic_cast<ZStackBall*>(obj);
      if (ball) {
        ball->setCenter(pos.toIntPoint());
      }
    }
      break;
    case ZStackObject::EType::SWC:
    {
      ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
      if (tree) {
        Swc_Tree_Node *tn = tree->firstRegularRoot();
        if (tn) {
          if (SwcTreeNode::hasChild(tn)) {
            tn = SwcTreeNode::firstChild(tn);
          }
          SwcTreeNode::setCenter(tn, pos);
          tree->deprecate(ZSwcTree::BOUND_BOX);
        }
      }
    }
      break;
    default:
      obj = nullptr;
      break;
    }

    if (postProc) {
      postProc(obj);
    }
  }
}

void ZMouseCursorGlyph::appendActiveGlyphPosition(
    const ZPoint &pos, std::function<void(ZStackObject*)> postProc)
{
  if (m_activeGlyph) {
    ZStackObject *obj = m_activeGlyph;
    switch (obj->getType()) {
    case ZStackObject::EType::STROKE:
    {
      ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(obj);
      if (stroke) {
        stroke->append(pos);
      }
    }
      break;
    default:
      obj = nullptr;
      break;
    }

    if (postProc) {
      postProc(obj);
    }
  }
}

ZWeightedPoint ZMouseCursorGlyph::getActiveGlyphGeometry() const
{
  ZWeightedPoint pt(ZPoint::INVALID_POINT);
  if (m_activeGlyph) {
    switch (m_activeGlyph->getType()) {
    case ZStackObject::EType::STROKE:
    {
      ZStroke2d *obj = dynamic_cast<ZStroke2d*>(m_activeGlyph);
      pt.set(obj->getLastPoint(), obj->getWidth() * 0.5);
    }
      break;
    case ZStackObject::EType::STACK_BALL:
    {
      ZStackBall *obj = dynamic_cast<ZStackBall*>(m_activeGlyph);
      pt.set(obj->getCenter(), obj->getRadius());
    }
      break;
    case ZStackObject::EType::SWC:
    {
      ZSwcTree *tree = dynamic_cast<ZSwcTree*>(m_activeGlyph);
      if (tree) {
        Swc_Tree_Node *tn = tree->firstRegularRoot();
        if (tn) {
          if (SwcTreeNode::hasChild(tn)) {
            tn = SwcTreeNode::firstChild(tn);
          }
          pt.set(SwcTreeNode::center(tn), SwcTreeNode::radius(tn));
        }
      }
    }
      break;
    default:
      break;
    }
  }

  return pt;
}

void ZMouseCursorGlyph::useActiveGlyph(
    std::function<void (const ZPoint &, double)> proc)
{
  ZWeightedPoint pt = getActiveGlyphGeometry();
  proc(pt, pt.weight());
}

void ZMouseCursorGlyph::updateActiveGlyph(
    std::function<void (ZStackObject *)> f)
{
  if (m_activeGlyph) {
    f(m_activeGlyph);
  }
}

ZStroke2d* ZMouseCursorGlyph::makeStrokeFromActiveGlyph() const
{
  ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getActiveGlyph());
  if (stroke) {
    return stroke->clone();
  }

  return nullptr;
}

void ZMouseCursorGlyph::setSliceAxis(neutu::EAxis axis)
{
  foreach (ZStackObject *obj, m_glyphMap) {
    if (obj->getType() == ZStackObject::EType::STROKE) {
      obj->setSliceAxis(axis);
    }
  }
}

ZStackObject* ZMouseCursorGlyph::_getGlyph(ERole role) const
{
  return getGlyph(role);
}

ZStackObject* ZMouseCursorGlyph::_getActiveGlyph() const
{
  return getActiveGlyph();
}
