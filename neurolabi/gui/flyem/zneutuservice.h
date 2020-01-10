#ifndef ZNEUTUSERVICE_H
#define ZNEUTUSERVICE_H

#include <string>
#include <vector>
#include <stdint.h>
#include <set>
#include <cstdint>

#include <QMutex>

#include "common/zsharedpointer.h"

class ZDvidTarget;
namespace libdvid {
class DVIDConnection;
}

class ZNeutuService
{
public:
  ZNeutuService(const std::string &server = "");

  void setServer(const std::string &server);
  std::string getServer() const {
    return m_server;
  }

  enum class EStatus {
    NORMAL, DOWN
  };

  enum EUpdateOption {
    UPDATE_ALL, UPDATE_INVALIDATE, UPDATE_DELETE, UPDATE_MISSING
  };

  enum ERequestStatus {
    REQUEST_IGNORED, REQUEST_FAILED, REQUEST_SUCC
  };

  ERequestStatus requestBodyUpdate(const ZDvidTarget &target, uint64_t bodyId,
                         EUpdateOption option);
  ERequestStatus requestBodyUpdate(const ZDvidTarget &target,
                         const std::vector<uint64_t> &bodyIdArray,
                         EUpdateOption option);
  ERequestStatus requestBodyUpdate(const ZDvidTarget &target,
                         const std::set<uint64_t> &bodyIdArray,
                         EUpdateOption option);

  std::string getBodyUpdateUrl() const;
  static std::string GetBodyUpdatePath();
  std::string getHomeUrl() const;
  static std::string GetHomePath();
  EStatus getStatus() const {
    return m_status;
  }

  void reset();
  void updateStatus();

  bool isNormal() const;


private:
  template <class InputIterator>
  ERequestStatus requestBodyUpdate(
      const ZDvidTarget &target, const InputIterator &first,
      const InputIterator &last, EUpdateOption option);

private:
  std::string m_server;
  EStatus m_status;
#if defined(_ENABLE_LIBDVIDCPP_)
  ZSharedPointer<libdvid::DVIDConnection> m_connection;
  QMutex m_connectionMutex;
#endif
};

#endif // ZNEUTUSERVICE_H
