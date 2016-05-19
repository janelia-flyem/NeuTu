#ifndef ZNEUTUSERVICE_H
#define ZNEUTUSERVICE_H

#include <string>
#include <vector>
#include <stdint.h>

#include "tz_stdint.h"

class ZDvidTarget;

class ZNeutuService
{
public:
  ZNeutuService(const std::string &server = "");

  void setServer(const std::string &server);
  std::string getServer() const {
    return m_server;
  }

  enum EStatus {
    STATUS_NORMAL, STATUS_DOWN
  };

  enum EUpdateOption {
    UPDATE_ALL, UPDATE_INVALIDATE, UPDATE_DELETE
  };

  enum ERequestStatus {
    REQUEST_IGNORED, REQUEST_FAILED, REQUEST_SUCC
  };

  ERequestStatus requestBodyUpdate(const ZDvidTarget &target, uint64_t bodyId,
                         EUpdateOption option);
  ERequestStatus requestBodyUpdate(const ZDvidTarget &target,
                         const std::vector<uint64_t> &bodyIdArray,
                         EUpdateOption option);

  std::string getBodyUpdateUrl() const;
  std::string getHomeUrl() const;
  EStatus getStatus() const {
    return m_status;
  }

  void updateStatus();

  bool isNormal() const;

private:
  std::string m_server;
  EStatus m_status;
};

#endif // ZNEUTUSERVICE_H
