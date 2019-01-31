#ifndef ZSTACKDOCDATABUFFER_H
#define ZSTACKDOCDATABUFFER_H

#include <QObject>
#include <QList>
#include <QMutex>

class ZStackObject;

class ZStackDocObjectUpdate {
public:
  enum class EAction {
    NONE, ADD_NONUNIQUE, ADD_UNIQUE,
    UPDATE, SELECT, DESELECT,
    RECYCLE, EXPEL, KILL, ADD_BUFFER
  };

  ZStackDocObjectUpdate(ZStackObject *m_obj, EAction action);
  ~ZStackDocObjectUpdate();

  void reset();

  EAction getAction() const {
    return m_action;
  }

  ZStackObject* getObject() const {
    return m_obj;
  }

  void setAction(EAction action) {
    m_action = action;
  }

  void print() const;

  static QMap<ZStackObject*, ZStackDocObjectUpdate::EAction>
  MakeActionMap(QList<ZStackDocObjectUpdate*> updateList);

private:
  ZStackObject *m_obj;
  EAction m_action;
};


class ZStackDocDataBuffer : public QObject
{
  Q_OBJECT
public:
  explicit ZStackDocDataBuffer(QObject *parent = 0);
  ~ZStackDocDataBuffer();

  void clearList();
  void clear();

  void addUpdate(ZStackObject *obj, ZStackDocObjectUpdate::EAction action);
  void addUpdate(
      QList<ZStackObject*> objList, ZStackDocObjectUpdate::EAction action);
  template<typename InputIterator>
  void addUpdate(const InputIterator &first, const InputIterator &last,
                 ZStackDocObjectUpdate::EAction action);

  QList<ZStackDocObjectUpdate *> take();
  void deliver();

  int getActionCount(ZStackDocObjectUpdate::EAction action) const;

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
