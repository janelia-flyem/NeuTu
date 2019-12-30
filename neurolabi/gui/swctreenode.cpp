#include "swctreenode.h"

#include <sstream>
#include <vector>
#include <set>
#include <list>
#include <stack>
#include <cassert>

#include "common/math.h"
#include "zswctree.h"
#include "zgraph.h"
#include "zstring.h"
#include "tz_geo3d_utils.h"
#include "c_stack.h"
#include "tz_stack_threshold.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_stat.h"
#include "tz_stack.h"
#include "neutubeconfig.h"
#include "zerror.h"
#include "zweightedpointarray.h"
#include "tz_stack_objlabel.h"
#include "imgproc/zstackprocessor.h"
#include "zstack.hxx"
#include "tz_3dgeom.h"

using namespace std;

Swc_Tree_Node* SwcTreeNode::MakePointer()
{
  return MakePointer(0.0, 0.0, 0.0, 1.0);
}

Swc_Tree_Node* SwcTreeNode::MakePointer(int id, int type, const ZPoint &pos,
                                        double radius, int parentId)
{
  return SwcTreeNode::MakePointer(id, type, pos.x(), pos.y(), pos.z(), radius,
                                  parentId);
}

Swc_Tree_Node* SwcTreeNode::MakePointer(int id, int type, double x, double y,
                                        double z, double radius, int parentId)
{
  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  Default_Swc_Node(&(tn->node));
  tn->node.id = id;
  tn->node.type = type;
  tn->node.x = x;
  tn->node.y = y;
  tn->node.z = z;
  tn->node.d = radius;
  tn->node.parent_id = parentId;

  return tn;
}

Swc_Tree_Node* SwcTreeNode::MakePointer(
    double x, double y, double z, double radius, Swc_Tree_Node *parent)
{
  Swc_Tree_Node *tn = SwcTreeNode::MakePointer(1, 0, x, y, z, radius, -1);

  if (parent != NULL) {
    setFirstChild(parent, tn);
    //setParent(tn, parent);
  }

  return tn;
}

Swc_Tree_Node* SwcTreeNode::MakePointer(const ZPoint &pos, double radius)
{
  return SwcTreeNode::MakePointer(1, 0, pos, radius, -1);
}

Swc_Tree_Node* SwcTreeNode::MakePointer(
    const ZPoint &pos, double radius, Swc_Tree_Node *parent)
{
  return MakePointer(pos.x(), pos.y(), pos.z(), radius, parent);
}

Swc_Tree_Node* SwcTreeNode::makeVirtualNode()
{
  return Make_Virtual_Swc_Tree_Node();
}

void SwcTreeNode::setDefault(Swc_Tree_Node *tn)
{
  Default_Swc_Tree_Node(tn);
}

double SwcTreeNode::x(const Swc_Tree_Node *tn)
{
  return tn->node.x;
}

double SwcTreeNode::y(const Swc_Tree_Node *tn)
{
  return tn->node.y;
}

double SwcTreeNode::z(const Swc_Tree_Node *tn)
{
  return tn->node.z;
}

void SwcTreeNode::setNode(Swc_Tree_Node *tn, int id, int type, double x,
                          double y, double z, double radius, int parentId)
{
  setId(tn, id);
  setType(tn, type);
  setRadius(tn, radius);
  setParentId(tn, parentId);
  setPos(tn, x, y, z);
}

void SwcTreeNode::setPos(Swc_Tree_Node *tn, double x, double y, double z)
{
  if (tn != NULL) {
    tn->node.x = x;
    tn->node.y = y;
    tn->node.z = z;
  }
}

void SwcTreeNode::setPos(Swc_Tree_Node *tn, const ZPoint &pt)
{
  if (tn != NULL) {
    tn->node.x = pt.x();
    tn->node.y = pt.y();
    tn->node.z = pt.z();
  }
}

double SwcTreeNode::radius(const Swc_Tree_Node *tn)
{
  return tn->node.d;
}

void SwcTreeNode::setRadius(Swc_Tree_Node *tn, double r)
{
  tn->node.d = dmax2(0.0, r);
}

void SwcTreeNode::changeRadius(Swc_Tree_Node *tn, double dr, double scale)
{
  double r = dmax2(SwcTreeNode::radius(tn) * scale + dr,
                   SwcTreeNode::MinimalRadius);
  SwcTreeNode::setRadius(tn, r);
}

int SwcTreeNode::id(const Swc_Tree_Node *tn)
{
  return tn->node.id;
}

void SwcTreeNode::setId(Swc_Tree_Node *tn, int id)
{
  tn->node.id = id;
}

void SwcTreeNode::setParentId(Swc_Tree_Node *tn, int id)
{
  tn->node.parent_id = id;
}

int SwcTreeNode::parentId(const Swc_Tree_Node *tn)
{
  return tn->node.parent_id;
}

bool SwcTreeNode::hasChild(const Swc_Tree_Node *tn)
{
  return tn->first_child != NULL;
}

int SwcTreeNode::childNumber(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Child_Number(tn);
}

bool SwcTreeNode::isParentIdConsistent(const Swc_Tree_Node *tn)
{
  assert(tn != NULL);

  if (tn->parent == NULL) {
    return (SwcTreeNode::parentId(tn) == -1);
  }

  return (tn->parent->node.id == tn->node.parent_id);
}

int SwcTreeNode::downstreamSize(Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Fsize(tn);
}

double SwcTreeNode::downstreamLength(Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return 0;
  }

  double length = 0;

  Swc_Tree_Node *pointer = tn;
  std::stack<Swc_Tree_Node*> nodeStack;

  do {
    Swc_Tree_Node *child = pointer->first_child;

    while (child != NULL) {
      nodeStack.push(child);
      length += SwcTreeNode::length(child);
      child = child->next_sibling;
    }
    if (!nodeStack.empty()) {
      pointer = nodeStack.top();
      nodeStack.pop();
    } else {
      pointer = NULL;
    }
  } while (pointer != NULL);

  return length;
}

double SwcTreeNode::downstreamLength(
    Swc_Tree_Node *tn, double sx, double sy, double sz)
{
  if (tn == NULL) {
    return 0;
  }

  double length = 0;

  Swc_Tree_Node *pointer = tn;
  std::stack<Swc_Tree_Node*> nodeStack;

  do {
    Swc_Tree_Node *child = pointer->first_child;

    while (child != NULL) {
      nodeStack.push(child);
      length += SwcTreeNode::length(child, sx, sy, sz);
      child = child->next_sibling;
    }
    if (!nodeStack.empty()) {
      pointer = nodeStack.top();
      nodeStack.pop();
    } else {
      pointer = NULL;
    }
  } while (pointer != NULL);

  return length;
}

int SwcTreeNode::downstreamSize(Swc_Tree_Node *tn,
                                Swc_Tree_Node_Compare compfunc)
{
  std::set<Swc_Tree_Node*> blocker;
  Swc_Tree_Node *child = tn->first_child;
  while (child != NULL) {
    if (compfunc(child, tn) <= 0) {
      blocker.insert(child);
    }
    child = child->next_sibling;
  }

  ZSwcTree tree;
  tree.setDataFromNodeRoot(tn);
  int count =
      tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, tn, blocker, _FALSE_);
  tree.setData(NULL, ZSwcTree::FREE_WRAPPER);

  return count;
}

int SwcTreeNode::singleTreeSize(Swc_Tree_Node *tn)
{
  return SwcTreeNode::downstreamSize(
        SwcTreeNode::regularRoot(tn));
}

bool SwcTreeNode::isRegular(const Swc_Tree_Node *tn)
{
  return (Swc_Tree_Node_Is_Regular(tn) == _TRUE_);
}

bool SwcTreeNode::isVirtual(const Swc_Tree_Node *tn)
{
  return (Swc_Tree_Node_Is_Virtual(tn) == _TRUE_);
}

