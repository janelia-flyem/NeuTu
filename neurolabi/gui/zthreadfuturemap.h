#ifndef QTHREADFUTUREMAP_H
#define QTHREADFUTUREMAP_H

#include <QMap>
#include <QHash>
#include <QString>
#include <QFuture>

class ZThreadFutureMap : public QHash<QString, QFuture<void> >
{
public:
  ZThreadFutureMap();
  ~ZThreadFutureMap();

  bool hasFuture(const QString &id) const;
  void removeDeadThread();
  int getLivingThreadNumber() const;
  QFuture<void>* getFuture(const QString &id);
  bool isAlive(const QString &id) const;

  bool hasThreadAlive() const;

  /*!
   * \brief Wait the thread with a certain ID to be finished.
   *
   * \param id The ID of the thread to be waited.
   */
  void waitForFinished(const QString &id);

  void waitForFinished();

};

#endif // QTHREADFUTUREMAP_H
