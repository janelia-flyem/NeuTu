#include "zdvidbuffer.h"
#include <QDebug>
#include "dvid/zdvidclient.h"

ZDvidBuffer::ZDvidBuffer(ZDvidClient *parent) :
  QObject(parent), m_dvidClient(parent)
{
}

ZDvidBuffer::~ZDvidBuffer()
{

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

void ZDvidBuffer::clear()
{
  m_bodyArray.clear();
  m_swcTreeArray.clear();
}