Swc_Tree_Node* SwcTreeNode::regularRoot(Swc_Tree_Node *tn)
{
  Swc_Tree_Node *root = NULL;

  while (SwcTreeNode::isRegular(tn)) {
    root = tn;
    tn = tn->parent;
  }

  return root;
}

Swc_Tree_Node* SwcTreeNode::root(const Swc_Tree_Node *tn)
{
  const Swc_Tree_Node *root = tn;

  while (tn != NULL) {
    root = tn;
    tn = tn->parent;
  }

  return const_cast<Swc_Tree_Node*>(root);
}

ZPoint SwcTreeNode::center(const Swc_Tree_Node *tn)
{
  return ZPoint(tn->node.x, tn->node.y, tn->node.z);
}

void SwcTreeNode::setCenter(Swc_Tree_Node *tn, double x, double y, double z)
{
  if (tn != NULL) {
    tn->node.x = x;
    tn->node.y = y;
    tn->node.z = z;
  }
}

void SwcTreeNode::setCenter(Swc_Tree_Node *tn, const ZPoint &center)
{
  setCenter(tn, center.x(), center.y(), center.z());
}

std::string SwcTreeNode::toSwcLine(const Swc_Tree_Node *tn)
{
  std::ostringstream stream;
  if (isRegular(tn)) {
    stream << SwcTreeNode::id(tn) << ' ' << SwcTreeNode::type(tn) << ' '
           << SwcTreeNode::x(tn) << ' ' << SwcTreeNode::y(tn) << ' '
           << SwcTreeNode::z(tn) << ' ' << SwcTreeNode::radius(tn) << ' '
           << SwcTreeNode::parentId(tn) << std::endl;
  }

  return stream.str();
}

std::string SwcTreeNode::toString(const Swc_Tree_Node *tn)
{
  std::ostringstream stream;

  if (isVirtual(tn)) {
    stream << "swc node: Virtual" << endl;
  } else {
    stream << "swc node: " << SwcTreeNode::id(tn) << "(" << SwcTreeNode::type(tn)
           << ")" << "-->"
           << SwcTreeNode::parentId(tn)
           << "; " << "(" << SwcTreeNode::x(tn) << ", " << SwcTreeNode::y(tn)
           << ", " << SwcTreeNode::z(tn) << "); r = " << SwcTreeNode::radius(tn);
    stream << "; E: " << SwcTreeNode::label(tn) << " ";
  }

  return stream.str();
}

int SwcTreeNode::label(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Label(tn);
}

void SwcTreeNode::setLabel(Swc_Tree_Node *tn, int label)
{
  Swc_Tree_Node_Set_Label(tn, label);
}

void SwcTreeNode::addLabel(Swc_Tree_Node *tn, int label)
{
  tn->node.label += label;
}

void SwcTreeNode::copyProperty(const Swc_Tree_Node *src, Swc_Tree_Node *dst)
{
  Swc_Tree_Node_Copy_Property(src, dst);
}

int SwcTreeNode::index(const Swc_Tree_Node *tn)
{
  return tn->index;
}

void SwcTreeNode::print(const Swc_Tree_Node *tn)
{
  Print_Swc_Tree_Node(tn);
}

double SwcTreeNode::length(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Length(tn);
}

double SwcTreeNode::length(
    const Swc_Tree_Node *tn, double sx, double sy, double sz)
{
  return Swc_Tree_Node_Scaled_Length(tn, sx, sy, sz);
}

bool SwcTreeNode::isLeaf(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Is_Leaf(tn);
}

bool SwcTreeNode::isBranchPoint(const Swc_Tree_Node *tn)
{
  if (isRegularRoot(tn)) {
    if (childNumber(tn) > 2) {
      return true;
    }
  } else {
    return Swc_Tree_Node_Is_Branch_Point(tn);
  }

  return false;
}

bool SwcTreeNode::isTerminal(const Swc_Tree_Node *tn)
{
  if (isRegular(tn)) {
    int childNumber = SwcTreeNode::childNumber(tn);
    if (childNumber == 0) {
      return true;
    } else if (isRoot(tn) && childNumber == 1) {
      return true;
    }
  }

  return false;
}

int SwcTreeNode::minChildLabel(const Swc_Tree_Node *tn)
{
  int minLabel = 0;

  Swc_Tree_Node *child = tn->first_child;
  if (child != NULL) {
    minLabel = SwcTreeNode::label(tn);
  }

  while (child != NULL) {
    if (minLabel < SwcTreeNode::label(tn)) {
      minLabel = SwcTreeNode::label(tn);
    }
    child = child->next_sibling;
  }

  return minLabel;
}

void SwcTreeNode::setDownstreamType(Swc_Tree_Node *tn, int type)
{
  Swc_Tree_Node_Build_Downstream_List(tn);

  while (tn != NULL) {
    SwcTreeNode::setType(tn, type);
    tn = tn->next;
  }
}

bool SwcTreeNode::isAncestor(const Swc_Tree_Node *ancestor,
                             const Swc_Tree_Node *tn)
{
  while (tn != NULL) {
    if (ancestor == tn) {
      return true;
    }
    tn = tn->parent;
  }

  return false;
}

bool SwcTreeNode::isConnected(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2)
{
  if (tn1 == NULL || tn2 == NULL) {
    return false;
  }

  return (parent(tn1) == tn2 || parent(tn2) == tn1);
}


Swc_Tree_Node* SwcTreeNode::commonAncestor(Swc_Tree_Node *tn1,
                                           Swc_Tree_Node *tn2)
{
  return const_cast<Swc_Tree_Node*>(
        commonAncestor(static_cast<const Swc_Tree_Node*>(tn1),
                       static_cast<const Swc_Tree_Node*>(tn2)));
}

const Swc_Tree_Node* SwcTreeNode::commonAncestor(const Swc_Tree_Node *tn1,
                                                 const Swc_Tree_Node *tn2)
{
  return Swc_Tree_Node_Common_Ancestor(tn1, tn2);
}

void SwcTreeNode::setUpstreamType(
    Swc_Tree_Node *tn, int type, Swc_Tree_Node *stop)
{
  while (tn != NULL) {
    SwcTreeNode::setType(tn, type);
    if (tn == stop) {
      break;
    }

    tn = tn->parent;
  }
}

void SwcTreeNode::translate(Swc_Tree_Node *tn, double dx, double dy, double dz)
{
  if (tn != NULL) {
    tn->node.x += dx;
    tn->node.y += dy;
    tn->node.z += dz;
  }
}

void SwcTreeNode::translate(Swc_Tree_Node *tn, const ZPoint &pt)
{
  translate(tn, pt.x(), pt.y(), pt.z());
}

void SwcTreeNode::translate(Swc_Tree_Node *tn, const ZIntPoint &pt)
{
  translate(tn, pt.getX(), pt.getY(), pt.getZ());
}

void SwcTreeNode::rotate(Swc_Tree_Node *tn, double theta, double psi,
                         const ZPoint &center, bool inverse)
{
  if (tn != NULL) {
    translate(tn, -center.x(), -center.y(), -center.z());
    rotate(tn, theta, psi, inverse);
    translate(tn, center.x(), center.y(), center.z());
  }
}

void SwcTreeNode::rotate(
    Swc_Tree_Node *tn, double theta, double psi, bool inverse)
{
  if (tn != NULL) {
    Geo3d_Rotate_Coordinate(&(tn->node.x), &(tn->node.y), &(tn->node.z),
                            theta, psi, inverse);
  }
}

void SwcTreeNode::rotateAroundZ(
    Swc_Tree_Node *tn, double theta, double cx, double cy)
{
  if (tn != NULL) {
    translate(tn, -cx, -cy, 0);
    double pos[3];
    pos[0] = x(tn);
    pos[1] = y(tn);
    pos[2] = z(tn);
    Rotate_Z(pos, pos, 1, theta, false);
    SwcTreeNode::setPos(tn, pos[0], pos[1], pos[2]);
    translate(tn, cx, cy, 0);
  }
}

