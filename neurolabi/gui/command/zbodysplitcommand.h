#ifndef ZBODYSPLITCOMMAND_H
#define ZBODYSPLITCOMMAND_H

#include "zcommandmodule.h"

class ZJsonObject;
class ZDvidReader;
class ZStack;
class ZSparseStack;
class ZStackGarbageCollector;
class ZIntCuboid;
class ZStackWatershedContainer;

class ZBodySplitCommand : public ZCommandModule
{
public:
  ZBodySplitCommand();

  int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config);

private:
  static ZDvidReader* ParseInputPath(const std::string inputPath, ZJsonObject &inputJson,
      std::string &splitTaskKey, std::string &splitResultKey, std::string &dataDir, bool &isFile);
  static std::pair<ZStack *, ZSparseStack*> ParseSignalPath(
      std::string &signalPath, const std::string &dataDir,
      bool isFile, const ZIntCuboid &range, ZStackGarbageCollector &gc);
  static void LoadSeeds(
      const ZJsonObject &inputJson, ZStackWatershedContainer &container,
      const std::string &dataDir, bool isFile);
  static void ProcessResult(ZStackWatershedContainer &container, const std::string &output,
      const std::string &splitTaskKey);
};

#endif // ZBODYSPLITCOMMAND_H
