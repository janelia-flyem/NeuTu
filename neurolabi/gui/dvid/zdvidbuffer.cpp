#include "zdvidbuffer.h"
#include <QDebug>
#include "dvid/zdvidclient.h"

ZDvidBuffer::ZDvidBuffer(ZDvidClient *parent) :
  QObject(parent), m_dvidClient(parent)
{
}

ZDvidBuffer::~ZDvidBuffer()
{
  clear();
}

void ZDvidBuffer::importSparseObject()
{
  if (m_dvidClient != NULL) {
    const ZObject3dScan &obj = m_dvidClient->getObject();
    m_bodyArray.append(obj);
    qDebug() << "Emitting dataTransfered from importSparseObject()";
    emit dataTransfered();
  }
}

void ZDvidBuffer::importSwcTree()
{
  if (m_dvidClient != NULL) {
    m_swcTreeArray.append(m_dvidClient->getSwcTree().clone());
    qDebug() << "Emitting dataTransfered from importSwcTree()";
    emit dataTransfered();
  }
}

void ZDvidBuffer::importImage()
{
  if (m_dvidClient != NULL) {
    ZStack *image = m_dvidClient->getImage().clone();
    m_imageArray.append(image);

    qDebug() << "Emitting dataTransfered from importImage()";
    emit dataTransfered();
  }
}

void ZDvidBuffer::clear()
{
  m_bodyArray.clear();
  foreach (ZSwcTree *tree, m_swcTreeArray) {
    delete tree;
  }
  m_swcTreeArray.clear();

  foreach (ZStack *stack, m_imageArray) {
    delete stack;
  }
  m_imageArray.clear();
}
