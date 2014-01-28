#ifndef ZDVIDBUFFER_H
#define ZDVIDBUFFER_H

#include <QObject>
#include "zobject3dscan.h"
#include "zswctree.h"

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

  inline const QVector<ZObject3dScan>& getBodyArray() const {
    return m_bodyArray;
  }

  inline const QVector<ZSwcTree*>& getSwcTreeArray() const {
    return m_swcTreeArray;
  }

  void clear();

private:
  QVector<ZObject3dScan> m_bodyArray;
  QVector<ZSwcTree*> m_swcTreeArray;

  ZDvidClient *m_dvidClient;
};

#endif // ZDVIDBUFFER_H
