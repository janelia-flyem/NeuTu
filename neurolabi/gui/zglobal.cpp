#include "zglobal.h"

#include <QUrl>
#include <map>
#include <QClipboard>
#include <QApplication>

#include "logging/zqslog.h"
#include "geometry/zintpoint.h"
#include "geometry/zpoint.h"
#include "zstring.h"
#include "zjsonparser.h"
#include "neutubeconfig.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"
#include "zdvidutil.h"
#include "sandbox/zbrowseropener.h"
#include "flyem/zglobaldvidrepo.h"
#include "service/neuprintreader.h"
#include "logging/neuopentracing.h"

class ZGlobalData {
public:
  ZGlobalData();
  ~ZGlobalData();

  ZIntPoint m_stackPosition;
  std::string m_3dcamera;
  std::map<std::string, ZDvidReader*> m_dvidReaderMap;
  std::map<std::string, ZDvidWriter*> m_dvidWriterMap;
  NeuPrintReader *m_neuprintReader = nullptr;
  neutu::EServerStatus m_neuprintStatus = neutu::EServerStatus::OFFLINE;
};

ZGlobalData::ZGlobalData()
{
  m_stackPosition.invalidate();
}

ZGlobalData::~ZGlobalData()
{
  for (auto &obj : m_dvidReaderMap) {
    delete obj.second;
  }

  for (auto &obj : m_dvidWriterMap) {
    delete obj.second;
  }

  delete m_neuprintReader;
}


ZGlobal::ZGlobal()
{
  m_data = new ZGlobalData;
  m_browserOpener = ZSharedPointer<ZBrowserOpener>(new ZBrowserOpener);
  m_browserOpener->setChromeBrowser();
//  m_browserOpener->setBrowserPath(ZBrowserOpener::INTERNAL_BROWSER);
}

ZGlobal::~ZGlobal()
{
  delete m_data;
  m_data = NULL;
}


ZBrowserOpener* ZGlobal::getBrowserOpener() const
{
  return m_browserOpener.get();
}

void ZGlobal::setDataPosition(int x, int y, int z)
{
  m_data->m_stackPosition.set(x, y, z);
}

ZIntPoint ZGlobal::getStackPosition() const
{
  return m_data->m_stackPosition;
}

void ZGlobal::setDataPosition(const ZIntPoint &pt)
{
  m_data->m_stackPosition = pt;
}

void ZGlobal::setDataPosition(const ZPoint &pt)
{
  setDataPosition(pt.toIntPoint());
}

void ZGlobal::clearStackPosition()
{
  m_data->m_stackPosition.invalidate();
}

void ZGlobal::set3DCamera(const std::string &config)
{
  m_data->m_3dcamera = config;
}

std::string ZGlobal::get3DCamera() const
{
  return m_data->m_3dcamera;
}

void ZGlobal::setMainWindow(QMainWindow *win)
{
  m_mainWin = win;
}

QMainWindow* ZGlobal::getMainWindow() const
{
  return m_mainWin;
}

QString ZGlobal::getNeuPrintServer() const
{
  return qgetenv("NEUPRINT");
}

void ZGlobal::setNeuPrintServer(const QString &server)
{
  if (getNeuPrintServer() != server) {
    qputenv("NEUPRINT", QByteArray::fromStdString(server.toStdString()));
    delete m_data->m_neuprintReader;
    m_data->m_neuprintReader = nullptr;
  }
}

QString ZGlobal::getNeuPrintAuth() const
{
  QString authFile = qgetenv("NEUPRINT_AUTH");
  if (authFile.isEmpty()) {
    authFile = NeutubeConfig::getInstance().getPath(
          NeutubeConfig::EConfigItem::NEUPRINT_AUTH).c_str();
    LINFO() << "NeuPrint auth path:" << authFile;
  }

  QString auth;

  QFile f(authFile);
  if (f.open(QIODevice::ReadOnly)) {
    QTextStream stream(&f);
    auth = stream.readAll();
  }

  return auth;
}