double SwcTreeNode::pathLength(const Swc_Tree_Node *tn1,
                               const Swc_Tree_Node *tn2)
{
  if (tn1 == tn2) {
    return 0.0;
  }

  double dist = Infinity;

  const Swc_Tree_Node *ancestor = SwcTreeNode::commonAncestor(tn1, tn2);
  if (SwcTreeNode::isRegular(ancestor)) {
    dist = 0.0;
    const Swc_Tree_Node *tn = tn1;
    while (tn != ancestor) {
      dist += Swc_Tree_Node_Length(tn);
      tn = tn->parent;
    }

    tn = tn2;
    while (tn != ancestor) {
      dist += Swc_Tree_Node_Length(tn);
      tn = tn->parent;
    }
  }

  return dist;
}

double SwcTreeNode::planePathLength(const Swc_Tree_Node *tn1,
                               const Swc_Tree_Node *tn2)
{
  double dist = Infinity;

  const Swc_Tree_Node *ancestor = SwcTreeNode::commonAncestor(tn1, tn2);
  if (SwcTreeNode::isRegular(ancestor)) {
    dist = 0.0;
    const Swc_Tree_Node *tn = tn1;
    while (tn != ancestor) {
      double dx = x(tn) - x(parent(tn));
      double dy = y(tn) - y(parent(tn));
      dist += sqrt(dx * dx + dy * dy);
      tn = tn->parent;
    }

    tn = tn2;
    while (tn != ancestor) {
      double dx = x(tn) - x(parent(tn));
      double dy = y(tn) - y(parent(tn));
      dist += sqrt(dx * dx + dy * dy);
      tn = tn->parent;
    }
  }

  return dist;
}

double SwcTreeNode::distance(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
                             SwcTreeNode::EDistanceType distType)
{
  if (!isRegular(tn1) || !isRegular(tn2)) {
    return 0;
  }

  double dist = 0.0;
  switch (distType) {
  case SwcTreeNode::EUCLIDEAN:
    dist = Swc_Tree_Node_Dist(tn1, tn2);
    break;
  case SwcTreeNode::EUCLIDEAN_SQUARE:
  {
    double dx = x(tn1) - x(tn2);
    double dy = y(tn1) - y(tn2);
    double dz = z(tn1) - z(tn2);
    dist = dx * dx + dy * dy + dz * dz;
  }
    break;
  case SwcTreeNode::PLANE_EUCLIDEAN:
  {
    double dx = x(tn1) - x(tn2);
    double dy = y(tn1) - y(tn2);
    dist = sqrt(dx * dx + dy * dy);
  }
    break;
  case SwcTreeNode::GEODESIC:
    dist = SwcTreeNode::pathLength(tn1, tn2);
    break;
  case SwcTreeNode::EUCLIDEAN_SURFACE:
    dist = Swc_Tree_Node_Dist(tn1, tn2) - radius(tn1) - radius(tn2);
    /*
    if (dist < 0) {
      dist = 0;
    }
    */
    break;
  }

  return dist;
}

double SwcTreeNode::distance(const Swc_Tree_Node *tn, double x, double y,
                             double z, EDistanceType distType)
{
  if (!isRegular(tn)) {
    return 0.0;
  }

  double dist = 0.0;
  switch (distType) {
  case SwcTreeNode::EUCLIDEAN:
    dist = Geo3d_Dist(SwcTreeNode::x(tn), SwcTreeNode::y(tn), SwcTreeNode::z(tn),
                      x, y, z);
    break;
  case SwcTreeNode::EUCLIDEAN_SURFACE:
    dist = distance(tn, x, y, z, SwcTreeNode::EUCLIDEAN) - radius(tn);
    break;
  default:
    throw std::invalid_argument("invalid distance type");
  }

  return dist;
}

double SwcTreeNode::scaledDistance(
    const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
    double sx, double sy, double sz)
{
  double dx = (x(tn1) - x(tn2)) * sx;
  double dy = (y(tn1) - y(tn2)) * sy;
  double dz = (z(tn1) - z(tn2)) * sz;

  return sqrt(dx * dx + dy * dy + dz * dz);
}

double SwcTreeNode::scaledSurfaceDistance(
    const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
    double sx, double sy, double sz)
{
  return scaledDistance(tn1, tn2, sx, sy, sz) - radius(tn1) - radius(tn2);
}

Swc_Tree_Node*
SwcTreeNode::furthestNode(Swc_Tree_Node *tn, EDistanceType distType)
{
  std::vector<Swc_Tree_Node*> leafArray;

  Swc_Tree_Node *root = SwcTreeNode::regularRoot(tn);
  leafArray.push_back(root);
  Swc_Tree_Node_Build_Downstream_List(root);
  for (Swc_Tree_Node *leaf = root; leaf != NULL; leaf = leaf->next) {
    if (SwcTreeNode::isLeaf(leaf)) {
      leafArray.push_back(leaf);
    }
  }

  double maxDist = 0.0;
  Swc_Tree_Node *target = NULL;

  double dist = 0.0;
  for (size_t i = 0; i < leafArray.size(); i++) {
    dist = SwcTreeNode::distance(tn, leafArray[i], distType);
    if (dist >= maxDist) {
      maxDist = dist;
      target = leafArray[i];
    }
  }

  return target;
}

void SwcTreeNode::setPathType(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2, int type)
{
  Swc_Tree_Node *ancestor = SwcTreeNode::commonAncestor(tn1, tn2);
  if (SwcTreeNode::isRegular(ancestor)) {
    Swc_Tree_Node *tn = tn1;
    while (tn != ancestor) {
      SwcTreeNode::setType(tn, type);
      tn = tn->parent;
    }

    tn = tn2;
    while (tn != ancestor) {
      SwcTreeNode::setType(tn, type);
      tn = tn->parent;
    }

    SwcTreeNode::setType(ancestor, type);
  }
}

void SwcTreeNode::addPathType(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2, int type)
{
  Swc_Tree_Node *ancestor = SwcTreeNode::commonAncestor(tn1, tn2);
  if (SwcTreeNode::isRegular(ancestor)) {
    Swc_Tree_Node *tn = tn1;
    while (tn != ancestor) {
      SwcTreeNode::addType(tn, type);
      tn = tn->parent;
    }

    tn = tn2;
    while (tn != ancestor) {
      SwcTreeNode::addType(tn, type);
      tn = tn->parent;
    }

    SwcTreeNode::addType(ancestor, type);
  }
}

void SwcTreeNode::addPathLabel(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2, int type)
{
  Swc_Tree_Node *ancestor = SwcTreeNode::commonAncestor(tn1, tn2);
  if (SwcTreeNode::isRegular(ancestor)) {
    Swc_Tree_Node *tn = tn1;
    while (tn != ancestor) {
      SwcTreeNode::addLabel(tn, type);
      tn = tn->parent;
    }

    tn = tn2;
    while (tn != ancestor) {
      SwcTreeNode::addLabel(tn, type);
      tn = tn->parent;
    }

    SwcTreeNode::addLabel(ancestor, type);
  }
}

void SwcTreeNode::setAsRoot(Swc_Tree_Node *tn)
{
  Swc_Tree_Node_Set_Root(tn);
}

int SwcTreeNode::labelDifference(Swc_Tree_Node *lhs, Swc_Tree_Node *rhs)
{
  return SwcTreeNode::label(lhs) - SwcTreeNode::label(rhs);
}

void SwcTreeNode::setParent(Swc_Tree_Node *tn, Swc_Tree_Node *parent,
                            EChildPosition childPos)
{
  switch (childPos) {
  case CHILD_POS_FIRST:
    setFirstChild(parent, tn);
    break;
  case CHILD_POS_LAST:
    Swc_Tree_Node_Set_Parent(tn, parent);
    break;
  }
}

