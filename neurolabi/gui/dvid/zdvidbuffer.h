#ifndef ZDVIDBUFFER_H
#define ZDVIDBUFFER_H

#include <QObject>
#include <QStringList>
#include "zobject3dscan.h"
#include "zstack.hxx"

class ZSwcTree;
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
  void importInfo();
  void importKeyValue();
  void importKeys();

  inline const QVector<ZObject3dScan>& getBodyArray() const {
    return m_bodyArray;
  }

  inline const QVector<ZSwcTree*>& getSwcTreeArray() const {
    return m_swcTreeArray;
  }

  inline QVector<ZSwcTree*>& getSwcTreeArray() {
    return m_swcTreeArray;
  }

  inline const QVector<ZStack*>& getImageArray() const {
    return m_imageArray;
  }

  inline const QStringList& getInfoArray() const {
    return m_infoArray;
  }

  inline const QVector<QByteArray>& getKeyValueArray() const {
    return m_keyValueArray;
  }

  inline const QVector<QByteArray>& getKeysArray() const {
    return m_keysArray;
  }

  void clear();

  void clearInfoArray();
  void clearKeyValueArray();
  void clearKeysArray();
  void clearBodyArray();
  void clearImageArray();
  void clearTreeArray();

private:
  QVector<ZObject3dScan> m_bodyArray;
  QVector<ZSwcTree*> m_swcTreeArray;
  QVector<ZStack*> m_imageArray;
  QStringList m_infoArray;
  QVector<QByteArray> m_keyValueArray;
  QVector<QByteArray> m_keysArray;

  ZDvidClient *m_dvidClient;
};

#endif // ZDVIDBUFFER_H
