#include "zneurontracer.h"
//#include "zlocsegchain.h"
#include "swctreenode.h"
#include "c_stack.h"
#include "zswcconnector.h"
#include "tz_math.h"
#include "zvoxelarray.h"
#include "tz_stack_sampling.h"
#include "zstackbinarizer.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_math.h"
#include "tz_fimage_lib.h"
#include "tz_voxel_graphics.h"
#include "zstack.hxx"
#include "swc/zswcresampler.h"
#include "zintpoint.h"
#include "neutubeconfig.h"
#include "zstackprocessor.h"
#include "zobject3darray.h"
#include "tz_objdetect.h"
#include "zjsonobject.h"
#include "zswctree.h"
#include "swc/zswcsignalfitter.h"
#include "zneurontracerconfig.h"
#include "swc/zswcpruner.h"

ZNeuronTraceSeeder::ZNeuronTraceSeeder()
{
}

ZNeuronTraceSeeder::~ZNeuronTraceSeeder()
{
}

Stack* ZNeuronTraceSeeder::sortSeed(
    Geo3d_Scalar_Field *seedPointArray, const Stack *signal, Trace_Workspace *ws)
{
  Locseg_Fit_Workspace *fws = (Locseg_Fit_Workspace *) ws->fit_workspace;
  fws->sws->fs.n = 2;
  fws->sws->fs.options[0] = STACK_FIT_DOT;
  fws->sws->fs.options[1] = STACK_FIT_CORRCOEF;
  fws->pos_adjust = 1;

  m_seedArray.resize(seedPointArray->size);
  m_seedScoreArray.resize(seedPointArray->size);

  /* <seed_mask> allocated */
  Stack *seed_mask = C_Stack::make(GREY, signal->width, signal->height,
                                   signal->depth);
  Zero_Stack(seed_mask);

  for (int i = 0; i < seedPointArray->size; i++) {
    printf("-----------------------------> seed: %d / %d\n", i,
           seedPointArray->size);

    int index = i;
    int x = (int) seedPointArray->points[index][0];
    int y = (int) seedPointArray->points[index][1];
    int z = (int) seedPointArray->points[index][2];

    if (ws->trace_mask != NULL) {
      int v = C_Stack::value(ws->trace_mask, x, y, z);
      if (v > 0) {
        m_seedScoreArray[i] = 0;
        continue;
      }
    }

    double width = seedPointArray->values[index];

    ssize_t seed_offset = C_Stack::offset(x, y, z, signal->width, signal->height,
                                          signal->depth);

    if (width < 3.0) {
      width += 0.5;
    }
    Set_Neuroseg(&(m_seedArray[i].seg), width, 0.0, NEUROSEG_DEFAULT_H,
                 0.0, 0.0, 0.0, 0.0, 1.0);

    double cpos[3];
    cpos[0] = x;
    cpos[1] = y;
    cpos[2] = z;
    //cpos[2] /= z_scale;

    Set_Neuroseg_Position(&(m_seedArray[i]), cpos, NEUROSEG_CENTER);

    if (seed_mask->array[seed_offset] > 0) {
      printf("labeled\n");
      m_seedScoreArray[i] = 0.0;
      continue;
    }

    //Local_Neuroseg_Optimize(locseg + i, signal, z_scale, 0);
    double z_scale = 1.0;
    Local_Neuroseg_Optimize_W(&(m_seedArray[i]), signal, z_scale, 0, fws);

    if (ws->trace_mask != NULL) {
      Local_Neuroseg &seg = m_seedArray[i];
      int v = C_Stack::value(
            ws->trace_mask,
            iround(seg.pos[0]), iround(seg.pos[1]), iround(seg.pos[2]));
      if (v > 0) {
        m_seedScoreArray[i] = 0;
        continue;
      }
    }

    m_seedScoreArray[i] = fws->sws->fs.scores[1];

    double min_score = ws->min_score;

    if (m_seedScoreArray[i] > min_score) {
      Local_Neuroseg_Label_G(&(m_seedArray[i]), seed_mask, -1, 2, z_scale);
    } else {
      Local_Neuroseg_Label_G(&(m_seedArray[i]), seed_mask, -1, 1, z_scale);
    }
  }

  /* <seed_mask> freed */
//  C_Stack::kill(seed_mask);

  return seed_mask;
}

ZNeuronConstructor::ZNeuronConstructor() : m_connWorkspace(NULL), m_signal(NULL)
{

}


ZSwcTree *ZNeuronConstructor::reconstruct(
    std::vector<Locseg_Chain*> &chainArray)
{
  ZSwcTree *tree = NULL;

  if (!chainArray.empty()) {
    int chain_number = chainArray.size();
    /* <neuronComponent> allocated */
    Neuron_Component *neuronComponent =
        Make_Neuron_Component_Array(chain_number);

    for (int i = 0; i < chain_number; i++) {
      Set_Neuron_Component(neuronComponent + i,
                           NEUROCOMP_TYPE_LOCSEG_CHAIN,
                           chainArray[i]);
    }

    /* reconstruct neuron */
    /* alloc <ns> */
    double zscale = 1.0;
    Neuron_Structure *ns = Locseg_Chain_Comp_Neurostruct(
          neuronComponent, chain_number, m_signal, zscale, m_connWorkspace);

    Process_Neuron_Structure(ns);

    if (m_connWorkspace->crossover_test == TRUE) {
      Neuron_Structure_Crossover_Test(ns, zscale);
    }

    /* alloc <ns2> */
    Neuron_Structure* ns2=
        Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);

    Neuron_Structure_To_Tree(ns2);

    tree = new ZSwcTree;
    tree->setData(Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, NULL));
    tree->resortId();

    /* free <ns2> */
    Kill_Neuron_Structure(ns2);
    /* free <ns> */
    ns->comp = NULL;
    Kill_Neuron_Structure(ns);

    /* free <neuronComponent> */
    Clean_Neuron_Component_Array(neuronComponent, chain_number);
    free(neuronComponent);
  }

  return tree;
}



//////////////////////////ZNeuronTracer///////////////////////////////

ZNeuronTracer::ZNeuronTracer() : m_stack(NULL), m_traceWorkspace(NULL),
  m_connWorkspace(NULL), m_swcConnector(NULL),
  m_backgroundType(NeuTube::IMAGE_BACKGROUND_DARK),
  m_vertexOption(ZStackGraph::VO_ALL)
{
  init();
}

ZNeuronTracer::~ZNeuronTracer()
{
  clear();
}

void ZNeuronTracer::init()
{
  m_swcConnector = new ZSwcConnector;
  for (int i = 0; i < 3; ++i) {
    m_resolution[i] = 1.0;
//    m_stackOffset[i] = 0.0;
  }

  m_mask = NULL;
  m_baseMask = NULL;

  m_bcAdjust = false;
  m_greyFactor = 1.0;
  m_greyOffset = 0.0;
  m_preferredSignalChannel = 0;

  config();
}