void SwcTreeNode::setFirstChild(Swc_Tree_Node *tn, Swc_Tree_Node *child)
{
  if (child == NULL) {
    return;
  }

  Swc_Tree_Node *oldFirstChild = firstChild(tn);
  if (oldFirstChild == child) {
    return;
  }

  detachParent(child);
  if (tn != NULL) {
    SwcTreeNode::setLink(child, tn, SwcTreeNode::PARENT);
    SwcTreeNode::setLink(tn, child, SwcTreeNode::FIRST_CHILD);
    SwcTreeNode::setLink(child, oldFirstChild, SwcTreeNode::NEXT_SIBLING);
//    child->tree_state = tn->tree_state;
  }
}

bool SwcTreeNode::isRegularRoot(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Is_Regular_Root(tn);
}

bool SwcTreeNode::isRoot(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Is_Root(tn);
}

ZPoint SwcTreeNode::upStreamDirection(Swc_Tree_Node *tn, int n)
{
  ZPoint direction(0.0, 0.0, 0.0);
  Swc_Tree_Node *ancester = parent(tn);
  int upLevel = 1;
  while (ancester != NULL) {
    if (isBranchPoint(ancester) || isRegularRoot(ancester) ||
        (n > 0 && upLevel >= n)) {
      direction = center(tn) - center(ancester);
      direction.normalize();
      break;
    }
    ancester = parent(ancester);
    upLevel ++;
  }

  return direction;
}

ZPoint SwcTreeNode::weightedDirection(const ZWeightedPointArray &ptArray)
{
  ZPoint direction;

  double totalLength = 0.0;
  for (size_t i = 0; i < ptArray.size() - 1; ++i) {
    const ZWeightedPoint &pt = ptArray[i];
    ZPoint v = pt - ptArray[i + 1];
    v *= (pt.weight() + ptArray[i + 1].weight());
    totalLength += v.length();
    direction += v;
  }

  if (totalLength > 0) {
    double ratio = direction.length() / totalLength;
    direction.normalize();
    direction *= ratio;
  }

  return direction;
}

ZPoint SwcTreeNode::weightedDirection(const Swc_Tree_Node *tn, int extend)
{
  ZWeightedPointArray ptArray;

  const Swc_Tree_Node *start = tn;

  for (int i = 0; i < extend; ++i) {
    ptArray.append(SwcTreeNode::center(start), SwcTreeNode::radius(start));
    if (SwcTreeNode::isRegular(start->parent)) {
      start = start->parent;
    } else {
      break;
    }
  }

  ZPoint direction = weightedDirection(ptArray);

  return direction;
}

ZPoint SwcTreeNode::localDirection(const Swc_Tree_Node *tn, int extend)
{
  const Swc_Tree_Node *start = tn;
  const Swc_Tree_Node *end = tn;

  if (SwcTreeNode::isRegular(tn->parent)) {
    start = tn->parent;
  }

  for (int i = 1; i < extend; ++i) {
    if (SwcTreeNode::isRegular(start->parent)) {
      start = start->parent;
    } else {
      break;
    }
  }

  for (int i = 0; i < extend; ++i) {
    if (SwcTreeNode::isContinuation(end)) {
      end = end->first_child;
    } else {
      break;
    }
  }

  return center(end) - center(start);
}

void SwcTreeNode::killSubtree(Swc_Tree_Node *tn)
{
#ifdef _DEBUG_2
  std::cout << "Kill subtree of " << tn << std::endl;
  Print_Swc_Tree_Node(tn);
#endif

  Swc_Tree_Node_Kill_Subtree(tn);
}

void SwcTreeNode::setLink(Swc_Tree_Node *tn, Swc_Tree_Node *target,
                          EKnowingLink link)
{
  if (tn != NULL) {
    switch (link) {
    case FIRST_CHILD:
      tn->first_child = target;
      break;
    case PARENT:
      tn->parent = target;
      break;
    case NEXT_SIBLING:
      tn->next_sibling = target;
      break;
    }
  }
}

Swc_Tree_Node *SwcTreeNode::prevSibling(Swc_Tree_Node *tn)
{
  if (parent(tn) == NULL) {
    return NULL;
  }

  Swc_Tree_Node *sibling = SwcTreeNode::firstChild(SwcTreeNode::parent(tn));
  if (sibling == tn) {
    return NULL;
  }

  while (sibling != NULL) {
    if (SwcTreeNode::nextSibling(sibling) == tn) {
      break;
    }
    sibling = SwcTreeNode::nextSibling(sibling);
  }

  assert(sibling != NULL);

  return sibling;
}

Swc_Tree_Node* SwcTreeNode::lastChild(Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return NULL;
  }

  return Swc_Tree_Node_Last_Child(tn);
}

bool SwcTreeNode::isContinuation(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Is_Continuation(tn);
}

double SwcTreeNode::localRadius(const Swc_Tree_Node *tn, int extend)
{
  const Swc_Tree_Node *start = tn;
  const Swc_Tree_Node *end = tn;

  if (SwcTreeNode::isRegular(tn->parent)) {
    start = tn->parent;
  }

  for (int i = 1; i < extend; ++i) {
    if (SwcTreeNode::isRegular(start->parent)) {
      start = start->parent;
    } else {
      break;
    }
  }

  for (int i = 0; i < extend; ++i) {
    if (SwcTreeNode::isContinuation(end)) {
      end = end->first_child;
    } else {
      break;
    }
  }

  double r = SwcTreeNode::radius(end);
  int count = 1;

  while (start != end) {
    end = end->parent;
    r += SwcTreeNode::radius(end);
    count++;
  }

  return r / count;
}

ZWeightedPointArray SwcTreeNode::localSegment(const Swc_Tree_Node *tn, int extend)
{
  const Swc_Tree_Node *start = tn;
  const Swc_Tree_Node *end = tn;

  if (SwcTreeNode::isContinuation(tn->parent)) {
    start = tn->parent;
  }

  for (int i = 1; i < extend; ++i) {
    if (SwcTreeNode::isContinuation(start->parent)) {
      start = start->parent;
    } else {
      break;
    }
  }

  for (int i = 0; i < extend; ++i) {
    if (SwcTreeNode::isContinuation(end)) {
      end = end->first_child;
    } else {
      break;
    }
  }

  ZWeightedPointArray segment;
  segment.push_back(ZWeightedPoint(x(end), y(end), z(end), radius(end)));

  while (start != end) {
    end = end->parent;
    segment.push_back(ZWeightedPoint(x(end), y(end), z(end), radius(end)));
  }

  return segment;
}

Swc_Tree_Node* SwcTreeNode::continuousAncestor(
    const Swc_Tree_Node *tn, double minDist)
{
  Swc_Tree_Node *target = NULL;
  Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
  double dist = 0.0;
  while (SwcTreeNode::isRegular(parent)) {
    dist += SwcTreeNode::length(tn);
    if (dist >= minDist) {
      target = parent;
      break;
    }
    tn = parent;
    if (!isContinuation(tn)) {
      break;
    }
    parent = SwcTreeNode::parent(tn);
  }

  return target;
}

Swc_Tree_Node *SwcTreeNode::continuousDescendent(
    const Swc_Tree_Node *tn, double minDist)
{
  Swc_Tree_Node *target = NULL;
  Swc_Tree_Node *child = SwcTreeNode::firstChild(tn);
  double dist = 0.0;
  while (SwcTreeNode::isRegular(child)) {
    dist += SwcTreeNode::length(child);
    if (dist >= minDist) {
      target = child;
      break;
    }
    if (!isContinuation(child)) {
      break;
    }
    child = SwcTreeNode::firstChild(child);
  }

  return target;
}

Swc_Tree_Node *SwcTreeNode::thickestChild(Swc_Tree_Node *tn)
{
  Swc_Tree_Node *result = NULL;

  if (tn != NULL) {
    Swc_Tree_Node *child = tn->first_child;
    result = child;
    while (child != NULL) {
      if (radius(result) < radius(child)) {
        result = child;
      }
      child = child->next_sibling;
    }
  }

  return result;
}

