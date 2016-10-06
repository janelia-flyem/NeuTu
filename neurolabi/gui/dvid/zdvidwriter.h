#ifndef ZDVIDWRITER_H
#define ZDVIDWRITER_H

#include <QObject>
#include <QEventLoop>
#include <QTimer>
#include <QMap>
#include <QString>

#include <string>
#include <vector>
#include "zobject3dscan.h"
#include "zstack.hxx"
#include "zdvidclient.h"
#include "flyem/zflyem.h"
#include "zintcuboid.h"
#include "zsparsestack.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidwriter.h"
#include "zflyembodyannotation.h"
#include "zjsonobject.h"
#include "zsharedpointer.h"


namespace libdvid{
class DVIDNodeService;
}

class ZFlyEmNeuron;
class ZClosedCurve;
class ZIntCuboid;
class ZSwcTree;
class QProcess;
class ZFlyEmBookmark;
class ZDvidSynapse;
class ZFlyEmToDoItem;
class ZArray;

class ZDvidWriter : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidWriter(QObject *parent = 0);
  ~ZDvidWriter();

  bool open(const QString &serverAddress, const QString &uuid,
            int port = -1);
  bool open(const ZDvidTarget &target);
  bool open(const QString &sourceString);

  void clear();

  const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  void writeSwc(uint64_t bodyId, ZSwcTree *tree);
  void writeThumbnail(uint64_t bodyId, ZStack *stack);
  void writeThumbnail(uint64_t bodyId, Stack *stack);
  void writeAnnotation(uint64_t bodyId, const ZJsonObject &obj);
  void writeAnnotation(const ZFlyEmNeuron &neuron);

  void writeBodyAnntation(const ZFlyEmBodyAnnotation &annotation);
  void removeBodyAnnotation(uint64_t bodyId);


  void writeRoiCurve(const ZClosedCurve &curve, const std::string &key);
  void deleteRoiCurve(const std::string &key);
  void writeJsonString(const std::string &dataName, const std::string &key,
                       const std::string jsonString);
  void writeJson(const std::string &dataName, const std::string &key,
                 const ZJsonValue &obj);
  void writeJson(const std::string &url, const ZJsonValue &value,
                 const std::string &emptyValueString = "");
  void writeBoundBox(const ZIntCuboid &cuboid, int z);

  //void writeSplitLabel(const ZObject3dScan &obj, int label);

  void createData(
      const std::string &type, const std::string &name, bool versioned = true);

  void syncAnnotation(const std::string &name);
  void syncLabelsz(const std::string &dataName,
                   const std::string &annotationName);
  void syncSynapseLabelsz();
  void createSynapseLabelsz();

  void writeBodyInfo(uint64_t bodyId, const ZJsonObject &obj);
  void writeBodyInfo(uint64_t bodyId);
  //void writeMaxBodyId(int bodyId);

  void mergeBody(const std::string &dataName, uint64_t targetId,
                 const std::vector<uint64_t> &bodyId);

  /*!
   * \brief Create a new keyvalue data in DVID.
   *
   * It does nothing if the data has already existed.
   */
  void createKeyvalue(const std::string &name);

  void deleteKey(const char *dataName, const char *key);
  void deleteKey(const std::string &dataName, const std::string &key);
  void deleteKey(const QString &dataName, const QString &key);

  void deleteKey(const std::string &dataName,
                 const std::string &minKey, const std::string &maxKey);
  void deleteKey(const QString &dataName,
                 const QString &minKey, const QString &maxKey);

  void deleteSkeleton(uint64_t bodyId);
  void deleteBodyAnnotation(uint64_t bodyId);

  void invalidateBody(uint64_t bodyId);

  void postLog(const std::string &message);
  bool lockNode(const std::string &message);
  std::string createBranch();

  uint64_t writeSplit(const std::string &dataName, const ZObject3dScan &obj,
                  uint64_t oldLabel, uint64_t label, uint64_t newBodyId = 0);
  uint64_t writeSplit(const ZObject3dScan &obj,
                      uint64_t oldLabel, uint64_t label,
                      uint64_t newBodyId = 0);

  uint64_t writeSplitMultires(
      const ZObject3dScan &bf, const ZObject3dScan &bs, uint64_t oldLabel);
  uint64_t writePartition(const ZObject3dScan &bm, const ZObject3dScan &bs,
                          uint64_t oldLabel);

  uint64_t writeCoarseSplit(const ZObject3dScan &obj, uint64_t oldLabel);

  void writeMergeOperation(const QMap<uint64_t, uint64_t> &bodyMap);
  /*
  void writeMergeOperation(const std::string &dataName, const std::string &key,
                           const QMap<uint64_t, uint64_t> &bodyMap);
                           */

  void writePointAnnotation(
      const std::string &dataName, const ZJsonObject &annotationJson);
  void writePointAnnotation(
      const std::string &dataName, const ZJsonArray &annotationJson);
  void deletePointAnnotation(const std::string &dataName, int x, int y, int z);
  void deletePointAnnotation(const std::string &dataName, const ZIntPoint &pt);

  //For old bookmark management
  void writeBookmark(const ZFlyEmBookmark &bookmark);
  void writeBookmark(const ZJsonObject &bookmarkJson);
  void writeBookmark(const ZJsonArray &bookmarkJson);
  void writeBookmark(const std::vector<ZFlyEmBookmark*> &bookmarkArray);
  void writeBookmarkKey(const ZFlyEmBookmark &bookmark);
  void deleteBookmarkKey(const ZFlyEmBookmark &bookmark);

  void deleteBookmark(int x, int y, int z);
  void deleteBookmark(const ZIntPoint &pt);
  void deleteBookmark(const std::vector<ZFlyEmBookmark*> &bookmarkArray);

  /*
  void writeCustomBookmark(const ZJsonValue &bookmarkJson);
  void deleteAllCustomBookmark();
  */

  void deleteSynapse(int x, int y, int z);
  void writeSynapse(const ZDvidSynapse &synapse);
  void moveSynapse(const ZIntPoint &from, const ZIntPoint &to);
  void writeSynapse(const ZJsonObject &synapseJson);
  void writeSynapse(const ZJsonArray &synapseJson);
  void linkSynapse(const ZIntPoint &v1, const ZIntPoint &v2);
  void addSynapseProperty(const ZIntPoint &synapse,
                          const std::string &key, const std::string &value);

  void deleteToDoItem(int x, int y, int z);
  void writeToDoItem(const ZFlyEmToDoItem &item);

  void writeLabel(const ZArray &label);
  void refreshLabel(const ZIntCuboid &box, uint64_t bodyId);
  void refreshLabel(const ZIntCuboid &box, const std::set<uint64_t> &bodySet);

  void writeMasterNode(const std::string &uuid);

  inline int getStatusCode() const {
    return m_statusCode;
  }

  inline bool isStatusOk() const {
    return m_statusCode == 200;
  }

  inline const QString& getStandardOutput() const {
    return m_standardOutout;
  }

  inline const QString& getErrorOutput() const {
    return m_errorOutput;
  }

  void writeUrl(const std::string &url, const std::string &method = "POST");

  bool good() const;

