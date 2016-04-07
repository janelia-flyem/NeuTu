#include "zthreadfuturemap.h"

ZThreadFutureMap::ZThreadFutureMap()
{
}

ZThreadFutureMap::~ZThreadFutureMap()
{
  waitForFinished();
}

void ZThreadFutureMap::waitForFinished()
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    if (!(iter.value().isFinished())) {
      iter.value().waitForFinished();
    }
  }
}

bool ZThreadFutureMap::hasFuture(const QString &id) const
{
  return contains(id);
}

QFuture<void> *ZThreadFutureMap::getFuture(const QString &id)
{
  QFuture<void> *future = NULL;

  if (hasFuture(id)) {
    future = &((*this)[id]);
  }

  return future;
}

bool ZThreadFutureMap::isAlive(const QString &id)
{
  QFuture<void> *future = getFuture(id);
  if (future != NULL) {
    return !future->isFinished();
  }

  return false;
}

void ZThreadFutureMap::removeDeadThread()
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

int ZThreadFutureMap::getLivingThreadNumber() const
{
  int count = 0;
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    if (!(iter.value().isFinished())) {
      ++count;
    }
  }

  return count;
}

bool ZThreadFutureMap::hasThreadAlive() const
{
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    if (!(iter.value().isFinished())) {
      return true;
    }
  }

  return false;
}
