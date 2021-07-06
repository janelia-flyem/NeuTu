#ifndef ZDVIDWRITER_H
#define ZDVIDWRITER_H

#include <string>
#include <vector>

#include "c_stack.h"

#include "zqtheader.h"
#include "zjsonobject.h"
#include "zdvidreader.h"

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
class ZStack;
class ZObject3dScan;
class ZFlyEmBodyAnnotation;

class ZDvidWriter /*: public QObject*/
{
  /*Q_OBJECT*/
public:
  explicit ZDvidWriter(/*QObject *parent = 0*/);
  ~ZDvidWriter();

  bool open(const QString &serverAddress, const QString &uuid,
            int port = -1);
  bool open(const ZDvidTarget &target);
  bool open(const QString &sourceString);

  bool openRaw(const ZDvidTarget &target);

  void setAdmin(bool admin);

  void clear();

  const ZDvidTarget& getDvidTarget() const {
    return m_reader.getDvidTarget();
  }

  const ZDvidReader& getDvidReader() const {
    return m_reader;
  }

  ZDvidReader& getDvidReader() {
    return m_reader;
  }

  void writeSwc(uint64_t bodyId, ZSwcTree *tree);
  bool isSwcWrittable();

  void writeMesh(const ZMesh &mesh, uint64_t bodyId, int zoom);

  /*!
   * \brief Delete all related mesh data
   */
  void deleteMesh(uint64_t bodyId);

  void writeSupervoxelMesh(const ZMesh &mesh, uint64_t svId);

  void writeThumbnail(uint64_t bodyId, ZStack *stack);
  void writeThumbnail(uint64_t bodyId, Stack *stack);
  void writeAnnotation(uint64_t bodyId, const ZJsonObject &obj);
  void writeAnnotation(const ZFlyEmNeuron &neuron);

  void writeBodyAnnotation(
      uint64_t bodyId, const ZFlyEmBodyAnnotation &annotation);
  void deleteBodyAnnotation(uint64_t bodyId);

//  void removeBodyAnnotation(uint64_t bodyId);

  void writeRoiCurve(const ZClosedCurve &curve, const std::string &key);
  void deleteRoiCurve(const std::string &key);
  void writeJsonString(const std::string &dataName, const std::string &key,
                       const std::string jsonString);
  void writeJson(const std::string &dataName, const std::string &key,
                 const ZJsonValue &obj);
  void writeJson(const std::string &url, const ZJsonValue &value,
                 const std::string &emptyValueString = "");
  void writeBoundBox(const ZIntCuboid &cuboid, int z);
  void writeRoi(const ZObject3dScan &roi, const std::string &roiName);
  void writeRoiRef(
      const std::string &roiName, const std::string &key,
      const std::string &type);
  void writeRoiRef(
      const std::string &roiName, const std::vector<std::string> &keyList,
      const std::string &type);

  //void writeSplitLabel(const ZObject3dScan &obj, int label);

  void createData(
      const std::string &type, const std::string &name, bool versioned = true);
  void deleteData(const std::string &type, const std::string &name);

  void syncAnnotationToLabel(
      const std::string &name, const std::string &queryString = "");
  void syncLabelsz(const std::string &dataName,
                   const std::string &annotationName);
  void syncSynapseLabelsz();
  void createSynapseLabelsz();
  void reloadSynapseLabelsz();

  void syncData(
      const std::string &dataName, const std::string &syncDataName,
      const std::string &queryString = "");

  void writeBodyInfo(uint64_t bodyId, const ZJsonObject &obj);
  void writeBodyInfo(uint64_t bodyId);
  //void writeMaxBodyId(int bodyId);

  void mergeBody(const std::string &dataName, uint64_t targetId,
                 const std::vector<uint64_t> &bodyId);
  void mergeBody(uint64_t targetId, const std::vector<uint64_t> &bodyId);
//  void mergeBody(
//      const std::string &dataName,
//      const std::vector<uint64_t> &bodyId,
//      bool mergingToLargest);

  /*!
   * \brief Create a new keyvalue data in DVID.
   *
   * It does nothing if the data has already existed.
   */
  void createKeyvalue(const std::string &name);

  void createSplitLabel();

  void deleteKey(const char *dataName, const char *key);
  void deleteKey(const std::string &dataName, const std::string &key);
  void deleteKey(const QString &dataName, const QString &key);

  void deleteKey(const std::string &dataName,
                 const std::string &minKey, const std::string &maxKey);
  void deleteKey(const QString &dataName,
                 const QString &minKey, const QString &maxKey);

  void deleteSkeleton(uint64_t bodyId);
//  void deleteMesh(uint64_t bodyId);

//  void invalidateBody(uint64_t bodyId);

