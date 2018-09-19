#include "taskio.h"

#include "zqslog.h"
#include "zdvidutil.h"
#include "neutubeconfig.h"

namespace neutuse {

TaskIO::TaskIO()
{
}

TaskIO::~TaskIO()
{

}

void TaskIO::setStatusCode(int code)
{
  m_statusCode = code;
}

int TaskIO::getStatusCode() const
{
  return m_statusCode;
}

std::string TaskIO::getResponse() const
{
  return m_response;
}

void TaskIO::open(const std::string &server)
{
  try {
    m_connection = std::unique_ptr<libdvid::DVIDConnection>(
          new libdvid::DVIDConnection(server, GET_FLYEM_CONFIG.getUserName(),
                                      NeutubeConfig::GetSoftwareName()));
  } catch (std::exception &e){
    m_connection.reset();
    LWARN() << "Failed to connect to" << server;
  }
}

bool TaskIO::ready() const
{
  return (m_connection.get() != nullptr);
}

void TaskIO::post(const std::string &path, const ZJsonObject &obj)
{
  if (ready()) {
    libdvid::BinaryDataPtr response = ZDvid::MakePostRequest(
          *m_connection, path, obj, m_statusCode);
    m_response = response->get_data();
  }
}

}
