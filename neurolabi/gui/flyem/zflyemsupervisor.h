#ifndef ZFLYEMSUPERVISOR_H
#define ZFLYEMSUPERVISOR_H

#include <QObject>
#include <QString>

#include "dvid/zdvidtarget.h"

class ZFlyEmSupervisor : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmSupervisor(QObject *parent = 0);
  bool checkIn(uint64_t bodyId);
  bool checkOut(uint64_t bodyId);

  void setDvidTarget(const ZDvidTarget &target);

signals:

public slots:

private:
  std::string m_userName;
  ZDvidTarget m_dvidTarget;
};

#endif // ZFLYEMSUPERVISOR_H
