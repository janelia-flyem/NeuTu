#ifndef QTHREADFUTUREMAP_H
#define QTHREADFUTUREMAP_H

#include <QMap>
#include <QHash>
#include <QString>
#include <QFuture>

class QThreadFutureMap : public QHash<QString, QFuture<void> >
{
public:
  QThreadFutureMap();

  bool hasFuture(const QString &id) const;
  void removeDeadThread();
  int getLivingThreadNumber() const;
  QFuture<void>* getFuture(const QString &id);
  bool isAlive(const QString &id);

  bool hasThreadAlive() const;

};

#endif // QTHREADFUTUREMAP_H
