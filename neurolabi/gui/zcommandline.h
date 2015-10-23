#ifndef ZCOMMANDLINE_H
#define ZCOMMANDLINE_H

#include <string>
#include <vector>
#include <set>

#include "zjsonobject.h"
#include "zmessagereporter.h"

class ZCommandLine
{
public:
  ZCommandLine();

  enum ECommand {
    OBJECT_MARKER, BOUNDARY_ORPHAN, OBJECT_OVERLAP,
    SYNAPSE_OBJECT, CLASS_LIST, FLYEM_NEURON_FEATURE,
    SKELETONIZE, SEPARATE_IMAGE, TRACE_NEURON, TEST_SELF,
    UNKNOWN_COMMAND
  };

  int run(int argc, char *argv[]);


private:
  static ECommand getCommand(const char *cmd);
  int runObjectMarker();
  int runBoundaryOrphan();
  int runObjectOverlap();
  int runSynapseObjectList();
  int runOutputClassList();
  int runComputeFlyEmNeuronFeature();
  int runSkeletonize();
  int runImageSeparation();
  int runTraceNeuron();
  int runTest();

  std::set<uint64_t> loadBodySet(const std::string &input);

  void loadConfig(const std::string &filePath);
  void expandConfig(const std::string &configFilePath, const std::string &key);
  std::string extractIncludePath(
      const std::string &configFilePath, const std::string &key);

private:
  std::vector<std::string> m_input;
  std::string m_output;
  std::string m_blockFile;
  std::string m_referenceBlockFile;
  std::string m_synapseFile;
  ZJsonObject m_configJson;
  std::string m_configDir;
  int m_ravelerHeight;
  int m_zStart;
  int m_intv[3];
  int m_blockOffset[3];
  bool m_fullOverlapScreen;
  bool m_isVerbose;
  bool m_forceUpdate;
  ZMessageReporter m_reporter;
};

#endif // ZCOMMANDLINE_H
