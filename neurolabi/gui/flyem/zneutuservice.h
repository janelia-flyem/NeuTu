#ifndef ZNEUTUSERVICE_H
#define ZNEUTUSERVICE_H

#include <string>
#include <vector>
#include <stdint.h>

class ZDvidTarget;

class ZNeutuService
{
public:
  ZNeutuService(const std::string &server = "");

  void setServer(const std::string &server) {
    m_server = server;
  }

  void requestBodyUpdate(const ZDvidTarget &target, uint64_t bodyId);
  void requestBodyUpdate(const ZDvidTarget &target,
                         const std::vector<uint64_t> &bodyIdArray);

  std::string getBodyUpdateUrl() const;

private:
  std::string m_server;

};

#endif // ZNEUTUSERVICE_H
