#ifndef ZFLYEMSUPERVISOR_H
#define ZFLYEMSUPERVISOR_H

#include <QObject>
#include <QString>

#include "core/neutube_def.h"
#include "dvid/zdvidtarget.h"
#include "zsharedpointer.h"

namespace libdvid{
class DVIDConnection;
}

class ZFlyEmSupervisor : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmSupervisor(QObject *parent = 0);
  bool checkIn(uint64_t bodyId, flyem::EBodySplitMode mode);
  bool checkOut(uint64_t bodyId, flyem::EBodySplitMode mode);

  bool checkInAdmin(uint64_t bodyId);

  std::string getOwner(uint64_t bodyId) const;

  /*!
   * \brief Test if the server is normal
   *
   * \return the status code from the server if the server address is valid.
   *         Otherwise it returns 0.
   */
  int testServer();

  bool isLocked(uint64_t bodyId) const;

  void setDvidTarget(const ZDvidTarget &target);
  const ZDvidTarget& getDvidTarget() const;

  bool isEmpty() const;

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
//    EBodyStatus m_bodyStatus;
    std::string m_user;
//    uint64_t m_bodyId;
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

  std::string getCheckinUrl(
      const std::string &uuid, uint64_t bodyId, flyem::EBodySplitMode mode) const;
  std::string getCheckoutUrl(
      const std::string &uuid, uint64_t bodyId, flyem::EBodySplitMode mode) const;

  std::string getCheckinUrl(const std::string &uuid, uint64_t bodyId,
                            const std::string &userName) const;

  std::string getCheckinUrl(uint64_t bodyId, flyem::EBodySplitMode mode) const;
  std::string getCheckoutUrl(uint64_t bodyId, flyem::EBodySplitMode mode) const;

signals:

public slots:

private:
  static std::string GetUserName(
      const std::string &userName, flyem::EBodySplitMode mode);
  std::string getUserName(flyem::EBodySplitMode mode) const;

private:
  std::string m_userName;
  ZDvidTarget m_dvidTarget;
  std::string m_server;

#if defined(_ENABLE_LIBDVIDCPP_)
  ZSharedPointer<libdvid::DVIDConnection> m_connection;
#endif
};

#endif // ZFLYEMSUPERVISOR_H
