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

  bool checkInAdmin(uint64_t bodyId);

  std::string getOwner(uint64_t bodyId) const;

  bool isLocked(uint64_t bodyId) const;

  void setDvidTarget(const ZDvidTarget &target);
  const ZDvidTarget& getDvidTarget() const;

  void setSever(const std::string &server);

  inline const std::string &getUserName() const {
    return m_userName;
  }

  void setUserName(const std::string userName);

  enum EBodyStatus {
    BODY_CHECKED_OUT, BODY_FREE, BODY_UNKNOWN
  };

  class BodyStatus {
  private:
    EBodyStatus m_bodyStatus;
    std::string m_user;
    uint64_t m_bodyId;
  };

  std::string getUuid() const;

  std::vector<std::string> getUuidList() const;


  std::string getMainUrl() const;
  std::string getUuidsUrl() const;
  std::string getStateUrl(const std::string &uuid) const;
  std::string getStateUrl() const;
  std::string getHistoryUrl(const std::string &uuid) const;
  std::string getHistoryUrl() const;

  std::string getCheckinUrl(const std::string &uuid) const;
  std::string getCheckoutUrl(const std::string &uuid) const;

  std::string getCheckinUrl(const std::string &uuid, uint64_t bodyId) const;
  std::string getCheckoutUrl(const std::string &uuid, uint64_t bodyId) const;

  std::string getCheckinUrl(const std::string &uuid, uint64_t bodyId,
                            const std::string &userName) const;

  std::string getCheckinUrl(uint64_t bodyId) const;
  std::string getCheckoutUrl(uint64_t bodyId) const;

signals:

public slots:

private:
  std::string m_userName;
  ZDvidTarget m_dvidTarget;
  std::string m_server;
};

#endif // ZFLYEMSUPERVISOR_H