  void postLog(const std::string &message);
  bool lockNode(const std::string &message);
  std::string createBranch();

//  uint64_t rewriteBody(uint64_t label);

  //Returns (remainderId, newBodyId)
  std::pair<uint64_t, uint64_t> writeSupervoxelSplit(
      const std::string &dataName, const ZObject3dScan &obj,
      uint64_t oldLabel);
  std::pair<uint64_t, uint64_t> writeSupervoxelSplit(
      const ZObject3dScan &obj, uint64_t oldLabel);

  uint64_t writeSplit(const std::string &dataName, const ZObject3dScan &obj,
                  uint64_t oldLabel, uint64_t label, uint64_t newBodyId = 0);
  uint64_t writeSplit(const ZObject3dScan &obj,
                      uint64_t oldLabel, uint64_t label,
                      uint64_t newBodyId = 0);

  uint64_t writeSplitMultires(
      const ZObject3dScan &bf, const ZObject3dScan &bs, uint64_t oldLabel);
  uint64_t writePartition(const ZObject3dScan &bm, const ZObject3dScan &bs,
                          uint64_t oldLabel);
  uint64_t chopBody(
      const ZObject3dScan &obj, const ZIntCuboid &box, uint64_t oldLabel);

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

  bool writeLabel(const ZArray &label);
  void writeLabel(const ZArray &label, int zoom);
  void refreshLabel(const ZIntCuboid &box, uint64_t bodyId);
  void changeLabel(const ZIntCuboid &box, uint64_t oldId, uint64_t newId);
  void refreshLabel(const std::vector<ZIntCuboid> &boxArray, uint64_t bodyId);
  void changeLabel(
      const std::vector<ZIntCuboid> &boxArray, uint64_t oldId, uint64_t newId);
  void refreshLabel(const ZIntCuboid &box, uint64_t bodyId, int zoom);
  void refreshLabel(const ZIntCuboid &box, const std::set<uint64_t> &bodySet);

  void writeMasterNode(const std::string &uuid);
  void writeDefaultDataSetting(const ZJsonObject &obj);
  void writeDefaultDataSetting();

  void writeDataMap(const ZJsonObject &obj);

  inline int getStatusCode() const {
    return m_statusCode;
  }

  inline bool isStatusOk() const {
    return m_statusCode == 200;
  }

  inline const QString& getStatusErrorMessage() const {
    return m_statusErrorMessage;
  }

  inline const QString& getStandardOutput() const {
    return m_standardOutout;
  }

  inline const QString& getErrorOutput() const {
    return m_errorOutput;
  }

//  void writeUrl(const std::string &url, const std::string &method = "POST");

  bool good() const;

  void writeData(const std::string &dest, const QByteArray &data);
  void writeDataToKeyValue(
      const std::string &dataName, const std::string &key,
      const QByteArray &data);

  std::string writeServiceResult(
      const QString &group, const QByteArray &data, bool head);
  std::string writeServiceResult(
      const QString &group, const ZJsonObject &result);

  std::string writeServiceTask(
      const QString &group, const QByteArray &task, bool head);
  std::string writeServiceTask(const QString &group, const ZJsonObject &task);
  void writeSplitTask(const QString &key, const ZJsonObject &task);
  void deleteSplitTask(const QString &key);
  void writeTestResult(const std::string &key, const ZJsonObject &result);

  void writeBodyStatusList(const std::vector<std::string> &statusList);

  /*!
   * \brief Upload a mesh as a ROI
   *
   * It will create a reference for the ROI.
   *
   * \param meshPath The path of the mesh file.
   * \param name Name of the ROI
   */
  void uploadRoiMesh(const std::string &meshPath, const std::string &name);
  void uploadRoiMesh(
      const QByteArray &data, const std::string &format, const std::string &name);
//  std::string transferLocalSplitTaskToServer(const ZJsonObject &task);

public:
  //For the following functions, nothing will be done with an empty string
  //returned if the input url is empty.
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

//  bool runCommand(const QString &command, const QStringList &argList);
//  bool runCommand(const QString &command);
//  bool runCommand(QProcess &process);

  void parseStandardOutput();
  void init();
//  bool startService();

private:
//  QEventLoop *m_eventLoop;
//  ZDvidClient *m_dvidClient;
//  QTimer *m_timer;
//  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  QString m_errorOutput;
  QString m_standardOutout;
  ZJsonObject m_jsonOutput;
  int m_statusCode;
  QString m_statusErrorMessage;
  bool m_admin = false;

#if defined(_ENABLE_LIBDVIDCPP_)
  std::shared_ptr<libdvid::DVIDNodeService> m_service;
  std::shared_ptr<libdvid::DVIDConnection> m_connection;
#endif
};

#endif // ZDVIDWRITER_H
