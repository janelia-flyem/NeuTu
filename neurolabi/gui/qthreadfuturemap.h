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

};

#endif // QTHREADFUTUREMAP_H
