#ifndef ZDVIDBUFFER_H
#define ZDVIDBUFFER_H

#include <QObject>
#include "zobject3dscan.h"
#include "zswctree.h"
#include "zstack.hxx"

class ZDvidClient;

class ZDvidBuffer : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidBuffer(ZDvidClient *parent = 0);
  ~ZDvidBuffer();

signals:
  void dataTransfered();

public slots:
  void importSwcTree();
  void importSparseObject();
  void importImage();

  inline const QVector<ZObject3dScan>& getBodyArray() const {
    return m_bodyArray;
  }

  inline const QVector<ZSwcTree*>& getSwcTreeArray() const {
    return m_swcTreeArray;
  }

  inline const QVector<ZStack*>& getImageArray() const {
    return m_imageArray;
  }

  void clear();

private:
  QVector<ZObject3dScan> m_bodyArray;
  QVector<ZSwcTree*> m_swcTreeArray;
  QVector<ZStack*> m_imageArray;

  ZDvidClient *m_dvidClient;
};

#endif // ZDVIDBUFFER_H
