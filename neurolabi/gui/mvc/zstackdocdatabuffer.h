#ifndef ZSTACKDOCDATABUFFER_H
#define ZSTACKDOCDATABUFFER_H

#include <QObject>
#include <QList>
#include <QMutex>

#include "zstackdocobjectupdate.h"

class ZStackDocDataBuffer : public QObject
{
  Q_OBJECT
public:
  explicit ZStackDocDataBuffer(QObject *parent = nullptr);
  ~ZStackDocDataBuffer();

  void clearList();
  void clear();

  void addUpdate(ZStackObject *obj, ZStackDocObjectUpdate::EAction action);
  void addUpdate(
      QList<ZStackObject*> objList, ZStackDocObjectUpdate::EAction action);
  template<typename InputIterator>
  void addUpdate(const InputIterator &first, const InputIterator &last,
                 ZStackDocObjectUpdate::EAction action);
  void addUpdate(ZStackDocObjectUpdate *u);

  QList<ZStackDocObjectUpdate *> take();
  void deliver();

  int getActionCount(ZStackDocObjectUpdate::EAction action) const;

  void removeObjectUpdate(
      std::function<bool(ZStackDocObjectUpdate*)> pred);

  void print() const;

signals:
  void delivering();

public slots:

private:
  QList<ZStackDocObjectUpdate*> m_updateList;

  mutable QMutex m_mutex;
};

template<typename InputIterator>
void ZStackDocDataBuffer::addUpdate(
    const InputIterator &first, const InputIterator &last,
    ZStackDocObjectUpdate::EAction action)
{
  for (InputIterator iter = first; iter != last; ++iter) {
    addUpdate(*iter, action);
  }
}

#endif // ZSTACKDOCDATABUFFER_H
