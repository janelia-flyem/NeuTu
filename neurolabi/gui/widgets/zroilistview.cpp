#include "zroilistview.h"

#include "flyem/zflyemroiobjsmodel.h"
#include "mvc/zstackdoc.h"

ZRoiListView::ZRoiListView(ZStackDocPtr doc, QWidget *parent) :
  QTreeView(parent)
{
  m_model = new ZFlyEMRoiObjsModel(doc, this);
  init();
}

ZRoiListView::~ZRoiListView()
{

}

void ZRoiListView::init()
{
  ZStackDoc *doc = m_model->getDocument().get();

  if (doc != NULL) {

  }
}