void ZNeuronTracer::config()
{
  ZNeuronTracerConfig &config = ZNeuronTracerConfig::getInstance();

  m_seedMinScore = config.getMinSeedScore();
  m_autoTraceMinScore = config.getMinAutoScore();
  m_traceMinScore = config.getMinManualScore();
  m_2dTraceMinScore = config.getMin2dScore();
  m_usingEdgePath = config.usingEdgePath();

  m_enhancingMask = config.enhancingMask();
  m_seedingMethod = config.getSeedMethod();
  m_recover = config.getRecoverLevel();

  if (m_traceWorkspace != NULL) {
    m_traceWorkspace->refit = config.isRefit();
    m_traceWorkspace->tune_end = config.tuningEnd();
  }

  if (m_connWorkspace != NULL) {
    m_connWorkspace->sp_test = config.spTest();
    m_connWorkspace->crossover_test = config.crossoverTest();
    m_connWorkspace->dist_thre = config.getMaxEucDist();
  }
}

void ZNeuronTracer::clear()
{
  if (m_traceWorkspace != NULL) {
    if (m_traceWorkspace->fit_workspace != NULL) {
      Locseg_Fit_Workspace *fw =
          (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
      fw->sws->mask = NULL;
      Kill_Locseg_Fit_Workspace(fw);
      m_traceWorkspace->fit_workspace = NULL;
    }
    Kill_Trace_Workspace(m_traceWorkspace);
    m_traceWorkspace = NULL;
  }

  if (m_connWorkspace != NULL) {
    Kill_Connection_Test_Workspace(m_connWorkspace);
    m_connWorkspace = NULL;
  }

  delete m_swcConnector;
  m_swcConnector = NULL;
  m_chainArray.clear();
  m_stack = NULL;

  clearBuffer();
}

Stack* ZNeuronTracer::getIntensityData() const
{
  Stack *stack = NULL;

  if (m_stack != NULL) {
    if (m_preferredSignalChannel < m_stack->channelNumber()) {
      stack = m_stack->c_stack(m_preferredSignalChannel);
    } else {
      stack = m_stack->c_stack();
    }
  }

  return stack;
}

void ZNeuronTracer::setIntensityField(ZStack *stack)
{
  m_stack = stack;
}

void ZNeuronTracer::setTraceRange(const ZIntCuboid &box)
{
  if (m_traceWorkspace != NULL) {
    ZIntPoint stackOffset;
    if (getStack() != NULL) {
      stackOffset = getStack()->getOffset();
    }

    m_traceWorkspace->trace_range[0] =
        box.getFirstCorner().getX() - stackOffset.getX();
    m_traceWorkspace->trace_range[3] =
        box.getLastCorner().getX() - stackOffset.getX();
    m_traceWorkspace->trace_range[1] =
        box.getFirstCorner().getY() - stackOffset.getY();
    m_traceWorkspace->trace_range[4] =
        box.getLastCorner().getY() - stackOffset.getY();
    m_traceWorkspace->trace_range[2] =
        box.getFirstCorner().getZ() - stackOffset.getZ();
    m_traceWorkspace->trace_range[5] =
        box.getLastCorner().getZ() - stackOffset.getZ();
  }
}

void ZNeuronTracer::initTraceMask()
{
  if (m_traceWorkspace->trace_mask == NULL) {
    m_traceWorkspace->trace_mask =
        C_Stack::make(GREY16, getStack()->width(), getStack()->height(),
                      getStack()->depth());
  }
  Zero_Stack(m_traceWorkspace->trace_mask);
}

ZSwcPath ZNeuronTracer::trace(double x, double y, double z)
{
  prepareTraceScoreThreshold(TRACING_INTERACTIVE);

  initTraceMask();

  Stack *stackData = getIntensityData();

  ZIntPoint stackOffset = getStack()->getOffset();

  double pos[3];
  pos[0] = x - stackOffset.getX();
  pos[1] = y - stackOffset.getY();
  pos[2] = z - stackOffset.getZ();

  /* alloc <locseg> */
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Neuroseg(&(locseg->seg), 3.0, 0.0, 11.0, TZ_PI_4, 0.0, 0.0, 0.0, 1.0);

  Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

  Locseg_Fit_Workspace *ws =
      (Locseg_Fit_Workspace*) m_traceWorkspace->fit_workspace;
  Local_Neuroseg_Optimize_W(locseg, stackData, 1.0, 1, ws);

  Trace_Record *tr = New_Trace_Record();
  tr->mask = ZERO_BIT_MASK;
  Trace_Record_Set_Fix_Point(tr, 0.0);
  Trace_Record_Set_Direction(tr, DL_BOTHDIR);
  /* consume <locseg> */
  Locseg_Node *p = Make_Locseg_Node(locseg, tr);

  /* alloc <locseg_chain> */
  Locseg_Chain *locseg_chain = Make_Locseg_Chain(p);

  Trace_Workspace_Set_Trace_Status(m_traceWorkspace, TRACE_NORMAL,
                                   TRACE_NORMAL);
  Trace_Locseg(stackData, 1.0, locseg_chain, m_traceWorkspace);
  Locseg_Chain_Remove_Overlap_Ends(locseg_chain);
  Locseg_Chain_Remove_Turn_Ends(locseg_chain, 1.0);

  int n;
  /* alloc <circles> */
  Geo3d_Circle *circles =
      Locseg_Chain_To_Geo3d_Circle_Array(locseg_chain, NULL, &n);

  /* free <locseg_chain> */
  Kill_Locseg_Chain(locseg_chain);

  ZSwcPath path;
  if (n > 0) {
//    bool hit = false;
    int start = 0;
    int end = n;
    if (Trace_Workspace_Mask_Value(m_traceWorkspace, circles[0].center) > 0) {
      for (int i = 1; i < n; ++i) {
        start = i - 1;
        if (Trace_Workspace_Mask_Value(m_traceWorkspace, circles[i].center) == 0) {
          break;
        }
      }
    }

    if (n > 1) {
      if (Trace_Workspace_Mask_Value(m_traceWorkspace, circles[n - 1].center) > 0) {
        for (int i = n - 2; i >= 0; --i) {
          end = i + 2;
          if (Trace_Workspace_Mask_Value(m_traceWorkspace, circles[i].center) == 0) {
            break;
          }
        }
      }
    }

    for (int i = start; i < end; ++i) {
      Swc_Tree_Node *tn = SwcTreeNode::makePointer(circles[i].center[0],
          circles[i].center[1], circles[i].center[2], circles[i].radius);
      if (!path.empty()) {
        SwcTreeNode::setParent(tn, path.back());
      }
      SwcTreeNode::translate(tn, stackOffset);
      path.push_back(tn);
    }
  }

  /* free <circles> */
  if (circles != NULL) {
    free(circles);
  }

  return path;
}

void ZNeuronTracer::updateMask(const ZSwcPath &branch)
{
  Swc_Tree_Node_Label_Workspace workspace;
  Default_Swc_Tree_Node_Label_Workspace(&workspace);
  for (ZSwcPath::const_iterator iter = branch.begin(); iter != branch.end();
       ++iter) {
    Swc_Tree_Node_Label_Stack(*iter, m_traceWorkspace->trace_mask, &workspace);
  }
}

void ZNeuronTracer::updateMask(Swc_Tree *tree)
{
  Swc_Tree_Node_Label_Workspace workspace;
  Default_Swc_Tree_Node_Label_Workspace(&workspace);
  Swc_Tree_Label_Stack(tree, m_traceWorkspace->trace_mask, &workspace);
}

void ZNeuronTracer::setTraceWorkspace(Trace_Workspace *workspace)
{
  m_traceWorkspace = workspace;
}

void ZNeuronTracer::setConnWorkspace(Connection_Test_Workspace *workspace)
{
  m_connWorkspace = workspace;
}

#define MAX_P2P_TRACE_DISTANCE 100
#define MAX_P2P_TRACE_VOLUME 1000000

Swc_Tree* ZNeuronTracer::trace(double x1, double y1, double z1, double r1,
                               double x2, double y2, double z2, double r2)
{
  prepareTraceScoreThreshold(TRACING_INTERACTIVE);

  ZIntPoint stackOffset = getStack()->getOffset();

  ZPoint targetPos(x2, y2, z2);

  x1 = iround(x1);
  y1 = iround(y1);
  z1 = iround(z1);
  x2 = iround(x2);
  y2 = iround(y2);
  z2 = iround(z2);

  x1 -= stackOffset.getX();
  y1 -= stackOffset.getY();
  z1 -= stackOffset.getZ();

  x2 -= stackOffset.getX();
  y2 -= stackOffset.getY();
  z2 -= stackOffset.getZ();

  if (x1 < 0 || y1 < 0 || z1 < 0 || x1 >= getStack()->width() ||
      y1 >= getStack()->height() || z1 >= getStack()->depth()) {
    return NULL;
  }

  ZStackGraph stackGraph;
  if (m_resolution[2] / m_resolution[0] > 3.0) {
    stackGraph.setZMargin(2);
  }
  stackGraph.updateRange(
        x1, y1, z1, x2, y2, z2,
        getStack()->width(), getStack()->height(), getStack()->depth());
  if (stackGraph.getRoiVolume() > MAX_P2P_TRACE_VOLUME) {
    return NULL;
  }

  stackGraph.setResolution(m_resolution);

  if (m_vertexOption == ZStackGraph::VO_SURFACE) {
    stackGraph.setWeightFunction(Stack_Voxel_Weight_I);
  } else {
    if (m_usingEdgePath) {
      stackGraph.setWeightFunction(Stack_Voxel_Weight_S);
    } else {
      if (m_backgroundType == NeuTube::IMAGE_BACKGROUND_BRIGHT) {
        stackGraph.setWeightFunction(Stack_Voxel_Weight_Sr);
      } else {
        stackGraph.setWeightFunction(Stack_Voxel_Weight_S);
      }
    }
  }

  ZIntCuboid box = stackGraph.getRange();
//  if (m_usingEdgePath) {
//    box.setFirstCorner(imin2(x1, x2), imin2(y1, y2), imin2(z1, z2));
//    box.setLastCorner(imax2(x1, x2), imax2(y1, y2), imax2(z1, z2));
//  }

  Stack *partial = C_Stack::crop(
        getIntensityData(), box.getFirstCorner().getX(), box.getFirstCorner().getY(),
        box.getFirstCorner().getZ(), box.getWidth(), box.getHeight(),
        box.getDepth(), NULL);

  /*
  if (m_bcAdjust) {
    Stack_Scale(partial, 0, m_greyFactor, m_greyOffset);
  }
  */

  if (m_usingEdgePath) {
    Stack *partialEdge = C_Stack::computeGradient(partial);
    C_Stack::kill(partial);
    partial = partialEdge;

#ifdef _DEBUG_2
    C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", partial);
#endif
  }

  stackGraph.inferWeightParameter(partial);

  ZVoxelArray voxelArray;
  std::vector<int> path;

  if (m_usingEdgePath) {
    int x0 = box.getFirstCorner().getX();
    int y0 = box.getFirstCorner().getY();
    int z0 = box.getFirstCorner().getZ();

    int startIndex = C_Stack::indexFromCoord(
          x1 - x0, y1 - y0 , z1 - z0, C_Stack::width(partial),
          C_Stack::height(partial),
          C_Stack::depth(partial));
    int endIndex = C_Stack::indexFromCoord(
          x2 - x0, y2 - y0, z2 - z0, C_Stack::width(partial),
          C_Stack::height(partial),
          C_Stack::depth(partial));

    stackGraph.setRange(0, 0, 0, C_Stack::width(partial) - 1,
                        C_Stack::height(partial) - 1,
                        C_Stack::depth(partial) - 1);
    path = stackGraph.computeShortestPath(
          partial, startIndex, endIndex, m_vertexOption);


    for (size_t i = path.size(); i > 0; --i) {
      int x, y, z;
      C_Stack::indexToCoord(path[i - 1], C_Stack::width(partial),
          C_Stack::height(partial), &x, &y, &z);
      voxelArray.append(ZVoxel(x + x0, y + y0, z + z0));
    }

  } else {
    int width = getStack()->width();
    int height = getStack()->height();
    int depth = getStack()->depth();

    int startIndex = C_Stack::indexFromCoord(
          x1, y1, z1, width, height, depth);
    int endIndex = C_Stack::indexFromCoord(
          x2, y2, z2, width, height, depth);

    path = stackGraph.computeShortestPath(
          getIntensityData(), startIndex, endIndex, m_vertexOption);
//    C_Stack::kill(stackField);

    for (size_t i = path.size(); i > 0; --i) {
      int x, y, z;
      C_Stack::indexToCoord(path[i - 1], width, height, &x, &y, &z);
      voxelArray.append(ZVoxel(x, y, z));
    }
  }

  C_Stack::kill(partial);

  double length = voxelArray.getCurveLength();
  double dist = 0.0;

  const std::vector<ZVoxel> &voxelData = voxelArray.getInternalData();
  for (size_t i = 0; i < path.size(); ++i) {
    double ratio = dist / length;
    double r = r1 * ratio + r2 * (1 - ratio);
    voxelArray.setValue(i, r);
    if (i < path.size() - 1) {
      dist += voxelData[i].distanceTo(voxelData[i+1]);
    }
  }

  Swc_Tree *tree = voxelArray.toSwcTree();
  if (tree != NULL) {
    Swc_Tree_Translate(
          tree, stackOffset.getX(), stackOffset.getY(), stackOffset.getZ());
    ZSwcSignalFitter fitter;
    fitter.setBackground(m_backgroundType);
    fitter.setFixingTerminal(true);
    fitter.fitSignal(tree, getStack(), getSignalChannel());

    Swc_Tree_Node *leaf = tree->root;
    while (SwcTreeNode::firstChild(leaf) != NULL) {
      leaf = SwcTreeNode::firstChild(leaf);
    }
    SwcTreeNode::setPos(leaf, targetPos);
  }

  return tree;
}

Stack *ZNeuronTracer::binarize(const Stack *stack)
{
  Stack *out = C_Stack::clone(stack);

  ZStackBinarizer binarizer;
  binarizer.setMethod(ZStackBinarizer::BM_LOCMAX);
  binarizer.setRetryCount(3);
  if (binarizer.binarize(out) == false) {
    std::cout << "Thresholding failed" << std::endl;
    C_Stack::kill(out);
    out = NULL;
  }

  return out;
}

Stack* ZNeuronTracer::bwsolid(Stack *stack)
{
  Stack *clear_stack = NULL;

  const static int mnbr = 4;
  clear_stack = Stack_Majority_Filter_R(stack, NULL, 26, mnbr);

  Struct_Element *se = Make_Cuboid_Se(3, 3, 3);
  Stack *dilate_stack = Stack_Dilate_Fast(clear_stack, NULL, se);
  C_Stack::kill(clear_stack);
  Stack *fill_stack = dilate_stack;

  Stack *mask = Stack_Erode_Fast(fill_stack, NULL, se);
  C_Stack::kill(fill_stack);

  Kill_Struct_Element(se);

  return mask;
}

Stack* ZNeuronTracer::enhanceLine(const Stack *stack)
{
  double sigma[] = {1.0, 1.0, 1.0};
  FMatrix *result = NULL;

  if (stack->width * stack->height * stack->depth > 1024 * 1024 * 100) {
    result = El_Stack_L_F(stack, sigma, NULL);
  } else {
    result = El_Stack_F(stack, sigma, NULL);
  }

  Stack *out = Scale_Float_Stack(result->array, result->dim[0], result->dim[1],
      result->dim[2], GREY16);

  Kill_FMatrix(result);

  return out;
}


Geo3d_Scalar_Field* ZNeuronTracer::extractSeed(const Stack *mask)
{
  switch (m_seedingMethod) {
  case 1:
    return extractSeedOriginal(mask);
  case 2:
    return extractSeedSkel(mask);
  }

  return NULL;
}

Geo3d_Scalar_Field* ZNeuronTracer::extractLineSeed(
    const Stack *mask, const Stack *dist, int minObjSize)
{
  Object_3d_List *objList = Stack_Find_Object_N(
        const_cast<Stack*>(mask), NULL, 1, minObjSize, 26);
  ZObject3dArray objArray;
  objArray.append(objList);

  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(objArray.size());
  for (size_t i = 0; i < objArray.size(); ++i) {
    ZObject3d *obj = objArray[i];
    ZIntPoint pt = obj->getCentralVoxel();
    field->points[i][0] = pt.getX();
    field->points[i][1] = pt.getY();
    field->points[i][2] = pt.getZ();
    field->values[i] = sqrt(
          C_Stack::value(dist, pt.getX(), pt.getY(), pt.getZ()));
  }

  return field;
}

Geo3d_Scalar_Field* ZNeuronTracer::extractSeedSkel(const Stack *mask)
{
  Stack *skel = Stack_Bwthin(mask, NULL);

  /* alloc <dist> */
  Stack *dist = Stack_Bwdist_L_U16(mask, NULL, 0);


  ZStackProcessor::RemoveBranchPoint(skel, 26);

  Stack *skel_proc = C_Stack::clone(skel);
 Geo3d_Scalar_Field *field1 = extractLineSeed(skel_proc, dist);
 C_Stack::kill(skel_proc);

 for (int i = 0; i <field1->size; ++i) {
   int x = field1->points[i][0];
   int y = field1->points[i][1];
   int z = field1->points[i][2];
   Set_Stack_Pixel(skel, x, y, z, 0, 0);
 }

#ifdef _DEBUG_2
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", skel);
#endif


 Geo3d_Scalar_Field *field2 = extractLineSeed(skel, dist, 0);

 Geo3d_Scalar_Field *field = Geo3d_Scalar_Field_Merge(field1, field2, NULL);

 Kill_Geo3d_Scalar_Field(field1);
 Kill_Geo3d_Scalar_Field(field2);

#ifdef _DEBUG_2
  ZSwcTree tree;
  tree.forceVirtualRoot();
  for (int i = 0; i <field->size; ++i) {
    int x = field->points[i][0];
    int y = field->points[i][1];
    int z = field->points[i][2];
    double radius = field->values[i];
    SwcTreeNode::setFirstChild(
          tree.root(), SwcTreeNode::makePointer(x, y, z, radius));
  }
  tree.save(GET_TEST_DATA_DIR + "/test.swc");
#endif

  /* free <dist> */
  C_Stack::kill(dist);

  C_Stack::kill(skel);

  return field;
}

Geo3d_Scalar_Field* ZNeuronTracer::extractSeedOriginal(const Stack *mask)
{
  /* alloc <dist> */
  Stack *dist = Stack_Bwdist_L_U16(mask, NULL, 0);

  /* alloc <seeds> */
  Stack *seeds = Stack_Local_Max(dist, NULL, STACK_LOCMAX_CENTER);

  /* alloc <list> */
  Voxel_List *list = Stack_To_Voxel_List(seeds);

  /* alloc <pa> */
  Pixel_Array *pa = Voxel_List_Sampling(dist, list);

  /* free <dist> */
  C_Stack::kill(dist);

  /* alloc <voxel_array> */
  Voxel_P *voxel_array = Voxel_List_To_Array(list, 1, NULL, NULL);

  //double *pa_array = (double *) pa->array;
  uint16 *pa_array = (uint16 *) pa->array;

  printf("%d seeds found.\n", pa->size);

  /* alloc field */
  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(pa->size);
  field->size = 0;
  int i;
  for (i = 0; i < pa->size; i++) {
    if (IS_IN_OPEN_RANGE3(voxel_array[i]->x, voxel_array[i]->y,
                          voxel_array[i]->z, 0, seeds->width - 1,
                          0, seeds->height - 1, 0, seeds->depth - 1)) {
      field->points[field->size][0] = voxel_array[i]->x;
      field->points[field->size][1] = voxel_array[i]->y;
      field->points[field->size][2] = voxel_array[i]->z;
      field->values[field->size] = sqrt((double)pa_array[i]);
      field->size++;
    }
  }

  /* free <list> */
  Kill_Voxel_List(list);

  /* free <pa> */
  Kill_Pixel_Array(pa);

  /* free <voxel_array> */
  free(voxel_array);

  /* free <seeds> */
  C_Stack::kill(seeds);

  /* return <field> */
  return field;
}

int ZNeuronTracer::getRecoverLevel() const
{
  return m_recover;
}

void ZNeuronTracer::setRecoverLevel(int level)
{
  m_recover = level;
}

std::vector<Locseg_Chain*> ZNeuronTracer::recover(const Stack *stack)
{
  std::vector<Locseg_Chain*> chainArray;

  if (m_mask != NULL) {
    Stack *leftover = C_Stack::translate(m_mask, GREY, 0);
    Stack *traceMask = C_Stack::make(
          GREY, C_Stack::width(m_mask), C_Stack::height(m_mask),
          C_Stack::depth(m_mask));
    uint16_t *traceMaskArray =
        C_Stack::guardedArray16(m_traceWorkspace->trace_mask);
    size_t nvoxel = C_Stack::voxelNumber(m_mask);
    for (size_t i = 0; i < nvoxel; i++) {
      if ((traceMaskArray[i] > 0) || (m_baseMask->array[i] == 1)) {
        traceMask->array[i] = 1;
      } else {
        traceMask->array[i] = 0;
      }
    }

    C_Stack::kill(m_baseMask);
    m_baseMask = NULL;

    Stack *submask = Stack_Z_Dilate(traceMask, 5, stack, NULL);
    Stack_Bsub(leftover, submask, traceMask);

    C_Stack::kill(submask);
    submask = NULL;

    Stack_Remove_Small_Object(traceMask, leftover, 27, 26);
    C_Stack::translate(leftover, GREY, 1);

#ifdef _DEBUG_2
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", leftover);
#endif

    if (Stack_Is_Dark(leftover) == FALSE) {
      double originalMinLength = m_traceWorkspace->min_chain_length;
      if (m_traceWorkspace->refit == FALSE) {
        m_traceWorkspace->min_chain_length =
            (NEUROSEG_DEFAULT_H  - 1.0) * 2.0 - 1.0;
      } else {
        m_traceWorkspace->min_chain_length =
            (NEUROSEG_DEFAULT_H  - 1.0) * 1.5 - 1.0;
      }

      /* <seedPointArray> allocated */
      Geo3d_Scalar_Field *seedPointArray = extractSeed(leftover);
      C_Stack::kill(leftover);
      leftover = NULL;

      ZNeuronTraceSeeder seeder;
      prepareTraceScoreThreshold(TRACING_SEED);
      m_baseMask = seeder.sortSeed(seedPointArray, stack, m_traceWorkspace);

      /* <seedPointArray> freed */
      Kill_Geo3d_Scalar_Field(seedPointArray);

      std::vector<Local_Neuroseg>& locsegArray = seeder.getSeedArray();
      std::vector<double>& scoreArray = seeder.getScoreArray();
      chainArray = trace(stack, locsegArray, scoreArray);
      m_traceWorkspace->min_chain_length = originalMinLength;
    }
  }

  return chainArray;
}

std::vector<Locseg_Chain*> ZNeuronTracer::trace(const Stack *stack,
    std::vector<Local_Neuroseg> &locsegArray, std::vector<double> &values)
{
  prepareTraceScoreThreshold(TRACING_AUTO);

  int nchain;
  Locseg_Chain **chain =
      Trace_Locseg_S(stack, 1.0, &(locsegArray[0]), &(values[0]),
      locsegArray.size(), m_traceWorkspace, &nchain);

  std::vector<Locseg_Chain*> chainArray(nchain);

  for (int i = 0; i < nchain; ++i) {
    chainArray[i] = chain[i];
  }

  free(chain);

  return chainArray;
}

void ZNeuronTracer::clearBuffer()
{
  if (m_mask != NULL) {
    C_Stack::kill(m_mask);
    m_mask = NULL;
  }

  if (m_baseMask != NULL) {
    C_Stack::kill(m_baseMask);
    m_baseMask = NULL;
  }

  m_seedDsIntv.set(0, 0, 0);
}

#if 0
std::vector<Locseg_Chain*> ZNeuronTracer::screenChain(
    const Stack *stack, std::vector<Locseg_Chain*> &chainArray)
{
  std::vector<double> scoreArray(chainArray.size(), 0.0);
  std::vector<double> intensityArray(chainArray.size(), 0.0);
  std::vector<double> lengthArray(chainArray.size(), 0.0);

  std::vector<Locseg_Chain*> goodChainArray;

  double minIntensity = Infinity;
//  double minIntensity = 0.0;
  int count = 0;

  const double scoreThreshold = 0.6;
  size_t index = 0;
  for (std::vector<Locseg_Chain*>::iterator iter = chainArray.begin();
       iter != chainArray.end(); ++iter, ++index) {
    Locseg_Chain *chain = *iter;
    scoreArray[index] = Locseg_Chain_Average_Score(
          chain, stack, 1.0, STACK_FIT_CORRCOEF);
    intensityArray[index] = Locseg_Chain_Average_Signal(chain, stack, 1.0);
    //intensityArray[index] = Locseg_Chain_Min_Seg_Signal(chain, stack, 1.0);
    if (scoreArray[index] >= 0.5) {
      minIntensity += intensityArray[index];
      ++count;
      //intensityArray[index] = Locseg_Chain_Average_Signal(chain, stack, 1.0);
      //STACK_FIT_LOW_MEAN_SIGNAL
//      if (intensityArray[index] < minIntensity) {
//        minIntensity = intensityArray[index];
//      }
    }
  }

  /*
  if (count > 0) {
    minIntensity /= count;
  }
  */

  for (index = 0; index < chainArray.size(); ++index) {
    if (scoreArray[index] >= 0.5 || intensityArray[index] >= minIntensity) {
      goodChainArray.push_back(chainArray[index]);
    } else {
      delete chainArray[index];
    }
  }

  return goodChainArray;
}
#endif

std::vector<Locseg_Chain*> ZNeuronTracer::screenChain(
    const Stack *stack, std::vector<Locseg_Chain*> &chainArray)
{
  std::vector<double> scoreArray(chainArray.size(), 0.0);
  std::vector<double> intensityArray(chainArray.size(), 0.0);
  std::vector<double> lengthArray(chainArray.size(), 0.0);

  std::vector<Locseg_Chain*> goodChainArray;

  double minIntensity = Infinity;

  const double scoreThreshold = 0.6;
  size_t index = 0;
  for (std::vector<Locseg_Chain*>::iterator iter = chainArray.begin();
       iter != chainArray.end(); ++iter, ++index) {
    Locseg_Chain *chain = *iter;
    scoreArray[index] = Locseg_Chain_Average_Score(
          chain, stack, 1.0, STACK_FIT_CORRCOEF);
    intensityArray[index] = Locseg_Chain_Average_Signal(chain, stack, 1.0);
    lengthArray[index] = Locseg_Chain_Geolen(chain);
    //intensityArray[index] = Locseg_Chain_Min_Seg_Signal(chain, stack, 1.0);
    if (scoreArray[index] >= scoreThreshold) {
      //intensityArray[index] = Locseg_Chain_Average_Signal(chain, stack, 1.0);
      //STACK_FIT_LOW_MEAN_SIGNAL
      if (intensityArray[index] < minIntensity) {
        minIntensity = intensityArray[index];
      }
    }
  }

  for (index = 0; index < chainArray.size(); ++index) {
    if (scoreArray[index] >= scoreThreshold ||
        intensityArray[index] >= minIntensity) {
      goodChainArray.push_back(chainArray[index]);
    } else {
      delete chainArray[index];
    }
  }

  return goodChainArray;
}

ZSwcTree* ZNeuronTracer::trace(const ZStack *stack, bool doResampleAfterTracing)
{
  ZSwcTree *tree = NULL;

  if (stack != NULL) {
    Stack *signal = C_Stack::clone(stack->c_stack(m_preferredSignalChannel));

    if (signal != NULL) {
      tree = trace(signal, doResampleAfterTracing);
      C_Stack::kill(signal);
      if (tree != NULL) {
        tree->translate(stack->getOffset());
      }
    }
  }

  return tree;
}

Stack* ZNeuronTracer::computeSeedMask()
{
  return computeSeedMask(getStack()->c_stack(getSignalChannel()));
}

Stack* ZNeuronTracer::computeSeedMask(Stack *stack)
{
  if (m_backgroundType == NeuTube::IMAGE_BACKGROUND_BRIGHT) {
    double maxValue = C_Stack::max(stack);
    Stack_Csub(stack, maxValue);
  }

  ZStackProcessor::SubtractBackground(stack, 0.5, 3);

  //Extract seeds
  //First mask
  std::cout << "Binarizing ..." << std::endl;

  /* <bw> allocated */
  Stack *bw = binarize(stack);
  C_Stack::translate(bw, GREY, 1);

  std::cout << "Removing noise ..." << std::endl;

  /* <mask> allocated */
  Stack *mask = bwsolid(bw);

  /* <bw> freed */
  C_Stack::kill(bw);

  //Thin line mask
  /* <mask2> allocated */
  Stack *mask2 = NULL;

  if (m_enhancingMask) {
    std::cout << "Enhancing thin branches ..." << std::endl;
    mask2 = enhanceLine(stack);
  }

  if (mask2 != NULL) {
    std::cout << "Making mask for thin branches ..." << std::endl;
    ZStackBinarizer binarizer;
    binarizer.setMethod(ZStackBinarizer::BM_LOCMAX);
    binarizer.setRetryCount(5);
    binarizer.setMinObjectSize(27);

    if (binarizer.binarize(mask2) == false) {
      std::cout << "Thresholding failed" << std::endl;
      C_Stack::kill(mask2);
      mask2 = NULL;
    }
  }

  /* <mask2> freed */
  if (mask2 != NULL) {
    C_Stack::translate(mask2, GREY, 1);
    Stack_Or(mask, mask2, mask);
    C_Stack::kill(mask2);
    mask2 = NULL;
  }

  //Trace each seed
  std::cout << "Extracting seed points ..." << std::endl;

  /* <seedPointArray> allocated */
  Geo3d_Scalar_Field *seedPointArray = extractSeed(mask);

  int minSeedSize = 0;

  if (seedPointArray->size > 15000) {
    minSeedSize = 125;
  } else if (seedPointArray->size > 5000) {
    minSeedSize = 64;
  }

  if (minSeedSize > 0) {
    std::cout << "Too many seeds. Screening ..." << std::endl;
    Stack *tmpStack = C_Stack::clone(mask);
    mask = Stack_Remove_Small_Object(tmpStack, mask, minSeedSize, 26);
    C_Stack::kill(tmpStack);

    if (C_Stack::kind(mask) != GREY) {
      C_Stack::translate(mask, GREY, 1);
    }
    Kill_Geo3d_Scalar_Field(seedPointArray);
    seedPointArray = extractSeed(mask);
  }

  return mask;
}

ZSwcTree* ZNeuronTracer::trace(Stack *stack, bool doResampleAfterTracing)
{
  startProgress();

  ZSwcTree *tree = NULL;

  if (m_backgroundType == NeuTube::IMAGE_BACKGROUND_BRIGHT) {
    double maxValue = C_Stack::max(stack);
    Stack_Csub(stack, maxValue);
  }

  ZStackProcessor::SubtractBackground(stack, 0.5, 3);

#ifdef _DEBUG_2
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack);
#endif

  //Extract seeds
  //First mask
  std::cout << "Binarizing ..." << std::endl;

  /* <bw> allocated */
  Stack *bw = binarize(stack);
  C_Stack::translate(bw, GREY, 1);

  advanceProgress(0.05);

  std::cout << "Removing noise ..." << std::endl;

  /* <mask> allocated */
  Stack *mask = bwsolid(bw);
  advanceProgress(0.05);

  /* <bw> freed */
  C_Stack::kill(bw);

  //Thin line mask
  /* <mask2> allocated */
  Stack *mask2 = NULL;

  if (m_enhancingMask) {
    std::cout << "Enhancing thin branches ..." << std::endl;
    mask2 = enhanceLine(stack);
    advanceProgress(0.05);
  }

  if (mask2 != NULL) {
    std::cout << "Making mask for thin branches ..." << std::endl;
    ZStackBinarizer binarizer;
    binarizer.setMethod(ZStackBinarizer::BM_LOCMAX);
    binarizer.setRetryCount(5);
    binarizer.setMinObjectSize(27);

    if (binarizer.binarize(mask2) == false) {
      std::cout << "Thresholding failed" << std::endl;
      C_Stack::kill(mask2);
      mask2 = NULL;
    }
  }

  /* <mask2> freed */
  if (mask2 != NULL) {
    C_Stack::translate(mask2, GREY, 1);
    Stack_Or(mask, mask2, mask);
    C_Stack::kill(mask2);
    mask2 = NULL;
  }
  advanceProgress(0.05);

  //Trace each seed
  std::cout << "Extracting seed points ..." << std::endl;

  /* <seedPointArray> allocated */
  Geo3d_Scalar_Field *seedPointArray = extractSeed(mask);

  int minSeedSize = 0;

  if (seedPointArray->size > 15000) {
    minSeedSize = 125;
  } else if (seedPointArray->size > 5000) {
    minSeedSize = 64;
  }

  if (minSeedSize > 0) {
    std::cout << "Too many seeds. Screening ..." << std::endl;
    Stack *tmpStack = C_Stack::clone(mask);
    mask = Stack_Remove_Small_Object(tmpStack, mask, minSeedSize, 26);
    C_Stack::kill(tmpStack);

    if (C_Stack::kind(mask) != GREY) {
      C_Stack::translate(mask, GREY, 1);
    }
    Kill_Geo3d_Scalar_Field(seedPointArray);
    seedPointArray = extractSeed(mask);
  }

#ifdef _DEBUG_2
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", mask);
#endif


  m_mask = mask;

  advanceProgress(0.05);

  std::cout << "Sorting seeds ..." << std::endl;
  ZNeuronTraceSeeder seeder;
  prepareTraceScoreThreshold(TRACING_SEED);
  m_baseMask = seeder.sortSeed(seedPointArray, stack, m_traceWorkspace);

#ifdef _DEBUG_2
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", m_baseMask);
#endif

  advanceProgress(0.1);

  /* <seedPointArray> freed */
  Kill_Geo3d_Scalar_Field(seedPointArray);

  std::vector<Local_Neuroseg>& locsegArray = seeder.getSeedArray();
  std::vector<double>& scoreArray = seeder.getScoreArray();

  std::cout << "Tracing ..." << std::endl;

  /* <chainArray> allocated */

  std::vector<Locseg_Chain*> chainArray = trace(stack, locsegArray, scoreArray);

  if (m_recover > 0) {
    std::vector<Locseg_Chain*> newChainArray = recover(stack);
    chainArray.insert(
          chainArray.end(), newChainArray.begin(), newChainArray.end());
  }
  advanceProgress(0.1);

  if (chainArray.size() > 100) {
    std::cout << "Screening " << chainArray.size() << " tubes ..." << std::endl;
    chainArray = screenChain(stack, chainArray);
  }
//  chainArray = screenChain(stack, chainArray);
  advanceProgress(0.3);

  /* <mask2> freed */
//  C_Stack::kill(mask);

  std::cout << "Reconstructing ..." << std::endl;
  ZNeuronConstructor constructor;
  constructor.setWorkspace(m_connWorkspace);
  constructor.setSignal(stack);

  //Create neuron structure

  BOOL oldSpTest = m_connWorkspace->sp_test;
  if (chainArray.size() > 500) {
    std::cout << "Too many chains: " << chainArray.size() << std::endl;
    std::cout << "Turn off shortest path test" << std::endl;
    m_connWorkspace->sp_test = FALSE;
  }

  /* free <chainArray> */
  tree = constructor.reconstruct(chainArray);

  m_connWorkspace->sp_test = oldSpTest;

  advanceProgress(0.1);

  //Post process
  if (tree != NULL) {
    Swc_Tree_Remove_Zigzag(tree->data());
    Swc_Tree_Tune_Branch(tree->data());
    Swc_Tree_Remove_Spur(tree->data());
    Swc_Tree_Merge_Close_Node(tree->data(), 0.01);
    Swc_Tree_Remove_Overshoot(tree->data());

    if (doResampleAfterTracing) {
      ZSwcResampler resampler;
      resampler.optimalDownsample(tree);
    }

    ZSwcPruner pruner;
    pruner.setMinLength(0);
    pruner.removeOrphanBlob(tree);

    advanceProgress(0.1);
  }

  std::cout << "Done!" << std::endl;
  endProgress();

  return tree;
}

