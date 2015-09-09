#include "zflyemexternalneurondoc.h"
#include "zstackobjectsourcefactory.h"
#include "zswctree.h"

ZFlyEmExternalNeuronDoc::ZFlyEmExternalNeuronDoc(QObject *parent) :
  ZStackDoc(parent)
{
}

ZFlyEmExternalNeuronDoc::~ZFlyEmExternalNeuronDoc()
{

}

void ZFlyEmExternalNeuronDoc::setDataDoc(ZSharedPointer<ZStackDoc> doc)
{
  m_dataDoc = doc;
  removeAllObject(true);

  TStackObjectList objList = m_dataDoc->getObjectGroup().findSameClass(
        ZStackObject::TYPE_SWC,
        ZStackObjectSourceFactory::MakeFlyEmExtNeuronClass());

  for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
       ++iter) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
    addObject(tree->clone());
  }
}
