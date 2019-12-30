#include "zflyemstackdoc.h"

#include <set>

#include "common/math.h"
#include "flyem/zsegmentationanalyzer.h"
#include "zstackfile.h"
#include "tz_stack_neighborhood.h"
#include "zstring.h"
#include "swctreenode.h"
#include "zswctree.h"
#include "geometry/zgeometry.h"
#include "zgraph.h"

using namespace std;

ZFlyEmStackDoc::ZFlyEmStackDoc(QObject *parent) :
  ZStackDoc(parent)
{

}

ZFlyEmStackDoc::~ZFlyEmStackDoc()
{
}

/*
void ZFlyEmStackDoc::setSuperpixelMap(const ZSuperpixelMapArray &superpixelMap)
{
  m_superpixelMap = superpixelMap;
}
*/

void ZFlyEmStackDoc::appendBodyNeighbor(vector<vector<double> > *selected)
{
//  TZ_ASSERT(selected != NULL, "null poiner");

  ZGraph *graph = getBodyGraph();

  if (graph->size() > 0 && selected->size() > 0) {
    vector<int> vertexArray(selected->size());
    vector<int>::iterator vertexArrayIter = vertexArray.begin();
    for (vector<vector<double> >::const_iterator iter = selected->begin();
         iter != selected->end(); ++iter, ++vertexArrayIter) {
      *vertexArrayIter = flyem::ZSegmentationAnalyzer::channelCodeToId(*iter);
    }

    set<int> neighborSet = graph->getNeighborSet(vertexArray);
    size_t channelNumber = (*selected)[0].size();

    size_t index = selected->size();
    selected->resize(selected->size() + neighborSet.size());

    for (set<int>::const_iterator iter = neighborSet.begin();
         iter != neighborSet.end(); ++iter, ++index) {
      vector<uint8_t> channelCode =
          flyem::ZSegmentationAnalyzer::idToChannelCode(*iter, channelNumber);

      for (size_t i = 0; i < channelNumber; i++) {
        (*selected)[index][i] = channelCode[i];
      }
    }
  }
}

QString ZFlyEmStackDoc::rawDataInfo(double x, double y, int z, neutu::EAxis axis) const
{
  QString info = ZStackDoc::rawDataInfo(x, y, z, axis);

  ZStack *segmentation = getSegmentation();

  if (segmentation != NULL) {
//    TZ_ASSERT(segmentation->channelNumber() != 0, "Empty stack");

    info += " | Body ID: ";
    int wx = neutu::iround(x);
    int wy = neutu::iround(y);
    int wz = z;
    zgeom::shiftSliceAxisInverse(wx, wy, wz, axis);
//    ZGeometry::shiftSliceAxis(wx, wy, wz, axis);
    uint64_t bodyId = flyem::ZSegmentationAnalyzer::channelCodeToId(
          segmentation->color(wx, wy, wz));

#ifdef _DEBUG_2
    if (bodyId < 0) {
      vector<double> color = segmentation->color(x, y, z);
      cout << color[0] << " " << color[1] << " " << color[2] << endl;
      cout << FlyEm::ZSegmentationAnalyzer::channelCodeToId(color) << endl;
    }
#endif

    info += QString("%1").arg(bodyId);
  }

  return info;
}
/*
void ZFlyEmStackDoc::setSegmentation(ZStack *seg)
{
  deprecate(STACK_SEGMENTATION);

  m_segmentation = seg;
}
*/

ZStack* ZFlyEmStackDoc::getSegmentation() const
{
  return m_segmentationBundle.getBodyStack();
}

ZStack* ZFlyEmStackDoc::stackMask() const
{
  return getSegmentation();
}

ZStack* ZFlyEmStackDoc::getStack() const
{
  if (m_segmentationBundle.getGreyScaleStack() != NULL) {
    return m_segmentationBundle.getGreyScaleStack();
  } else {
    return m_segmentationBundle.getBodyStack();
  }

  return NULL;
}


ZStack*& ZFlyEmStackDoc::stackRef()
{
  if (m_segmentationBundle.getGreyScaleStack() != NULL) {
    return m_segmentationBundle.getGreyScaleStackRef();
  } else {
    return m_segmentationBundle.getBodyStackRef();
  }
}