double ZNeuronTracer::findBestTerminalBreak(
    const ZPoint &terminalCenter, double terminalRadius,
    const ZPoint &innerCenter, double innerRadius, const Stack *stack)
{
  double d = terminalCenter.distanceTo(innerCenter);
  if (d < 0.5) {
    return 1.0;
  }

  ZPoint dvec = terminalCenter - innerCenter;
  dvec.normalize();

  double innerIntensity = Stack_Point_Sampling(
        stack, innerCenter.x(), innerCenter.y(), innerCenter.z());

  if (innerIntensity == 0.0) {
    return 1.0;
  }

  double lambda = 1.0;
  for (lambda = 1.0; lambda >= 0.3; lambda -= 0.1) {
    double radius = terminalRadius * lambda + innerRadius * (1 - lambda);
    ZPoint currentEnd = innerCenter + dvec * (d * lambda + radius);
    double terminalIntensity = Stack_Point_Sampling(
          stack, currentEnd.x(), currentEnd.y(), currentEnd.z());
    if (terminalIntensity / innerIntensity > 0.3) {
      break;
    }
  }

  return lambda;
}

void ZNeuronTracer::setMinScore(double score, ETracingMode mode)
{
  bool is2d = false;
  if (m_stack != NULL) {
    if (getStack()->depth() == 1) {
      is2d = true;
    }
  }

  if (is2d) {
    m_2dTraceMinScore = score;
  } else {
    switch (mode) {
    case TRACING_AUTO:
      m_autoTraceMinScore = score;
      break;
    case TRACING_INTERACTIVE:
      m_traceMinScore = score;
      break;
    case TRACING_SEED:
      m_seedMinScore = score;
      break;
    }
  }
}

