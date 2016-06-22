#ifndef ZNEURONTRACER_H
#define ZNEURONTRACER_H

#include <map>

#include "zswcpath.h"
#include "tz_trace_defs.h"
#include "tz_trace_utils.h"
#include "neutube_def.h"
#include "zstackgraph.h"
#include "tz_locseg_chain.h"
#include "zprogressable.h"
#include "zintpoint.h"

class ZStack;
class ZSwcTree;
class ZSwcConnector;
class ZJsonObject;
//class ZIntPoint;

class ZNeuronTraceSeeder {
public:
  ZNeuronTraceSeeder();
  ~ZNeuronTraceSeeder();

  Stack *sortSeed(Geo3d_Scalar_Field *seedPointArray, const Stack *signal,
                Trace_Workspace *ws);

  inline std::vector<Local_Neuroseg>& getSeedArray() { return m_seedArray; }
  inline std::vector<double>& getScoreArray() { return m_seedScoreArray; }

private:
  std::vector<Local_Neuroseg> m_seedArray;
  std::vector<double> m_seedScoreArray;
};

class ZNeuronConstructor {
public:
  ZNeuronConstructor();

  inline void setWorkspace(Connection_Test_Workspace *ws) {
    m_connWorkspace = ws;
  }

  inline void setSignal(Stack *signal) {
    m_signal = signal;
  }

  ZSwcTree *reconstruct(std::vector<Locseg_Chain*> &chainArray);

private:
  Connection_Test_Workspace *m_connWorkspace;
  Stack *m_signal;
};



class ZNeuronTracer : public ZProgressable
{
public:
  ZNeuronTracer();
  ~ZNeuronTracer();

public:
  ZSwcPath trace(double x, double y, double z);
  void updateMask(const ZSwcPath &branch);
  void setIntensityField(ZStack *stack);
  Stack* getIntensityData() const;
  inline const ZStack *getStack() const { return m_stack; }
  inline ZStack *getStack() { return m_stack; }
  void updateMask(Swc_Tree *tree);
  void setTraceWorkspace(Trace_Workspace *workspace);
  void setConnWorkspace(Connection_Test_Workspace *workspace);

  Swc_Tree* trace(double x1, double y1, double z1, double r1,
                 double x2, double y2, double z2, double r2);

  void setTraceRange(const ZIntCuboid &box);

  void clear();

  inline void setBackgroundType(NeuTube::EImageBackground bg) {
    m_backgroundType = bg;
  }

  inline void setResolution(double x, double y, double z) {
    m_resolution[0] = x;
    m_resolution[1] = y;
    m_resolution[2] = z;
  }

#if 0
  inline void setStackOffset(double x, double y, double z) {
    m_stackOffset[0] = x;
    m_stackOffset[1] = y;
    m_stackOffset[2] = z;
  }

  void setStackOffset(const ZIntPoint &pt);
#endif

  inline void setVertexOption(ZStackGraph::EVertexOption vertexOption) {
    m_vertexOption = vertexOption;
  }

  void setSignalChannel(int c) { m_preferredSignalChannel = c; }
  int getSignalChannel() const { return m_preferredSignalChannel; }

  /*!
   * \brief Auto trace
   *
   * It will also create workspaces automatically if necessary.
   */
  ZSwcTree* trace(Stack *stack, bool doResampleAfterTracing = true);

  ZSwcTree* trace(const ZStack *stack, bool doResampleAfterTracing = true);

  //Autotrace configuration
  //Trace level setup: 1 - 10 (fast -> accurate)
  void setTraceLevel(int level);


  //Helper functions
  static double findBestTerminalBreak(
      const ZPoint &terminalCenter, double terminalRadius,
      const ZPoint &innerCenter, double innerRadius,
      const Stack *stack);

  inline Trace_Workspace* getTraceWorkspace() const {
    return m_traceWorkspace;
  }

  inline Connection_Test_Workspace* getConnectionTestWorkspace() const {
    return m_connWorkspace;
  }

  void initTraceMask();
  void initTraceWorkspace(Stack *stack);
  void initTraceWorkspace(ZStack *stack);
  void initConnectionTestWorkspace();

  void updateTraceWorkspace(int traceEffort, bool traceMasked,
                            double xRes, double yRes, double zRes);
  void updateConnectionTestWorkspace(
      double xRes, double yRes, double zRes,
      char unit, double distThre, bool spTest, bool crossoverTest);

  void loadTraceMask(bool traceMasked);


  void loadJsonObject(const ZJsonObject &obj);


  enum ETracingMode {
    TRACING_AUTO, TRACING_INTERACTIVE, TRACING_SEED
  };
  void prepareTraceScoreThreshold(ETracingMode mode);
  void setMinScore(double score, ETracingMode mode);

  void useEdgePath(bool state) {
    m_usingEdgePath = state;
  }

  void setBcAdjust(bool on) { m_bcAdjust = on; }
  void setGreyFactor(double v) { m_greyFactor = v; }
  void setGrayOffset(double v) { m_greyOffset = v; }

  int getRecoverLevel() const;
  void setRecoverLevel(int level);

  Stack* computeSeedMask();
  Stack* computeSeedMask(Stack *stack);

private:
  //Helper functions
  Stack* binarize(const Stack *stack);
  Stack* bwsolid(Stack *stack);
  Stack* enhanceLine(const Stack *stack);
  Geo3d_Scalar_Field* extractSeed(const Stack *mask);
  Geo3d_Scalar_Field* extractSeedOriginal(const Stack *mask);
  Geo3d_Scalar_Field* extractSeedSkel(const Stack *mask);

  Geo3d_Scalar_Field* extractLineSeed(
      const Stack *mask, const Stack *dist, int minObjSize = 0);

  ZSwcTree *reconstructSwc(const Stack *stack,
                           std::vector<Locseg_Chain*> &chainArray);
  std::vector<Locseg_Chain*> trace(const Stack *stack,
                                   std::vector<Local_Neuroseg> &locsegArray,
                                   std::vector<double> &values);
  std::vector<Locseg_Chain*> recover(const Stack *stack);

  std::vector<Locseg_Chain*> screenChain(const Stack *stack,
                                         std::vector<Locseg_Chain*> &chainArray);

  void clearBuffer();

  void init();
  void config();


private:
  ZStack *m_stack;
  Trace_Workspace *m_traceWorkspace;
  Connection_Test_Workspace *m_connWorkspace;
  ZSwcConnector *m_swcConnector;
  NeuTube::EImageBackground m_backgroundType;
  ZStackGraph::EVertexOption m_vertexOption;
  double m_resolution[3];
//  double m_stackOffset[3];

  double m_seedMinScore;
  double m_autoTraceMinScore;
  double m_traceMinScore;
  double m_2dTraceMinScore;
  bool m_usingEdgePath;
  bool m_enhancingMask;

  int m_recover;
  int m_seedingMethod;
  int m_preferredSignalChannel;

  //Intermedite buffer
  std::vector<Locseg_Chain*> m_chainArray;
  Stack *m_mask;
  Stack *m_baseMask;
  ZIntPoint m_seedDsIntv;

  bool m_bcAdjust;
  double m_greyFactor;
  double m_greyOffset;

  /*
  static const char *m_levelKey;
  static const char *m_minimalScoreKey;
  static const char *m_minimalSeedScoreKey;
  static const char *m_spTestKey;
  static const char *m_enhanceLineKey;
  */
};

#endif // ZNEURONTRACER_H