void ZFlyEmStackDoc::importSegmentation(const string &filePath)
{
  m_segmentationBundle.importJsonFile(filePath);
  /*
  ZStackFile file;
  file.import(filePath);
  ZStack *stack = file.readStack();
  if (stack != NULL) {
    setSegmentation(stack);
  } else {
    cout << "Failed to load " << filePath << endl;
  }
  */
}

ZObject3dArray* ZFlyEmStackDoc::getBodyObject()
{
  return m_segmentationBundle.getBodyObject();
  /*
  updateBodyObject();

  return &m_bodyObject;
  */
}

ZObject3dArray* ZFlyEmStackDoc::getBodyBoundaryObject()
{
  return m_segmentationBundle.getBodyBoundaryObject();
}

map<int,size_t>* ZFlyEmStackDoc::getBodyIndexMap()
{
  return m_segmentationBundle.getBodyIndexMap();
  /*
  updateBodyIndexMap();

  return &m_bodyIndexMap;
  */
}

/*
void ZFlyEmStackDoc::updateBodyObject()
{
  if (isDeprecated(SEGMENTATION_OBJECT)) {
    m_bodyObject.clear();
    m_bodyIndexMap.clear();

    ZStack *segmentation = getSegmentation();

    if (segmentation != NULL) {
      for (int z = 0; z < segmentation->depth(); ++z) {
        for (int y = 0; y < segmentation->height(); ++y) {
          for (int x = 0; x < segmentation->width(); ++x) {
            int id = FlyEm::ZSegmentationAnalyzer::
                channelCodeToId(segmentation->color(x, y, z));
            if (m_bodyIndexMap.count(id) == 0) {
              m_bodyIndexMap[id] = m_bodyObject.size();
              m_bodyObject.resize(m_bodyObject.size() + 1);
            }

            m_bodyObject[m_bodyIndexMap[id]].append(x, y, z);
          }
        }
      }
    }
  }
}

void ZFlyEmStackDoc::updateBodyIndexMap()
{
  if (isDeprecated(SEGMENTATION_INDEX_MAP)) {
    updateBodyObject();
    TZ_ASSERT(!isDeprecated(SEGMENTATION_INDEX_MAP),
              "Index map should be ready now");
  }
}
*/
void ZFlyEmStackDoc::deprecateDependent(EComponent component)
{
  switch (component) {
  case EComponent::STACK:
    if (stackRef() == m_segmentationBundle.getBodyStackRef()) {
      deprecateDependent(EComponent::STACK_SEGMENTATION);
    }
    break;
  case EComponent::STACK_SEGMENTATION:
    deprecate(EComponent::SEGMENTATION_OBJECT);
    deprecate(EComponent::SEGMENTATION_GRAPH);
    break;
  case EComponent::SEGMENTATION_OBJECT:
    deprecate(EComponent::SEGMENTATION_INDEX_MAP);
    break;
  default:
    break;
  }
}

void ZFlyEmStackDoc::deprecate(EComponent component)
{
  deprecateDependent(component);

  switch (component) {
  case EComponent::STACK:
    ZStackDoc::deprecate(component);
    break;
  case EComponent::STACK_SEGMENTATION:
    m_segmentationBundle.deprecate(flyem::ZSegmentationBundle::BODY_STACK);
    break;
  case EComponent::SEGMENTATION_OBJECT:
    break;
  case EComponent::SEGMENTATION_INDEX_MAP:
    break;
  case EComponent::SEGMENTATION_GRAPH:
    m_segmentationBundle.deprecate(flyem::ZSegmentationBundle::BODY_GRAPH);
    break;
  default:
    break;
  }
}