void ZNeuronTracer::prepareTraceScoreThreshold(ETracingMode mode)
{
  bool is2d = false;
  if (m_stack != NULL) {
    if (getStack()->depth() == 1) {
      is2d = true;
    }
  }

  if (is2d) {
    m_traceWorkspace->min_score = m_2dTraceMinScore;
  } else {
    switch (mode) {
    case TRACING_AUTO:
      m_traceWorkspace->min_score = m_autoTraceMinScore;
      break;
    case TRACING_INTERACTIVE:
      m_traceWorkspace->min_score = m_traceMinScore;
      break;
    case TRACING_SEED:
      m_traceWorkspace->min_score = m_seedMinScore;
      break;
    }
  }
}

void ZNeuronTracer::initTraceWorkspace(Stack *stack)
{
  m_traceWorkspace =
      Locseg_Chain_Default_Trace_Workspace(m_traceWorkspace, stack);

  if (m_traceWorkspace->fit_workspace == NULL) {
    m_traceWorkspace->fit_workspace = New_Locseg_Fit_Workspace();
  }

  //m_traceWorkspace->min_score = 0.35;
  m_traceWorkspace->tune_end = ZNeuronTracerConfig::getInstance().tuningEnd();
  m_traceWorkspace->add_hit = TRUE;


  if (stack != NULL) {
    if (C_Stack::depth(stack) == 1) {
      m_traceWorkspace->min_score = m_2dTraceMinScore;
      Receptor_Fit_Workspace *rfw =
          (Receptor_Fit_Workspace*) m_traceWorkspace->fit_workspace;
      Default_R2_Rect_Fit_Workspace(rfw);
      rfw->sws->fs.n = 2;
      rfw->sws->fs.options[1] = STACK_FIT_CORRCOEF;
    }
  }
}


