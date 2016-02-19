#include "zstackdocreader.h"
#include "zfiletype.h"
#include "zswctree.h"
#include "zlocsegchain.h"
#include "zsparseobject.h"
#include "zstackfactory.h"
#include "zstack.hxx"
#include "zswcnetwork.h"
#include "zpunctum.h"
#include "zpunctumio.h"


///////////Stack Reader///////////

ZStackDocReader::ZStackDocReader() : m_stack(NULL), m_sparseStack(NULL),
  m_swcNetwork(NULL)
{

}

ZStackDocReader::~ZStackDocReader()
{
  clear();
}

bool ZStackDocReader::readFile(const QString &filePath)
{
  m_filePath = filePath;

  switch (ZFileType::fileType(filePath.toStdString())) {
  case ZFileType::SWC_FILE:
    loadSwc(filePath);
    break;
  case ZFileType::LOCSEG_CHAIN_FILE:
    loadLocsegChain(filePath);
    break;
  case ZFileType::SWC_NETWORK_FILE:
    loadSwcNetwork(filePath);
    break;
  case ZFileType::OBJECT_SCAN_FILE:
  case ZFileType::TIFF_FILE:
  case ZFileType::V3D_PBD_FILE:
  case ZFileType::LSM_FILE:
  case ZFileType::V3D_RAW_FILE:
  case ZFileType::MC_STACK_RAW_FILE:
  case ZFileType::DVID_OBJECT_FILE:
    loadStack(filePath);
    break;
  case ZFileType::V3D_APO_FILE:
  case ZFileType::V3D_MARKER_FILE:
  case ZFileType::RAVELER_BOOKMARK:
    loadPuncta(filePath);
    break;
  default:
    return false;
    break;
  }

  return true;
}

void ZStackDocReader::loadSwc(const QString &filePath)
{
  ZSwcTree *tree = new ZSwcTree();
  tree->load(filePath.toLocal8Bit().constData());
  if (!tree->isEmpty()) {
    addObject(tree);
    //addSwcTree(tree);
  } else {
    delete tree;
  }
}

void ZStackDocReader::loadLocsegChain(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    ZLocsegChain *chain = new ZLocsegChain();
    chain->load(filePath.toLocal8Bit().constData());
    if (!chain->isEmpty()) {
      addObject(chain);
      //addLocsegChain(chain);
    } else {
      delete chain;
    }
  }
}

/*
void ZStackDocReader::addLocsegChain(ZLocsegChain *chain)
{
  if (chain != NULL) {
    m_chainList.append(chain);
  }
}
*/
void ZStackDocReader::loadStack(const QString &filePath)
{
  ZFileType::EFileType type = ZFileType::fileType(filePath.toStdString());
  if (type == ZFileType::OBJECT_SCAN_FILE ||
      type == ZFileType::DVID_OBJECT_FILE) {
    ZSparseObject *sobj = new ZSparseObject;
    if (type == ZFileType::DVID_OBJECT_FILE) {
      sobj->importDvidObject(filePath.toStdString());
    } else {
      sobj->load(filePath.toStdString().c_str());
    }
    if (!sobj->isEmpty()) {
      addObject(sobj);
      sobj->setColor(255, 255, 255, 255);
      //addSparseObject(sobj);
//      sobj->setColor(128, 0, 0, 255);

      ZIntCuboid cuboid = sobj->getBoundBox();
      m_stack = ZStackFactory::makeVirtualStack(
            cuboid.getWidth(), cuboid.getHeight(), cuboid.getDepth());
      m_stack->setOffset(cuboid.getFirstCorner());
    }
  } else {
    m_stackSource.import(filePath.toStdString());
    m_stack = m_stackSource.readStack();
  }
}

void ZStackDocReader::clear()
{
  m_stack = NULL;
  m_sparseStack = NULL;
  m_stackSource.clear();
  m_playerList.clear();
  m_objectGroup.removeAllObject(false);
}

void ZStackDocReader::loadSwcNetwork(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    if (m_swcNetwork == NULL) {
      m_swcNetwork = new ZSwcNetwork;
    }

    m_swcNetwork->importTxtFile(filePath.toStdString());

    for (size_t i = 0; i < m_swcNetwork->treeNumber(); i++) {
      addObject(m_swcNetwork->getTree(i));
      //addSwcTree(m_swcNetwork->getTree(i));
    }
  }
}

void ZStackDocReader::loadPuncta(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    QList<ZPunctum*> plist = ZPunctumIO::load(filePath);
    foreach (ZPunctum* punctum, plist) {
      addObject(punctum, false);
      //addPunctum(punctum);
    }
  }
}

/*
void ZStackDocReader::addPunctum(ZPunctum *p)
{
  if (p != NULL) {
    m_punctaList.append(p);
  }
}

void ZStackDocReader::addStroke(ZStroke2d *stroke)
{
  if (stroke != NULL) {
    m_strokeList.append(stroke);
  }
}

void ZStackDocReader::addSparseObject(ZSparseObject *obj)
{
  if (obj != NULL) {
    m_sparseObjectList.append(obj);
  }
}
*/
void ZStackDocReader::setSparseStack(ZSparseStack *spStack)
{
  m_sparseStack = spStack;
}

void ZStackDocReader::setStack(ZStack *stack)
{
  m_stack = stack;
}

void ZStackDocReader::setStackSource(const ZStackFile &stackFile)
{
  m_stackSource = stackFile;
}

bool ZStackDocReader::hasData() const
{
  if (getStack() != NULL) {
    return true;
  }

  if (m_sparseStack != NULL) {
    return true;
  }

  if (!getObjectGroup().isEmpty()) {
    return true;
  }

  /*
  if (!getSwcList().isEmpty()) {
    return true;
  }

  if (!getPunctaList().isEmpty()) {
    return true;
  }

  if (!getStrokeList().isEmpty()) {
    return true;
  }

  if (!getObjectList().isEmpty()) {
    return true;
  }

  if (!getChainList().isEmpty()) {
    return true;
  }
  */

  return false;
}

void ZStackDocReader::addPlayer(ZStackObject *obj)
{
  if (obj != NULL) {
    if (obj->hasRole()) {
      ZDocPlayer *player = NULL;
      switch (obj->getType()) {
      case ZStackObject::TYPE_OBJ3D:
        player = new ZObject3dPlayer(obj);
        break;
      default:
        player = new ZDocPlayer(obj);
        break;
      }
      m_playerList.append(player);
    }
  }
}

void ZStackDocReader::addObject(ZStackObject *obj, bool uniqueSource)
{
  if (obj != NULL) {
    m_objectGroup.add(obj, uniqueSource);
    addPlayer(obj);
  }
}