bool ZFlyEmStackDoc::isDeprecated(EComponent component)
{
  switch (component) {
  case EComponent::STACK:
    return ZStackDoc::isDeprecated(component);
  case EComponent::STACK_SEGMENTATION:
    return m_segmentationBundle.isDeprecated(
          flyem::ZSegmentationBundle::BODY_STACK);
  case EComponent::SEGMENTATION_OBJECT:
    return m_segmentationBundle.isDeprecated(
          flyem::ZSegmentationBundle::BODY_STACK);
  case EComponent::SEGMENTATION_INDEX_MAP:
    return m_segmentationBundle.isDeprecated(
          flyem::ZSegmentationBundle::BODY_INDEX_MAP);
  case EComponent::SEGMENTATION_GRAPH:
    return m_segmentationBundle.isDeprecated(
          flyem::ZSegmentationBundle::BODY_GRAPH);
  default:
    break;
  }

  return false;
}

ZObject3d* ZFlyEmStackDoc::getBodyObject(int id)
{
  ZObject3dArray *bodyArray = getBodyObject();
  map<int, size_t> *bodyIndexMap = getBodyIndexMap();

  if (bodyArray == NULL) {
    return NULL;
  }

  if (bodyIndexMap->count(id) == 0) {
    return NULL;
  }

  return (*bodyArray)[(*bodyIndexMap)[id]];
}

ZObject3d* ZFlyEmStackDoc::getBodyBoundaryObject(int id)
{
  ZObject3dArray *bodyArray = getBodyBoundaryObject();
  map<int, size_t> *bodyIndexMap = getBodyIndexMap();

  if (bodyArray == NULL) {
    return NULL;
  }

  if (bodyIndexMap->count(id) == 0) {
    return NULL;
  }

  return (*bodyArray)[(*bodyIndexMap)[id]];
}



vector<ZObject3d*> ZFlyEmStackDoc::getNeighborBodyObject(int id)
{
  vector<ZObject3d*> neighborBodyObject;

  ZGraph *graph = getBodyGraph();

  if (graph != NULL) {
    set<int> neighborBodyId = graph->getNeighborSet(id);
    for (set<int>::const_iterator iter = neighborBodyId.begin();
         iter != neighborBodyId.end(); ++iter) {
      neighborBodyObject.push_back(getBodyObject(*iter));
    }
  }

  return neighborBodyObject;
}

ZObject3d* ZFlyEmStackDoc::getBodyObjectBorder(int id1, int id2)
{
  return m_segmentationBundle.getBodyBorderObject(id1, id2);
}

bool ZFlyEmStackDoc::importAxonExport(const string &filePath)
{
  FILE *fp = fopen(filePath.c_str(), "r");
  if (fp == NULL) {
    return false;
  }

  ZString str;
  bool bookmarkStart = false;

  ZSwcTree *tree = new ZSwcTree();
  tree->forceVirtualRoot();

  map<int, Swc_Tree_Node*> nodeMap;
  std::string axonTag;
  while (str.readLine(fp)) {
    if (!bookmarkStart) {
      if (str.startsWith("bookmarks")) {
        bookmarkStart = true;
        if (str.contains("axon1")) {
          axonTag = "axon1";
        } else if (str.contains("axon2")) {
          axonTag = "axon2";
        }
      }
    } else {
      if (str.contains(axonTag)){
        vector<int> bookmark = str.toIntegerArray();

        if (bookmark.size() >= 3) {
          Swc_Tree_Node *tn = New_Swc_Tree_Node();
          SwcTreeNode::setPos(tn, bookmark[0], bookmark[1], bookmark[2]);
          SwcTreeNode::setRadius(tn, 5.0);
          //SwcTreeNode::setParent(tn, currentEnd);
          //SwcTreeNode::setId(tn, SwcTreeNode::id(currentEnd) + 1);
          nodeMap[SwcTreeNode::z(tn)] = tn;
          //currentEnd = tn;
        }
      }
    }
  }

  Swc_Tree_Node *currentEnd = tree->data()->root;

  for (map<int, Swc_Tree_Node*>::iterator iter = nodeMap.begin();
       iter != nodeMap.end(); ++iter) {
    Swc_Tree_Node *tn = iter->second;
    SwcTreeNode::setParent(tn, currentEnd);
    SwcTreeNode::setId(tn, SwcTreeNode::id(currentEnd) + 1);
    currentEnd = tn;
  }

  if (tree->hasRegularNode()) {
    tree->setSource(filePath);
    addObject(tree);
  } else {
    delete tree;
    return false;
  }

  return true;
}

