#ifndef FLYEMDATAREADER_H
#define FLYEMDATAREADER_H

#include <vector>
#include <cstdint>
#include <string>
#include <functional>
#include <unordered_map>

#include "dvid/zdviddef.h"
#include "zflyemtodoitem.h"

class ZDvidReader;
class FlyEmDataConfig;
class ZFlyEmNeuronBodyInfo;
class ZFlyEmBodyAnnotation;
class ZMesh;
class ZObject3dScan;
class ZDvidSparseStack;
class ZDvidTarget;
class ZDvidSynapse;
class ZDvidInfo;
class ZDvidVersionDag;

/*!
 * \brief The class for wrapping functions of reading flyem data.
 */
class FlyEmDataReader
{
public:
  FlyEmDataReader();

public:
  static FlyEmDataConfig ReadDataConfig(const ZDvidReader &reader);
  static ZFlyEmNeuronBodyInfo ReadBodyInfo(
      const ZDvidReader &reader, uint64_t bodyId);
  static ZFlyEmBodyAnnotation ReadBodyAnnotation(
      const ZDvidReader &reader, uint64_t bodyId);

  static std::vector<ZFlyEmToDoItem> ReadToDoItem(
      const ZDvidReader &reader, const ZIntCuboid &box);
  static ZFlyEmToDoItem ReadToDoItem(
      const ZDvidReader &reader, int x, int y, int z);
  static ZFlyEmToDoItem ReadToDoItem(
      const ZDvidReader &reader, const ZIntPoint &pos);
  static ZIntCuboid ReadTodoDataRange(const ZDvidReader &reader);

  static ZMesh* ReadRoiMesh(
      const ZDvidReader &reader, const std::string &roiName,
      std::function<void(std::string)> errorMsgHandler = nullptr);

  static bool IsSkeletonSynced(const ZDvidReader &reader, uint64_t bodyId);

  /*!
   * \brief Read one or more ROIs into a single object
   *
   * All previous content of \a result  be cleared after the function all.
   *
   * \return A new object if \a result is NULL. If \a result is not NULL,
   * it returns \a result that holds the result.
   */
  static ZObject3dScan* ReadRoi(
      const ZDvidReader &reader, const std::vector<std::string> &roiList,
      ZObject3dScan *result);

  static ZDvidSparseStack* ReadDvidSparseStack(
      const ZDvidTarget &target, ZDvidReader *grayscaleReader,
      uint64_t bodyId, neutu::EBodyLabelType labelType, bool async);

  static std::unordered_map<ZIntPoint, uint64_t> ReadSynapseLabel(
      const ZDvidReader &reader, const std::vector<ZDvidSynapse>& synapseArray);

  static std::vector<ZDvidSynapse> ReadSynapse(
      const ZDvidReader &reader, const ZIntCuboid &box,
      dvid::EAnnotationLoadMode mode = dvid::EAnnotationLoadMode::NO_PARTNER);
  static void UpdateSynapsePartner(
      const ZDvidReader &reader, ZDvidSynapse *synapse);
  static void UpdateSynapse(
      const ZDvidReader &reader, ZDvidSynapse *synapse);
  static ZDvidSynapse ReadSynapse(
      const ZDvidReader &reader, const ZIntPoint &pos);

private:
  static ZMesh* LoadRoi(
      const ZDvidReader &reader, const std::string &roiName,
      const std::string &key, const std::string &source);
  static ZMesh* LoadRoi(
      const ZDvidReader &reader, const std::string &roiName,
      const std::vector<std::string> &keyList, const std::string &source);

#if 0
  static std::vector<ZDvidSynapse> ReadSynapse(
      const ZDvidReader &reader, const ZIntCuboid &box,
      dvid::EAnnotationLoadMode mode = dvid::EAnnotationLoadMode::NO_PARTNER);
  static std::vector<ZDvidSynapse> ReadSynapse(
      const ZDvidReader &reader, uint64_t label,
      dvid::EAnnotationLoadMode mode = dvid::EAnnotationLoadMode::NO_PARTNER);
  static std::vector<ZDvidSynapse> ReadSynapse(
      const ZDvidReader &reader, uint64_t label, const ZDvidRoi &roi,
      dvid::EAnnotationLoadMode mode);

  static ZDvidSynapse ReadSynapse(
      const ZDvidReader &reader, int x, int y, int z,
      dvid::EAnnotationLoadMode mode = dvid::EAnnotationLoadMode::NO_PARTNER);
  static ZDvidSynapse ReadSynapse(
      const ZDvidReader &reader, const ZIntPoint &pt,
      dvid::EAnnotationLoadMode mode = dvid::EAnnotationLoadMode::NO_PARTNER);
#endif
};

#endif // FLYEMDATAREADER_H