QString ZGlobal::getNeuPrintToken() const
{
//  QString auth = qgetenv("NEUPRINT_AUTH");
//  if (auth.isEmpty()) {
//    auth = NeutubeConfig::getInstance().getPath(
//          NeutubeConfig::EConfigItem::NEUPRINT_AUTH).c_str();
//    LINFO() << "NeuPrint auth path:" << auth;
//  }

  ZJsonObject obj;
  obj.decode(getNeuPrintAuth().toStdString());
  std::string token = ZJsonParser::stringValue(obj["token"]);

  return QString::fromStdString(token);
}

NeuPrintReader* ZGlobal::getNeuPrintReader()
{
  if (m_data->m_neuprintReader == nullptr) {
    m_data->m_neuprintReader = makeNeuPrintReader();
  }

  return m_data->m_neuprintReader;
}

NeuPrintReader* ZGlobal::makeNeuPrintReader()
{
  NeuPrintReader *reader = nullptr;
  QString server = qgetenv("NEUPRINT");
  if (!server.isEmpty()) {
    reader = new NeuPrintReader(server);
    reader->authorize(getNeuPrintToken());
  }

  return reader;
}

NeuPrintReader* ZGlobal::makeNeuPrintReader(const QString &uuid)
{
  NeuPrintReader *reader = nullptr;
  QString server = qgetenv("NEUPRINT");
  if (!server.isEmpty()) {
    reader = new NeuPrintReader(server);
    reader->authorize(getNeuPrintToken());
    reader->updateCurrentDataset(uuid);
    if (!reader->isReady()) {
      delete reader;
      reader = nullptr;
    }
  }

  return reader;
}

ZDvidTarget ZGlobal::getDvidTarget(const std::string &name) const
{
  return ZGlobalDvidRepo::GetInstance().getDvidTarget(name);
}

template<typename T>
T* ZGlobal::getIODevice(
    const std::string &name, std::map<std::string, T*> &ioMap,
    const std::string &key) const
{
  T *io = NULL;

  std::string nameKey = name + "$" + key;
  if (ioMap.count(nameKey) == 0) {
    {
      const ZDvidTarget &target =
          ZGlobalDvidRepo::GetInstance().getDvidTarget(name);
      if (target.isValid()) {
        io = new T;
        if (!io->open(target)) {
          delete io;
          io = NULL;
        }
      }
    }

    if (io == NULL) {
      ZDvidTarget target;
      target.setFromSourceString(name);
      if (target.isValid()) {
        io = new T;
        if (!io->open(target)) {
          delete io;
          io = NULL;
        }
      }
    }

    ioMap[nameKey] = io;
  } else {
    io = ioMap[nameKey];
  }

  return io;
}

template<typename T>
T* ZGlobal::getIODevice(const ZDvidTarget &target,
                        std::map<std::string, T*> &ioMap,
                        const std::string &key) const
{
  T *device = NULL;
  if (target.isValid()) {
    device = getIODevice(target.getSourceString(true), ioMap, key);
  }

  return device;
}

template<typename T>
T* ZGlobal::getIODeviceFromUrl(
    const std::string &path, std::map<std::string, T*> &ioMap,
    const std::string &key) const
{
  T *device = NULL;

  QUrl url(path.c_str());
  if (url.scheme() == "http" || url.scheme() == "dvid" ||
      url.scheme() == "mock") {
    ZDvidTarget target = dvid::MakeTargetFromUrl(path);
    return getIODevice(target, ioMap, key);
//    device = getIODevice(target.getSourceString(true), ioMap);
  }

  return device;
}

ZDvidReader* ZGlobal::getDvidReader(
    const std::string &name, const std::string &key) const
{
  return getIODevice(name, m_data->m_dvidReaderMap, key);
}

ZDvidWriter* ZGlobal::getDvidWriter(
    const std::string &name, const std::string &key) const
{
  return getIODevice(name, m_data->m_dvidWriterMap, key);
}

ZDvidReader* ZGlobal::getDvidReader(
    const ZDvidTarget &target, const std::string &key) const
{
  return getIODevice(target, m_data->m_dvidReaderMap, key);
}

