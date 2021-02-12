#ifndef TASKIO_H
#define TASKIO_H

#include <string>
#include <memory>

class ZJsonObject;

namespace libdvid {
class DVIDConnection;
}

namespace neutuse {

class TaskIO
{
public:
  TaskIO();
  TaskIO(const std::string &server);
  virtual ~TaskIO();

  void open(const std::string &server);
  std::string getServerAddress() const;

  void setStatusCode(int code);
  int getStatusCode() const;
  std::string getResponse() const;

  bool ready() const;

  void testConnection(const std::string &method = "HEAD");

  void reset();

protected:
  void post(const std::string &path, const ZJsonObject &obj);

protected:
  std::string m_address;
  int m_statusCode = 0;
  bool m_connected = false;
  std::string m_response;
  std::unique_ptr<libdvid::DVIDConnection> m_connection;
};

}

#endif // TASKIO_H
