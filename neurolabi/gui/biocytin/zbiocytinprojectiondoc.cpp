#include "zbiocytinprojectiondoc.h"
#include "zstackframe.h"

ZBiocytinProjectionDoc::ZBiocytinProjectionDoc(QObject *parent) :
  ZStackDoc(parent)
{
  setTag(NeuTube::Document::BIOCYTIN_PROJECTION);
}

ZBiocytinProjectionDoc::~ZBiocytinProjectionDoc()
{
  disconnect(this, SIGNAL(swcModified()),
          m_parentDoc.get(), SIGNAL(swcModified()));
  removeObject(ZStackObject::TYPE_SWC, false);
}

void ZBiocytinProjectionDoc::setParentDoc(ztr1::shared_ptr<ZStackDoc> parentDoc)
{
  m_parentDoc = parentDoc;
  connect(m_parentDoc.get(), SIGNAL(swcModified()),
          this, SLOT(updateSwc()));
  connect(this, SIGNAL(swcModified()),
          m_parentDoc.get(), SIGNAL(swcModified()));
  updateSwc();
}

void ZBiocytinProjectionDoc::updateSwc()
{
  disconnect(this, SIGNAL(swcModified()),
             m_parentDoc.get(), SIGNAL(swcModified()));
  removeObject(ZStackObject::TYPE_SWC, false);
  QList<ZSwcTree*> treeList = m_parentDoc->getSwcList();
  for (QList<ZSwcTree*>::iterator iter = treeList.begin();
       iter != treeList.end(); ++iter) {
    addObject(*iter);
  }
  connect(this, SIGNAL(swcModified()),
               m_parentDoc.get(), SIGNAL(swcModified()));
}

void ZBiocytinProjectionDoc::selectSwcNode(const ZRect2d &roi)
{
  ZStackDoc::selectSwcNode(roi);
  if (m_parentDoc.get() != NULL) {
    m_parentDoc->selectSwcNode(roi);
  }
}

bool ZBiocytinProjectionDoc::executeDeleteSwcNodeCommand()
{
  if (m_parentDoc.get() != NULL) {
    m_parentDoc->executeDeleteSwcNodeCommand();
  }

  return ZStackDoc::executeDeleteSwcNodeCommand();
}
