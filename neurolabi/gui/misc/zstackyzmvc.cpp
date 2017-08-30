#include "zstackyzmvc.h"
#include "zstackyzview.h"

ZStackYZMvc::ZStackYZMvc(QWidget *parent) :
  ZStackMvc(parent)
{
}


void ZStackYZMvc::createView()
{
  if (m_doc.get() != NULL) {
    //ZIntPoint size = m_doc->getStackSize();
    m_view = new ZStackView(qobject_cast<QWidget*>(this));
    m_view->setSliceAxis(NeuTube::X_AXIS);
    m_layout->addWidget(m_view);
  }
}

ZStackYZMvc* ZStackYZMvc::Make(QWidget *parent, ztr1::shared_ptr<ZStackDoc> doc)
{
  ZStackYZMvc *frame = new ZStackYZMvc(parent);
  BaseConstruct(frame, doc, NeuTube::X_AXIS);

  return frame;
}