ZDvidWriter* ZGlobal::getDvidWriter(
    const ZDvidTarget &target, const std::string &key) const
{
  return getIODevice(target, m_data->m_dvidWriterMap, key);
}

ZDvidReader* ZGlobal::getDvidReaderFromUrl(
    const std::string &url, const std::string &key) const
{
  return getIODeviceFromUrl(url, m_data->m_dvidReaderMap, key);
}

ZDvidWriter* ZGlobal::getDvidWriterFromUrl(
    const std::string &url, const std::string &key) const
{
  return getIODeviceFromUrl(url, m_data->m_dvidWriterMap, key);
}

ZDvidSparseStack* ZGlobal::readDvidSparseStack(const std::string &url) const
{
  ZDvidSparseStack *spStack = NULL;

  uint64_t bodyId = ZDvidUrl::GetBodyId(url);

  if (bodyId > 0) {
    ZDvidReader *reader = getDvidReaderFromUrl(url);
    if (reader != NULL) {
      if (reader->getDvidTarget().hasBodyLabel()) {
        spStack = reader->readDvidSparseStack(
              bodyId, neutu::EBodyLabelType::BODY);
      }
    }
  }

  return spStack;
}

ZDvidReader* ZGlobal::GetDvidReader(
    const std::string &name, const std::string &key)
{
  return GetInstance().getDvidReader(name, key);
}

ZDvidWriter* ZGlobal::GetDvidWriter(
    const std::string &name, const std::string &key)
{
  return GetInstance().getDvidWriter(name, key);
}

ZDvidReader* ZGlobal::GetDvidReader(
    const ZDvidTarget &target, const std::string &key)
{
  return GetInstance().getDvidReader(target, key);
}

ZDvidWriter* ZGlobal::GetDvidWriter(
    const ZDvidTarget &target, const std::string &key)
{
  return GetInstance().getDvidWriter(target, key);
}

ZDvidReader* ZGlobal::GetDvidReaderFromUrl(
    const std::string &url, const std::string &key)
{
  return GetInstance().getDvidReaderFromUrl(url, key);
}

ZDvidWriter* ZGlobal::GetDvidWriterFromUrl(
    const std::string &url, const std::string &key)
{
  return GetInstance().getDvidWriterFromUrl(url, key);
}

void ZGlobal::CopyToClipboard(const std::string &str)
{
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(str.c_str());
}

void ZGlobal::InitKafkaTracer(std::string serviceName)
{
#if defined(_NEU3_) || defined(_FLYEM_)
  std::string kafkaBrokers = "";

  if (!NeutubeConfig::GetUserInfo().getOrganization().empty()) {
    kafkaBrokers = "kafka.int.janelia.org:9092";
  }

  if (serviceName.empty()) {
#if defined(_NEU3_)
    serviceName = "neu3";
#else
    serviceName = "neutu";
#endif
  }

  std::string envName = "NEUTU_KAFKA_BROKERS";
#if defined(_NEU3_)
  envName = "NEU3_KAFKA_BROKERS";
#endif

  if (const char* kafkaBrokersEnv = std::getenv(envName.c_str())) {
    // The list of brokers should be separated by commans, per this example:
    // https://www.npmjs.com/package/node-rdkafka
    kafkaBrokers = kafkaBrokersEnv;
  }

//  std::cout << "Kafka broker: " << kafkaBrokers << std::endl;

  if (!kafkaBrokers.empty() && (kafkaBrokers != "none")) {
    try {
      auto config = neuopentracing::Config(kafkaBrokers);
      auto tracer = neuopentracing::Tracer::make(serviceName, config);
      neuopentracing::Tracer::InitGlobal(tracer);
      if (tracer) {
//        std::cout << "Kafka connected" << std::endl;
        LINFO() << "Kafka connected: " + kafkaBrokers;
      }
    } catch (std::exception &e) {
      LWARN() << "Cannot initialize Kafka tracer:" << e.what();
    }
  }
#endif
}