bool SwcTreeNode::connect(const vector<Swc_Tree_Node *> &nodeArray)
{
  ZGraph graph(ZGraph::UNDIRECTED_WITH_WEIGHT);
  for (size_t i = 0; i < nodeArray.size(); ++i) {
    for (size_t j = i + 1; j < nodeArray.size(); ++j) {
      if (i < j) {
        if (SwcTreeNode::regularRoot(nodeArray[i]) !=
            SwcTreeNode::regularRoot(nodeArray[j])) {
          double w = SwcTreeNode::distance(nodeArray[i], nodeArray[j]);
          graph.addEdge(i, j, w + 0.1);
        } else {
          graph.addEdge(i, j, 0);
        }
      }
    }
  }
  if (graph.size() > 0) {
    graph.toMst();
    for (size_t i = 0; i < graph.size(); ++i) {
      if (graph.edgeWeight(i) > 0.0) {
        int e1 = graph.edgeStart(i);
        int e2 = graph.edgeEnd(i);
        assert(e1 != e2);
        if (SwcTreeNode::regularRoot(nodeArray[e1]) !=
            SwcTreeNode::regularRoot(nodeArray[e2])) {
          SwcTreeNode::setAsRoot(nodeArray[e2]);
          SwcTreeNode::setParent(nodeArray[e2], nodeArray[e1]);
        }
      }
    }

    return true;
  }

  return false;
}

bool SwcTreeNode::connect(const set<Swc_Tree_Node *> &nodeSet)
{
  vector<Swc_Tree_Node*> nodeArray;
  for (set<Swc_Tree_Node *>::const_iterator iter = nodeSet.begin();
       iter != nodeSet.end(); ++iter) {
    nodeArray.push_back(*iter);
  }

  return connect(nodeArray);
}

void SwcTreeNode::addToClipboard(const Swc_Tree_Node *tn)
{
  std::ostringstream stream;

  stream <<  id(tn) << " " << type(tn) << " " << x(tn) << " " << y(tn) << " "
         << z(tn) << " " << radius(tn) << " " <<parentId(tn);

  cout << stream.str() << " copied" << endl;

  clipboard().push_back(stream.str());
}

void SwcTreeNode::paste(Swc_Tree_Node *tn, size_t index)
{
  if (clipboard().size() > index) {
    ZString str(clipboard()[clipboard().size() - index - 1]);
    vector<double> attribute = str.toDoubleArray();
    setPos(tn, attribute[2], attribute[3], attribute[4]);
    setRadius(tn, attribute[5]);
    setId(tn, neutu::iround(attribute[0]));
    setType(tn, neutu::iround(attribute[2]));
    setParentId(tn, neutu::iround(attribute[6]));
  }
}

void SwcTreeNode::clearClipboard()
{
  clipboard().clear();
}

void SwcTreeNode::detachParent(Swc_Tree_Node *tn)
{
  Swc_Tree_Node_Detach_Parent(tn);
}

ZPoint SwcTreeNode::centroid(const std::set<Swc_Tree_Node*> &nodeSet)
{
  ZPoint pt(0.0, 0.0, 0.0);
  double weight = 0.0;
  int count = 0;

  for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
       iter != nodeSet.end(); ++iter) {
    pt += center(*iter) * radius(*iter);
    weight += radius(*iter);
    ++count;
  }

  if (weight > 0.0) {
    pt /= weight;
  } else {
    pt /= count;
  }

  return pt;
}

double SwcTreeNode::maxRadius(const std::set<Swc_Tree_Node*> &nodeSet)
{
  double mr = 0.0;

  for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
       iter != nodeSet.end(); ++iter) {
    if (mr < radius(*iter)) {
      mr = radius(*iter);
    }
  }

  return mr;
}

ZCuboid SwcTreeNode::boundBox(const Swc_Tree_Node *tn)
{
  ZCuboid bound;
  bound.set(x(tn) - radius(tn), y(tn) - radius(tn), z(tn) - radius(tn),
            x(tn) + radius(tn), y(tn) + radius(tn), z(tn) + radius(tn));

  return bound;
}

ZCuboid SwcTreeNode::boundBox(const std::set<Swc_Tree_Node *> &nodeSet)
{
  ZCuboid bound = boundBox(*(nodeSet.begin()));
  for (std::set<Swc_Tree_Node *>::const_iterator iter = nodeSet.begin();
       iter != nodeSet.end(); ++iter) {
    bound.bind(boundBox(*iter));
  }

  return bound;
}

bool SwcTreeNode::isAllConnected(const std::set<Swc_Tree_Node *> &nodeSet)
{
  return isAllConnected(nodeSet.begin(), nodeSet.end());
}

Swc_Tree_Node* SwcTreeNode::merge(const set<Swc_Tree_Node*> &nodeSet)
{
  Swc_Tree_Node *coreNode = NULL;

  if (nodeSet.size() > 1) {
    ZPoint center = SwcTreeNode::centroid(nodeSet);
    double radius = SwcTreeNode::maxRadius(nodeSet);

    coreNode = MakePointer(center, radius);

    set<Swc_Tree_Node*> parentSet;
    //set<Swc_Tree_Node*> childSet;

    for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      assert(*iter != NULL);

      if (isRegular(parent(*iter))) {
        if (nodeSet.count(parent(*iter)) == 0) {
          parentSet.insert(parentSet.end(), parent(*iter));
        }
      }

      Swc_Tree_Node *child = firstChild(*iter);
      while (child != NULL) {
        Swc_Tree_Node *nextChild = SwcTreeNode::nextSibling(child);
        if (nodeSet.count(child) == 0) {
          SwcTreeNode::setParent(child, coreNode);
          //childSet.insert(childSet.end(), child);
        }
        child = nextChild;
      }
    }

    if (parentSet.empty()) { //try to attach to a virtual root
      for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        if (isVirtual(parent(*iter))) {
          SwcTreeNode::setParent(coreNode, parent(*iter));
          break;
        }
      }
    } else if (parentSet.size() > 1) {
      for (set<Swc_Tree_Node*>::iterator iter = parentSet.begin();
           iter != parentSet.end(); ++iter) {
        SwcTreeNode::setParent(*iter, coreNode);
      }
    } else {
      SwcTreeNode::setParent(coreNode, *parentSet.begin());
    }
  } else if (!nodeSet.empty()) {
    coreNode = *(nodeSet.begin());
  }

  return coreNode;
}

void SwcTreeNode::kill(set<Swc_Tree_Node *> &nodeSet)
{
  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
       iter != nodeSet.end(); ++iter) {
    detachParent(*iter);
    Swc_Tree_Node *child = (*iter)->first_child;
    while (child != NULL) {
      Swc_Tree_Node *nextChild = child->next_sibling;
      detachParent(child);
      child = nextChild;
    }

    Kill_Swc_Tree_Node(*iter);
  }

  nodeSet.clear();
}

void SwcTreeNode::kill(Swc_Tree_Node *tn)
{
#ifdef _DEBUG_2
  std::cout << "Deleting Swc node " << tn << std::endl;
//  Print_Swc_Tree_Node(tn);
#endif

  Kill_Swc_Tree_Node(tn);
}

double SwcTreeNode::segmentLength(std::set<Swc_Tree_Node *> &nodeSet)
{
  double length = 0.0;

  for (std::set<Swc_Tree_Node *>::const_iterator iter = nodeSet.begin();
       iter != nodeSet.end(); ++iter) {
    if (isRegular(*iter)) {
      if (nodeSet.count(parent(*iter)) > 0) {
        length += Swc_Tree_Node_Length(*iter);
      }
    }
  }

  return length;
}