void ZNeuronTracer::initTraceWorkspace(ZStack *stack)
{
  if (stack == NULL || stack->channelNumber() != 1) {
    Stack *nstack = NULL;
    initTraceWorkspace(nstack);
  } else {
    initTraceWorkspace(stack->c_stack());
  }
}

void ZNeuronTracer::updateTraceWorkspace(
    int traceEffort, bool traceMasked, double xRes, double yRes, double zRes)
{
  if (traceEffort > 0) {
    m_traceWorkspace->refit = FALSE;
  } else {
    m_traceWorkspace->refit = TRUE;
  }

  m_traceWorkspace->resolution[0] = xRes;
  m_traceWorkspace->resolution[1] = yRes;
  m_traceWorkspace->resolution[2] = zRes;

  loadTraceMask(traceMasked);
}

void ZNeuronTracer::loadTraceMask(bool traceMasked)
{
  if (traceMasked) {
    Trace_Workspace_Set_Fit_Mask(m_traceWorkspace, m_traceWorkspace->trace_mask);
  } else {
    Trace_Workspace_Set_Fit_Mask(m_traceWorkspace, NULL);
  }
}

void ZNeuronTracer::initConnectionTestWorkspace()
{
  if (m_connWorkspace == NULL) {
    m_connWorkspace = New_Connection_Test_Workspace();
    m_connWorkspace->sp_test = ZNeuronTracerConfig::getInstance().spTest();
    m_connWorkspace->crossover_test =
        ZNeuronTracerConfig::getInstance().crossoverTest();
  }
}

