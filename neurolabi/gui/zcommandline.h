#ifndef ZCOMMANDLINE_H
#define ZCOMMANDLINE_H

#include <string>
#include <vector>
#include <set>

class ZCommandLine
{
public:
  ZCommandLine();

  enum ECommand {
    OBJECT_MARKER, BOUNDARY_ORPHAN, OBJECT_OVERLAP,
    SYNAPSE_OBJECT, CLASS_LIST, FLYEM_NEURON_FEATURE,
    SKELETONIZE, SEPARATE_IMAGE,
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

  std::set<int> loadBodySet(const std::string &input);

private:
  std::vector<std::string> m_input;
  std::string m_output;
  std::string m_blockFile;
  std::string m_referenceBlockFile;
  std::string m_synapseFile;
  int m_ravelerHeight;
  int m_zStart;
  int m_intv[3];
  int m_blockOffset[3];
  bool m_fullOverlapScreen;
};

#endif // ZCOMMANDLINE_H
