#include "qthreadfuturemap.h"

QThreadFutureMap::QThreadFutureMap()
{
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