public:
  std::string post(const std::string &url);
  std::string post(const std::string &url, const QByteArray &payload, bool isJson);
  std::string post(const std::string &url, const std::string &payload, bool isJson);
  std::string post(const std::string &url, const char *payload, int length,
                   bool isJson);
  std::string post(const std::string &url, const ZJsonObject &payload);
  std::string post(const std::string &url, const ZJsonArray &payload);
  std::string del(const std::string &url);

  std::string put(
      const std::string &url, const char *payload, int length, bool isJson);
  std::string put(const std::string &url);

  std::string request(const std::string &url, const std::string &method,
                      const char *payload, int length, bool isJson);

private:
  std::string getJsonStringForCurl(const ZJsonValue &obj) const;
//  void writeJson(const std::string url, const ZJsonValue &value);
  void writeJsonString(const std::string &url, const std::string &jsonString);

  ZJsonValue getLocMessage(const std::string &message);

  bool runCommand(const QString &command, const QStringList &argList);
  bool runCommand(const QString &command);
  bool runCommand(QProcess &process);

  void parseStandardOutput();
  void init();
  bool startService();

private:
//  QEventLoop *m_eventLoop;
//  ZDvidClient *m_dvidClient;
//  QTimer *m_timer;
  ZDvidTarget m_dvidTarget;
  QString m_errorOutput;
  QString m_standardOutout;
  ZJsonObject m_jsonOutput;
  int m_statusCode;

#if defined(_ENABLE_LIBDVIDCPP_)
  ZSharedPointer<libdvid::DVIDNodeService> m_service;
#endif
};

#endif // ZDVIDWRITER_H