double SwcTreeNode::scaledSegmentLength(std::set<Swc_Tree_Node*> &nodeSet,
                                        double sx, double sy, double sz)
{
  double length = 0.0;

  for (std::set<Swc_Tree_Node *>::const_iterator iter = nodeSet.begin();
       iter != nodeSet.end(); ++iter) {
    if (isRegular(*iter)) {
      if (nodeSet.count(parent(*iter)) > 0) {
        length += Swc_Tree_Node_Scaled_Length(*iter, sx, sy, sz);
      }
    }
  }

  return length;
}


int SwcTreeNode::compareZ(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2)
{
  if (z(tn1) < z(tn2)) {
    return -1;
  } else if (z(tn1) > z(tn2)) {
    return 1;
  }

  return 0;
}

#define COMPARE_VIRTUAL(tn1, tn2) \
  if (isVirtual(tn1) && isVirtual(tn2)) { \
    return false; \
  } else if (isVirtual(tn1)) { \
    return true; \
  } else if (isVirtual(tn2)) { \
    return false; \
  }

bool SwcTreeNode::lessThanZ(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2)
{
  COMPARE_VIRTUAL(tn1, tn2);

  return (z(tn1) < z(tn2));
}

bool SwcTreeNode::lessThanWeight(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2)
{
  return (weight(tn1) < weight(tn2));
}

bool SwcTreeNode::biggerThanWeight(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2)
{
  return (weight(tn1) > weight(tn2));
}

void SwcTreeNode::average(const Swc_Tree_Node *tn1,
                          const Swc_Tree_Node *tn2, Swc_Tree_Node *out)
{
  if (isRegular(tn1) && isRegular(tn2) && isRegular(out)) {
    setRadius(out, (radius(tn1) + radius(tn2)) * 0.5);
    setX(out, (x(tn1) + x(tn2)) * 0.5);
    setY(out, (y(tn1) + y(tn2)) * 0.5);
    setZ(out, (z(tn1) + z(tn2)) * 0.5);
  }
}

void SwcTreeNode::weightedAverage(const Swc_Tree_Node *tn1,
                          const Swc_Tree_Node *tn2, Swc_Tree_Node *out)
{
  double w1 = weight(tn1);
  double w2 = weight(tn2);

  if (w1 == 0.0 && w2 == 0.0) {
    if (isRegular(tn1) && isRegular(tn2) && isRegular(out)) {
      setRadius(out, (radius(tn1) + radius(tn2)) * 0.5);
      setX(out, (x(tn1) + x(tn2)) * 0.5);
      setY(out, (y(tn1) + y(tn2)) * 0.5);
      setZ(out, (z(tn1) + z(tn2)) * 0.5);
    }
  } else {
    double w = w1 + w2;
    if (isRegular(tn1) && isRegular(tn2) && isRegular(out)) {
      setRadius(out, (radius(tn1) * w1 + radius(tn2) * w2) / w);
      setX(out, (x(tn1) * w1 + x(tn2) * w2) / w);
      setY(out, (y(tn1) * w1 + y(tn2) * w2) / w);
      setZ(out, (z(tn1) * w1 + z(tn2) * w2) / w);
    }
  }
}

void SwcTreeNode::LabelStack(
    const Swc_Tree_Node *tn, ZStack *stack, Swc_Tree_Node_Label_Workspace *ws)
{
  Swc_Tree_Node tmpTn = *tn;
  SwcTreeNode::translate(&tmpTn, -stack->getOffset());
  Swc_Tree_Node_Label_Stack(&tmpTn, stack->c_stack(), ws);
}

void SwcTreeNode::interpolate(
    const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2, double lambda,
    Swc_Tree_Node *out)
{
  if (out != NULL) {
    if (isRegular(tn1) && isRegular(tn2)) {
      setRadius(out, radius(tn1) * lambda + radius(tn2) * (1 - lambda));
      setX(out, x(tn1) * lambda + x(tn2) * (1 - lambda));
      setY(out, y(tn1) * lambda + y(tn2) * (1 - lambda));
      setZ(out, z(tn1) * lambda + z(tn2) * (1 - lambda));
    }
  }
}

double SwcTreeNode::estimateRadius(const Swc_Tree_Node *tn, const Stack *stack,
                                   neutu::EImageBackground bg)
{

  //Extract image slice
  int x1 = neutu::iround(x(tn) - radius(tn) * 2);
  int y1 = neutu::iround(y(tn) - radius(tn) * 2);
  int x2 = neutu::iround(x(tn) + radius(tn) * 2);
  int y2 = neutu::iround(y(tn) + radius(tn) * 2);
  int cz = neutu::iround(z(tn));

  Stack *slice = C_Stack::crop(
        stack, x1, y1, cz, x2 - x1 + 1, y2 - y1 + 1, 1, NULL);

  //RC threshold
  int thre = Stack_Threshold_Triangle(slice, 0, 65535);

  //Seed grow
  Stack_Threshold_Binarize(slice, thre);

  if (bg == neutu::EImageBackground::BRIGHT) {
    Stack_Invert_Value(slice);
  }

  //2D distance map
  Stack *dist = Stack_Bwdist_L_U16P(slice, NULL, 0);

  //Find maxima
  double r2 = Stack_Max(dist, NULL);

  C_Stack::kill(slice);
  C_Stack::kill(dist);

  //return radius
  return sqrt(r2);
}

bool SwcTreeNode::fitSignal(Swc_Tree_Node *tn, const Stack *stack,
                            neutu::EImageBackground bg, int option)
{
  if (tn == NULL || stack == NULL) {
    return false;
  }

  bool succ = false;

  double expandScale = 2.0;
  double expandRadius = radius(tn) * expandScale + 3.0;
  //Extract image slice
  int x1 = neutu::iround(x(tn) - expandRadius);
  int y1 = neutu::iround(y(tn) - expandRadius);
  int x2 = neutu::iround(x(tn) + expandRadius);
  int y2 = neutu::iround(y(tn) + expandRadius);

  if (x1 < 0) {
    x1 = 0;
  }
  if (y1 < 0) {
    y1 = 0;
  }
  if (x2 >= C_Stack::width(stack)) {
    x2 = C_Stack::width(stack) - 1;
  }
  if (y2 >= C_Stack::height(stack)) {
    y2 = C_Stack::height(stack) - 1;
  }

  int cz = neutu::iround(z(tn));

  if (cz >= C_Stack::depth(stack)) {
    return false;
  }

  Stack *slice = C_Stack::crop(
        stack, x1, y1, cz, x2 - x1 + 1, y2 - y1 + 1, 1, NULL);


  if (slice == NULL) {
    return false;
  }

  /*
  Filter_3d *filter = Gaussian_Filter_3d(1.0, 1.0, 1.0);

  Stack *stack2 = Filter_Stack(slice, filter);
  C_Stack::kill(slice);
  slice = Crop_Stack(stack2, 3, 3, 0, C_Stack::width(stack2) - 6,
                     C_Stack::height(stack2) - 6, 1, NULL);
  Kill_FMatrix(filter);
  C_Stack::kill(stack2);


  x1 += 3;
  y1 += 3;
  */

  if (bg == neutu::EImageBackground::BRIGHT) {
    Stack_Invert_Value(slice);
  }

  Stack *denoised = Stack_Median_Filter_N(slice, 8, NULL);
  C_Stack::kill(slice);
  slice = denoised;

#ifdef _DEBUG_2
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", slice);
#endif

  //RC threshold

  int thre = 0;
  if (option == 1) {
    thre = Stack_Threshold_RC(slice, 0, 65535);
  } else {
    thre = Stack_Threshold_Triangle(slice, 0, 65535);
  }
//  int thre = Stack_Threshold_RC(slice, 0, thre2);


//  int thre = (thre1 + thre2) / 2;

  //Binarize
  Stack_Threshold_Binarize(slice, thre);
  C_Stack::translate(slice, GREY, 1);

  Stack *skel = Stack_Bwthin(slice, NULL);

  //2D distance map
  Stack *dist = Stack_Bwdist_L_U16P(slice, NULL, 0);
  uint16_t *distArray = (uint16_t*) C_Stack::array8(dist);
  size_t voxelNumber = C_Stack::voxelNumber(dist);
  for (size_t i = 0; i < voxelNumber; ++i) {
    if (skel->array[i] == 0) {
      distArray[i] = 0;
    }
  }

#ifdef _DEBUG_2
  C_Stack::write(GET_TEST_DATA_DIR + "/test2.tif", dist);
#endif

//  Stack *locmax = Stack_Locmax(dist, NULL);
#if 0
  Stack *locmax = Stack_Local_Max(dist, NULL, STACK_LOCMAX_CENTER);
  for (size_t i = 0; i < voxelNumber; ++i) {
    if (locmax->array[i] == 0) {
      skel->array[i] = 0;
    }
  }

  C_Stack::kill(locmax);
#endif

  ZStackProcessor::ShrinkSkeleton(skel, 3);

#ifdef _DEBUG_2
  C_Stack::write(GET_TEST_DATA_DIR + "/test3.tif", skel);
#endif

  size_t index = C_Stack::closestForegroundPixel(skel, x(tn) - x1, y(tn) - y1, 0);

  int nx, ny, nz;
  C_Stack::indexToCoord(index, C_Stack::width(skel), C_Stack::height(skel),
                        &nx, &ny, &nz);

  double r2 = C_Stack::value(dist, index);

  if (r2 > 0) {
    double squreDistShift = Geo3d_Dist_Sqr(nx, ny, 0, x(tn) - x1, y(tn) - y1, 0);
    if (squreDistShift <= (r2 + 3) * (r2 + 3)) {
      Stack_Label_Object_Dist_N(skel, NULL, index, 1, 2, 3, 8);

      SwcTreeNode::setRadius(tn, sqrt(r2));
      SwcTreeNode::setX(tn, nx + x1);
      SwcTreeNode::setY(tn, ny + y1);
      succ = true;
    }
  }

  C_Stack::kill(slice);
  C_Stack::kill(dist);
  C_Stack::kill(skel);

  return succ;
}

