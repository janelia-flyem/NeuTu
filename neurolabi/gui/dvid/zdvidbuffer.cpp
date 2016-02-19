#include "zdvidbuffer.h"
#include <QDebug>
#include "dvid/zdvidclient.h"
#include "zswctree.h"

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
    if (!m_dvidClient->getImage().isEmpty()) {
      ZStack *image = m_dvidClient->getImage().clone();
      m_imageArray.append(image);

      qDebug() << "Emitting dataTransfered from importImage()";
      emit dataTransfered();
    }
  }
}

void ZDvidBuffer::importInfo()
{
  if (m_dvidClient != NULL) {
    m_infoArray.append(m_dvidClient->getInfo());
    qDebug() << "Emitting dataTransered from importInfo()";
    emit dataTransfered();
  }
}

void ZDvidBuffer::importKeyValue()
{
  if (m_dvidClient != NULL) {
    m_keyValueArray.append(m_dvidClient->getKeyValue());
    qDebug() << "Emitting dataTransered from importKeyValue()";
    emit dataTransfered();
  }
}

void ZDvidBuffer::importKeys()
{
  if (m_dvidClient != NULL) {
    m_keysArray.append(m_dvidClient->getKeys());
    qDebug() << "Emitting dataTransered from importKeys()";
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

  m_infoArray.clear();
  m_keysArray.clear();
  m_keyValueArray.clear();
}

void ZDvidBuffer::clearInfoArray()
{
  m_infoArray.clear();
}

void ZDvidBuffer::clearKeyValueArray()
{
  m_keyValueArray.clear();
}

void ZDvidBuffer::clearKeysArray()
{
  m_keysArray.clear();
}

void ZDvidBuffer::clearImageArray()
{
  foreach (ZStack *stack, m_imageArray) {
    delete stack;
  }
  m_imageArray.clear();
}

void ZDvidBuffer::clearTreeArray()
{
  foreach (ZSwcTree *tree, m_swcTreeArray) {
    delete tree;
  }
  m_swcTreeArray.clear();
}

void ZDvidBuffer::clearBodyArray()
{
  m_bodyArray.clear();
}