void ZNeuronTracer::updateConnectionTestWorkspace(
    double xRes, double yRes, double zRes,
    char unit, double distThre, bool spTest, bool crossoverTest)
{
  m_connWorkspace->resolution[0] = xRes;
  m_connWorkspace->resolution[1] = yRes;
  m_connWorkspace->resolution[2] = zRes;
  m_connWorkspace->unit = unit;
  m_connWorkspace->dist_thre = distThre;
  m_connWorkspace->sp_test = spTest;
  m_connWorkspace->crossover_test = crossoverTest;
}

/*
void ZNeuronTracer::setStackOffset(const ZIntPoint &pt)
{
  setStackOffset(pt.getX(), pt.getY(), pt.getZ());
}
*/

void ZNeuronTracer::setTraceLevel(int level)
{
  initTraceWorkspace(m_stack);
  initConnectionTestWorkspace();

  config();

  if (level > 0) {
    loadJsonObject(ZNeuronTracerConfig::getInstance().getLevelJson(level));
  }

#if 0
  if (m_traceWorkspace == NULL) {
    initTraceWorkspace(m_stack);
  }

  if (m_connWorkspace == NULL) {
    initConnectionTestWorkspace();
  }

//  m_traceWorkspace->tune_end = FALSE;
  m_traceWorkspace->refit = FALSE;
  m_connWorkspace->sp_test = FALSE;
  m_connWorkspace->crossover_test = FALSE;
  m_traceWorkspace->tune_end = TRUE;
  m_enhancingMask = false;
  m_seedingMethod = 1;
  m_recover = 0;

  if (level >= 2) {
    m_seedingMethod = 2;
  }

  if (level >= 3) {
    m_connWorkspace->sp_test = TRUE;
  }

  if (level >= 4) {
    m_enhancingMask = true;
  }

  if (level >= 5) {
    m_recover = 1;
  }

  if (level >= 6) {
    m_traceWorkspace->refit = TRUE;
  }
#endif
}

