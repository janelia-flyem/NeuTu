#include "zbiocytinprojectiondoc.h"

ZBiocytinProjectionDoc::ZBiocytinProjectionDoc(QObject *parent) :
  ZStackDoc(parent)
{
  setTag(NeuTube::Document::BIOCYTIN_PROJECTION);
}

void ZBiocytinProjectionDoc::setParentDoc(ztr1::shared_ptr<ZStackDoc> parentDoc)
{
  m_parentDoc = parentDoc;
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
