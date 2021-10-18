#ifndef ZBODYSPLITCOMMAND_H
#define ZBODYSPLITCOMMAND_H

#include "zcommandmodule.h"

#include "common/neutudefs.h"

class ZDvidReader;
class ZStack;
class ZSparseStack;
class ZStackGarbageCollector;
class ZIntCuboid;
class ZStackWatershedContainer;
class ZObject3dScanArray;
class ZDvidWriter;

class ZBodySplitCommand : public ZCommandModule
{
public:
  ZBodySplitCommand();

  int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config) override;

private:
  ZDvidReader* parseInputPath(
      const std::string &inputPath, ZJsonObject &inputJson,
      std::string &splitTaskKey, std::string &splitResultKey,
      std::string &dataDir, bool &isFile);
  static void LoadSeeds(
      const ZJsonObject &inputJson, ZStackWatershedContainer &container,
      const std::string &dataDir, bool isFile);

  std::pair<ZStack *, ZSparseStack*> parseSignalPath(
      std::string &signalPath, const ZJsonObject &signalInfo, const std::string &dataDir,
      bool isFile, const ZIntCuboid &range, ZStackGarbageCollector &gc);
  void processResult(
      ZStackWatershedContainer &container, const std::string &output,
      const std::string &splitTaskKey, const std::string &signalPath,
      bool committing, const std::string &commitPath);
  std::vector<uint64_t> commitResult(
      ZObject3dScanArray *objArray, ZDvidWriter &writer);

  std::string getLabelTypeName() const;

private:
  uint64_t m_bodyId = 0;
  neutu::EBodyLabelType m_labelType = neutu::EBodyLabelType::BODY;
};

#endif // ZBODYSPLITCOMMAND_H
