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

  switch (ZFileType::FileType(filePath.toStdString())) {
  case ZFileType::EFileType::SWC:
    loadSwc(filePath);
    break;
  case ZFileType::EFileType::LOCSEG_CHAIN:
    loadLocsegChain(filePath);
    break;
  case ZFileType::EFileType::SWC_NETWORK:
    loadSwcNetwork(filePath);
    break;
  case ZFileType::EFileType::OBJECT_SCAN:
  case ZFileType::EFileType::TIFF:
  case ZFileType::EFileType::V3D_PBD:
  case ZFileType::EFileType::LSM:
  case ZFileType::EFileType::V3D_RAW:
  case ZFileType::EFileType::MC_STACK_RAW:
  case ZFileType::EFileType::DVID_OBJECT:
    loadStack(filePath);
    break;
  case ZFileType::EFileType::V3D_APO:
  case ZFileType::EFileType::V3D_MARKER:
  case ZFileType::EFileType::RAVELER_BOOKMARK:
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
  ZFileType::EFileType type = ZFileType::FileType(filePath.toStdString());
  if (type == ZFileType::EFileType::OBJECT_SCAN ||
      type == ZFileType::EFileType::DVID_OBJECT) {
    ZSparseObject *sobj = new ZSparseObject;
    if (type == ZFileType::EFileType::DVID_OBJECT) {
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
      m_stack = ZStackFactory::MakeVirtualStack(
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
      case ZStackObject::EType::OBJ3D:
        player = new ZObject3dPlayer(obj);
        break;
      default:
        player = new ZDocPlayer(obj);
        break;
      }
      m_playerList.add(player);
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