/*
const char *ZNeuronTracer::m_levelKey = "level";
const char *ZNeuronTracer::m_minimalScoreKey = "minimalScore";
const char *ZNeuronTracer::m_minimalSeedScoreKey = "minimalSeedScore";
const char *ZNeuronTracer::m_spTestKey = "spTest";
const char *ZNeuronTracer::m_enhanceLineKey = "enhanceLine";
*/
void ZNeuronTracer::loadJsonObject(const ZJsonObject &obj)
{
#ifdef _DEBUG_
  obj.print();
#endif

  const char *key = ZNeuronTracerConfig::getLevelKey();
  if (obj.hasKey(key)) {
    setTraceLevel(ZJsonParser::integerValue(obj[key]));
  }

  key = ZNeuronTracerConfig::getMinimalManualScoreKey();
  if (obj.hasKey(key)) {
    m_traceMinScore = ZJsonParser::numberValue(obj[key]);
  }

  key = ZNeuronTracerConfig::getMinimalSeedScoreKey();
  if (obj.hasKey(key)) {
    m_seedMinScore = ZJsonParser::numberValue(obj[key]);
  }

  key = ZNeuronTracerConfig::getSpTestKey();
  if (obj.hasKey(key)) {
    m_connWorkspace->sp_test = ZJsonParser::booleanValue(obj[key]);
  }

  key = ZNeuronTracerConfig::getEnhanceLineKey();
  if (obj.hasKey(key)) {
    m_enhancingMask = ZJsonParser::booleanValue(obj[key]);
  }

  key = ZNeuronTracerConfig::getRefitKey();
  if (obj.hasKey(key)) {
    m_traceWorkspace->refit = ZJsonParser::booleanValue(obj[key]);
  }

  key = ZNeuronTracerConfig::getSeedMethodKey();
  if (obj.hasKey(key)) {
    m_seedingMethod = ZJsonParser::integerValue(obj[key]);
  }

  key = ZNeuronTracerConfig::getRecoverKey();
  if (obj.hasKey(key)) {
    m_recover = ZJsonParser::integerValue(obj[key]);
  }

  key = ZNeuronTracerConfig::getCrossoverTestKey();
  if (obj.hasKey(key)) {
    m_connWorkspace->crossover_test = ZJsonParser::booleanValue(obj[key]);
  }

  key = ZNeuronTracerConfig::getTuneEndKey();
  if (obj.hasKey(key)) {
    m_traceWorkspace->tune_end = ZJsonParser::booleanValue(obj[key]);
  }

  key = ZNeuronTracerConfig::getEdgePathKey();
  if (obj.hasKey(key)) {
    m_usingEdgePath = ZJsonParser::booleanValue(obj[key]);
  }
}