void SwcTreeNode::adoptChildren(
    Swc_Tree_Node *newParent, Swc_Tree_Node *oldParent)
{
  if (oldParent != NULL) {
    Swc_Tree_Node *child = SwcTreeNode::firstChild(oldParent);
    while (child != NULL) {
      SwcTreeNode::setParent(child, newParent);
      child = SwcTreeNode::nextSibling(child);
    }
  }
}

bool SwcTreeNode::isNearby(
    const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2, double distThre)
{
  if (!isRegular(tn1) || !isRegular(tn2)) {
    return false;
  }

  double dist = distance(tn1, tn2, SwcTreeNode::EUCLIDEAN_SURFACE);

  return (dist <= distThre);
}

bool SwcTreeNode::hasOverlap(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2)
{
  if (!isRegular(tn1) || !isRegular(tn2)) {
    return false;
  }

  double dist = distance(tn1, tn2);

  return dist < radius(tn1) + radius(tn2);
}

bool SwcTreeNode::hasSignificantOverlap(
    const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2)
{
  if (!isRegular(tn1) || !isRegular(tn2)) {
    return false;
  }

  double dist = distance(tn1, tn2);

  return (dist < radius(tn1)) || (dist < radius(tn2));
}

bool SwcTreeNode::isTurn(const Swc_Tree_Node *tn1,
                         const Swc_Tree_Node *tn2, const Swc_Tree_Node *tn3)
{
  if (!isRegular(tn1) || !isRegular(tn2) || !isRegular(tn3)) {
    return false;
  }

  return Swc_Tree_Node_Forming_Turn(tn1, tn2, tn3);
}

double SwcTreeNode::normalizedDot(
    const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2, const Swc_Tree_Node *tn3)
{
  if (!isRegular(tn1) || !isRegular(tn2) || !isRegular(tn3)) {
    return 0.0;
  }

  double vec1[3], vec2[3];
  vec1[0] = x(tn1) - x(tn2);
  vec1[1] = y(tn1) - y(tn2);
  vec1[2] = z(tn1) - z(tn2);

  vec2[0] = x(tn2) - x(tn3);
  vec2[1] = y(tn2) - y(tn3);
  vec2[2] = z(tn2) - z(tn3);

  Coordinate_3d_Unitize(vec1);
  Coordinate_3d_Unitize(vec2);

  double d =
    Geo3d_Dot_Product(vec1[0], vec1[1], vec1[2], vec2[0], vec2[1], vec2[2]);

  return d;
}

bool SwcTreeNode::isWithin(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2)
{
  if (!isRegular(tn1) || !isRegular(tn2)) {
    return false;
  }

  return (SwcTreeNode::radius(tn1) + SwcTreeNode::distance(tn1, tn2)) <=
      SwcTreeNode::radius(tn2);
}

void SwcTreeNode::mergeToParent(Swc_Tree_Node *tn, EMergeOption option)
{
  if (tn != NULL) {
    Swc_Tree_Node *parent = SwcTreeNode::parent(tn);

    switch (option) {
    case MERGE_W_CHILD:
      copyProperty(tn, parent);
      break;
    case MERGE_AVERAGE:
      average(tn, parent, parent);
      break;
    case MERGE_WEIGHTED_AVERAGE:
      weightedAverage(tn, parent, parent);
      parent->weight += 1.0;
      break;
    default:
      break;
    }

    Swc_Tree_Node_Merge_To_Parent(tn);
  }
}

Swc_Tree_Node* SwcTreeNode::findClosestNode(
    const std::set<Swc_Tree_Node *> &nodeSet, const Swc_Tree_Node *seeker)
{
  double minDist = Infinity;
  const Swc_Tree_Node *target = NULL;
  for (std::set<Swc_Tree_Node *>::const_iterator iter = nodeSet.begin();
       iter != nodeSet.end(); ++iter) {
    double dist = distance(*iter, seeker, SwcTreeNode::EUCLIDEAN_SURFACE);
    if (dist < minDist) {
      minDist = dist;
      target = *iter;
    }
  }

  return const_cast<Swc_Tree_Node*>(target);
}

void SwcTreeNode::dump(const Swc_Tree_Node *tn, ostream &stream)
{
  if (tn != NULL) {
    if (parent(tn) != NULL) {
      if (parentId(tn) != id(parent(tn))) {
        RECORD_WARNING_UNCOND("Inconsistent parent ID.")
      }
    }

    stream << id(tn) << " " << type(tn) << " " << x(tn) << " " << y(tn) << " "
           << z(tn) << " " << radius(tn) << " " << parentId(tn) << std::endl;
  }
}

void SwcTreeNode::interpolate(
    Swc_Tree_Node *tn1, Swc_Tree_Node *tn2, double lambda,
    double *x, double *y, double *z, double *r)
{
  if (isRegular(tn1) && isRegular(tn2)) {
    if (x != NULL) {
      *x = SwcTreeNode::x(tn1) * lambda + SwcTreeNode::x(tn2) * (1 - lambda);
    }
    if (y != NULL) {
      *y = SwcTreeNode::y(tn1) * lambda + SwcTreeNode::y(tn2) * (1 - lambda);
    }
    if (z != NULL) {
      *z = SwcTreeNode::z(tn1) * lambda + SwcTreeNode::z(tn2) * (1 - lambda);
    }
    if (r != NULL) {
      *r = SwcTreeNode::radius(tn1) * lambda + SwcTreeNode::radius(tn2) * (1 - lambda);
    }
  }
}

double SwcTreeNode::pathLengthRatio(
    Swc_Tree_Node *tn1, Swc_Tree_Node *tn2, Swc_Tree_Node *tn)
{
  double d1 = pathLength(tn1, tn);
  double d2 = pathLength(tn2, tn);

  if (d1 == 0.0 && d2 == 0.0) {
    return 0.5;
  } else if (d1 == 0.0) {
    return 0.0;
  }


  if (tz_isinf(d1) && tz_isinf(d2)) {
    return 0.5;
  } else if (tz_isinf(d1)) {
    return 1.0;
  } else if (tz_isinf(d2)) {
    return 0.0;
  }

  return d1 / (d1 + d2);
}

