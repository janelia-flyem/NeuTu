#include "qthreadfuturemap.h"

QThreadFutureMap::QThreadFutureMap()
{
}

QThreadFutureMap::~QThreadFutureMap()
{
  waitForFinished();
}

void QThreadFutureMap::waitForFinished()
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    if (!(iter.value().isFinished())) {
      iter.value().waitForFinished();
    }
  }
}

bool QThreadFutureMap::hasFuture(const QString &id) const
{
  return contains(id);
}

QFuture<void> *QThreadFutureMap::getFuture(const QString &id)
{
  QFuture<void> *future = NULL;

  if (hasFuture(id)) {
    future = &((*this)[id]);
  }

  return future;
}

bool QThreadFutureMap::isAlive(const QString &id)
{
  QFuture<void> *future = getFuture(id);
  if (future != NULL) {
    return !future->isFinished();
  }

  return false;
}

void QThreadFutureMap::removeDeadThread()
{
  QList<QString> keyList = this->keys();
  foreach (const QString &key, keyList) {
    QFuture<void> *future = getFuture(key);
    if (future != NULL) {
      if (future->isFinished()) {
        remove(key);
      }
    }
  }
}

int QThreadFutureMap::getLivingThreadNumber() const
{
  int count = 0;
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    if (!(iter.value().isFinished())) {
      ++count;
    }
  }

  return count;
}

bool QThreadFutureMap::hasThreadAlive() const
{
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    if (!(iter.value().isFinished())) {
      return true;
    }
  }

  return false;
}
