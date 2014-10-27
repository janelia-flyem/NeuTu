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

  if (!hasFuture(id)) {
    future = &((*this)[id]);
  }

  return future;
}

void QThreadFutureMap::removeDeadThread()
{
  QList<QString> keyList = this->keys();
  foreach (const QString &key, keyList) {
    QFuture<void> *future = getFuture(key);
    if (future != NULL) {
      if (future->isCanceled() || future->isFinished()) {
        remove(key);
      }
    }
  }
}

int QThreadFutureMap::getLivingThreadNumber() const
{
  int count = 0;
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    if (!(iter.value().isCanceled() || iter.value().isFinished())) {
      ++count;
    }
  }

  return count;
}