void SwcTreeNode::interpolate(
    Swc_Tree_Node *tn1, Swc_Tree_Node *tn2, Swc_Tree_Node *tn)
{
  if (!isRegular(tn1) || !isRegular(tn2) || !isRegular(tn)) {
    return;
  }

  double lambda = pathLengthRatio(tn2, tn1, tn);
  interpolate(tn1, tn2, lambda, tn);
}

std::vector<Swc_Tree_Node*> SwcTreeNode::neighborArray(const Swc_Tree_Node *tn)
{
  std::vector<Swc_Tree_Node*> neighborArray;
  if (isRegular(tn)) {
    if (isRegular(parent(tn))) {
      neighborArray.push_back(parent(tn));
    }
    Swc_Tree_Node *child = firstChild(tn);
    while (child != NULL) {
      neighborArray.push_back(child);
      child = nextSibling(child);
    }
  }

  return neighborArray;
}

int SwcTreeNode::regularNeighborNumber(const Swc_Tree_Node *tn)
{
  int count = 0;
  if (isRegular(tn)) {
    if (isRegular(parent(tn))) {
      ++count;
    }
    Swc_Tree_Node *child = firstChild(tn);
    while (child != NULL) {
      ++count;
      child = nextSibling(child);
    }
  }

  return count;
}

std::map<Swc_Tree_Node*, Swc_Tree_Node*> SwcTreeNode::crossoverMatch(
    const Swc_Tree_Node *center, double maxAngle)
{
  std::map<Swc_Tree_Node*, Swc_Tree_Node*> matching;
  if (isRegular(center)) {
    ZGraph graph(ZGraph::UNDIRECTED_WITH_WEIGHT);

    std::vector<Swc_Tree_Node*> nbrArray = SwcTreeNode::neighborArray(center);
    int goodNodeNumber = 0;

    double minDot = cos(maxAngle);

    double pos1[3] = {0, 0, 0};
    double centerPos[3] = {0, 0, 0};
    double pos2[3] = {0, 0, 0};

    Swc_Tree_Node_Pos(center, centerPos);

    if (nbrArray.size() >= 4) {

      for (size_t i = 0; i < nbrArray.size(); ++i) {
        Swc_Tree_Node_Pos(nbrArray[i], pos1);
        bool firstMatch = true;
        for (size_t j = i + 1; j < nbrArray.size(); ++j) {
          if (firstMatch) {
            goodNodeNumber++;
            firstMatch = false;
          }
          Swc_Tree_Node_Pos(nbrArray[j], pos2);
          double dot = Coordinate_3d_Cos3(pos1, centerPos, pos2);
          if (dot >= minDot) {
            bool isGood = true;
            //Calculate pairwise angles
            if (isGood) {
              goodNodeNumber++;
            }
            graph.addEdge(i, j, acos(dot));
          } else {
            #define LARGE_WEIGHT 10000.0
            graph.addEdge(i, j, LARGE_WEIGHT);
          }
        }
      }
    }

    if (goodNodeNumber >= 4) {
      std::map<int, int> matchIndex = graph.runMinWeightSumMatch();
      for (std::map<int, int>::const_iterator iter = matchIndex.begin();
           iter != matchIndex.end(); ++iter) {
        matching.insert(std::map<Swc_Tree_Node*, Swc_Tree_Node*>::value_type(
                          nbrArray[iter->first], nbrArray[iter->second]));
      }
    }
  }

  return matching;
}

double SwcTreeNode::computeCurvature(
    const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2, const Swc_Tree_Node *tn3)
{
  if (!isRegular(tn1) || !isRegular(tn2) || !isRegular(tn3)) {
    return 0.0;
  }

  double curvature = 0.0;

  double a = SwcTreeNode::distance(tn2, tn1);
  double b = SwcTreeNode::distance(tn2, tn3);
  double c = SwcTreeNode::distance(tn1, tn3);
  if (a > 0.0 && b > 0.0 && c > 0.0) {
    double u = b + c - a;
    double v = a + c - b;
    double w = a + b - c;
    if (u > 0.0 && v > 0.0 && w > 0.0) {
      curvature = sqrt(a + b + c) / a * sqrt(u) / b * sqrt(v) / c * sqrt(w);
    }
  }

  return curvature;
}

void SwcTreeNode::correctTurn(Swc_Tree_Node *tn)
{
  Swc_Tree_Node *tn1 = NULL;
  Swc_Tree_Node *tn2 = NULL;
  if (SwcTreeNode::isContinuation(tn)) {
    tn1 = SwcTreeNode::firstChild(tn);
    tn2 = SwcTreeNode::parent(tn);
  } else {
    std::vector<Swc_Tree_Node*> neighborArray =
        SwcTreeNode::neighborArray(tn);
    double minDot = 0.0;
    for (size_t i = 0; i < neighborArray.size(); ++i) {
      for (size_t j = 0; j < neighborArray.size(); ++j) {
        if (i != j) {
          double dot = Swc_Tree_Node_Dot(
                neighborArray[i], tn, neighborArray[j]);
          if (dot < minDot) {
            minDot = dot;
            tn1 = neighborArray[i];
            tn2 = neighborArray[j];
          }
        }
      }
    }
  }

  if (SwcTreeNode::isTurn(tn1, tn, tn2)) {
    double lambda = SwcTreeNode::pathLengthRatio(tn2, tn1, tn);
    double x, y, z, r;
    SwcTreeNode::interpolate(tn1, tn2, lambda, &x, &y, &z, &r);
    SwcTreeNode::setX(tn, x);
    SwcTreeNode::setY(tn, y);
    SwcTreeNode::setZ(tn, z);
    SwcTreeNode::setRadius(tn, r);
  }
}

double SwcTreeNode::maxBendingEnergy(const Swc_Tree_Node *tn)
{
  double e = 0.0;

  std::list<ZPoint> pointList;
  int count = 0;

  if (isRegular(tn) && !isBranchPoint(tn)) {
    pointList.insert(pointList.end(), center(tn));
    ++count;
    if (childNumber(tn) == 1) {
      pointList.insert(pointList.end(), center(firstChild(tn)));
      ++count;
      if (childNumber(firstChild(tn)) == 1) {
        pointList.insert(pointList.end(), center(firstChild(firstChild(tn))));
        ++count;
      }
    }
    if (!isRoot(tn)) {
      pointList.insert(pointList.begin(), center(parent(tn)));
      ++count;
      if (!isRoot(parent(tn))) {
        pointList.insert(pointList.begin(), center(parent(parent(tn))));
        ++count;
      }
    }
  }

  if (count > 2) {
    std::vector<ZPoint> pointArray;
    pointArray.insert(pointArray.begin(), pointList.begin(), pointList.end());
    ZPoint v1 = pointArray[1] - pointArray[0];
    ZPoint v2 = pointArray[2] - pointArray[1];
    e = v1.cosAngle(v2);

    for (size_t i = 3; i < pointArray.size(); ++i) {
      v1 = v2;
      v2 = pointArray[i] - pointArray[i - 1];
      double tmpE = v1.cosAngle(v2);
      if (tmpE < e) {
        e = tmpE;
      }
    }

    e = 1.0 - e;
  }

  return e;
}

double SwcTreeNode::averageIntensity(const Swc_Tree_Node *tn, const Stack *stack)
{
  double v = 0.0;
  if (tn && stack) {
    return Swc_Tree_Node_Intensity_Distribution_Ec(
          const_cast<Swc_Tree_Node*>(tn), const_cast<Stack*>(stack), NULL, 0.0);
  }

  return v;
}

