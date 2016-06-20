/**@file zswctree.h
 * @brief Swc tree
 * @author Ting Zhao
 */

#ifndef _ZSWCTREE_H_
#define _ZSWCTREE_H_

#include "zqtheader.h"

#include <vector>
#include <string>
#include <set>

#include "tz_swc_tree.h"
#include "zstackobject.h"
#include "zpoint.h"
#include "zswcpath.h"
#include "zcuboid.h"
#include "zuncopyable.h"
#include "zswctreenodeselector.h"


class ZSwcForest;
class ZSwcBranch;
class ZSwcTrunkAnalyzer;
class QPointF;
class ZClosedCurve;
class ZRect2d;

//! SWC tree class
/*!
 *It is a c++ wrapper of the Swc_Tree structure.
 *
 *The definition of the SWC file format can be found at
 *  <a href="http://research.mssm.edu/cnic/swc.html">CNIC website</a>
 *
 *Briefly, the basic element of an SWC tree is a node with the following
 *properties: ID, type, x, y, z, radius, parent ID. ZSwcTree provides interfaces
 *for iterating through the nodes with several modes.
 *Here any node with negative ID will be treated as virtual. A virtual
 *node, which is NOT considered as a part of the tree model, is introduced to
 *facilitate hosting multiple trees or handling specific data structure such as
 *binary trees. An object containing multiple trees is called a forest. All
 *normal operations should assume that ONLY the root node can
 *be virtual. The children of a virtual root are called regular roots, because
 *each of them is the root of a real SWC tree.
 *
 *To be compatible with some legacy code, the class is also a wrapper of the
 *C structure Swc_Tree, which is called a raw SWC tree here.
 */

class ZSwcTree : public ZStackObject, ZUncopyable {
public:
  //! Action of cleaning the existing data
  enum ESetDataOption {
    CLEAN_ALL, /*!< Clean all associated memory */
    FREE_WRAPPER, /*!< Free the wrapper pointer only */
    LEAVE_ALONE /*!< Do nothing */
  };

  //! Selection mode
  enum ESelectionMode {
    SWC_NODE, /*!< Single node */
    WHOLE_TREE, /*!< Whole tree */
    SWC_BRANCH /*!< Single branch */
  };

  //! Host state of a node
  enum ENodeState {
    NODE_STATE_COSMETIC
  };

  //Structral mode
  enum EStructrualMode {
    STRUCT_CLOSED_CURVE, STRUCT_NORMAL
  };

  enum EOperation {
    OPERATION_NULL,
    OPERATION_DELETE_NODE, OPERATION_SELECT_ALL_NODE,
    OPERATION_MOVE_NODE_LEFT, OPERATION_MOVE_NODE_LEFT_FAST,
    OPERATION_MOVE_NODE_RIGHT, OPERATION_MOVE_NODE_RIGHT_FAST,
    OPERATION_MOVE_NODE_UP, OPERATION_MOVE_NODE_UP_FAST,
    OPERATION_MOVE_NODE_DOWN, OPERATION_MOVE_NODE_DOWN_FAST,
    OPERATION_ADD_NODE,
    OPERATION_INCREASE_NODE_SIZE, OPERATION_DECREASE_NODE_SIZE,
    OPERATION_CONNECT_NODE, OPERATION_CONNECT_NODE_SMART,
    OPERATION_BREAK_NODE, OPERATION_CONNECT_ISOLATE,
    OPERATION_ZOOM_TO_SELECTED_NODE, OPERATION_INSERT_NODE, OPERATION_MOVE_NODE,
    OPERATION_RESET_BRANCH_POINT, OPERATION_CHANGE_NODE_FACUS,
    OPERATION_SET_AS_ROOT,
    OPERATION_EXTEND_NODE, OPERATION_SELECT, OPERATION_SELECT_CONNECTION,
    OPERATION_SELECT_FLOOD
  };

//  typedef uint32_t TVisualEffect;

//  const static TVisualEffect VE_NONE;
//  const static TVisualEffect VE_FULL_SKELETON;

  /** @name Constructors
   */
  ///@{

  /*!
   * \brief Copy constructor.
   *
   * It performs deep copy of a tree, i.e. there is no memory sharing between
   * the constructed object and \src . Intermediate results will not be copied.
   *
   * \param src Original object.
   */
  //ZSwcTree(const ZSwcTree &src);

  /*!
   * \brief Default constructor.
   */
  ZSwcTree();
  ///@}

  /*!
   * \brief Deconstructor.
   */
  ~ZSwcTree();

  virtual const std::string& className() const;

  friend void swap(ZSwcTree& first, ZSwcTree& second);
  ZSwcTree& operator=(const ZSwcTree &other);

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_SWC;
  }

public:
  /** @name Set data.
   * Load tree data into an object.
   */
  ///@{
  /*!
   * \brief Set the data to \a tree.
   *
   * \a tree will be owned by the object.
   *
   * \param tree The data.
   * \param option Optioni of handling existing data.
   */
  void setData(Swc_Tree *tree, ESetDataOption option = CLEAN_ALL);

  /*!
   * \brief Set the data from a node.
   *
   * \a tn becomes the root of the tree. The old data is treated based on
   * \a option:
   *    CLEAN_ALL (default): Clean all old data.
   *    FREE_WRAPPER: Free the old wrapper only.
   *    LEAVE_ONLY: Don't do anything.
   *
   * \a tn will be detached from its old parent after the operation.
   */
  void setDataFromNode(Swc_Tree_Node *tn, ESetDataOption option = CLEAN_ALL);

  void setDataFromNodeRoot(Swc_Tree_Node *tn,
                           ESetDataOption option = CLEAN_ALL);
  ///@}

  /*!
   * \brief Clone an SWC tree
   */
  ZSwcTree *clone() const;

  /*!
   * \brief Clone a raw SWC tree
   */
  Swc_Tree *cloneData() const;
  //Swc_Tree* copyData() const;  //copy from m_tree

  inline bool hasData() const { return m_tree != NULL; }
  inline Swc_Tree* data() const { return m_tree; }
  inline Swc_Tree_Node* root() const {
    return (m_tree != NULL) ? m_tree->root : NULL;
  }

  /*!
   * \brief Test if a tree is empty.
   * \return true iff the tree has no regular node.
   */
  bool isEmpty() const;

  /*!
   * \brief Test if a tree has any regular node.
   */
  bool hasRegularNode() const;

  /*!
   * \brief Test if a tree is valid.
   *
   * \return true iff there is no link loop.
   */
  bool isValid();

  /*!
   * \brief Test if a tree is a forest.
   *
   * \return true iff it has more than one regular roots.
   */
  bool isForest() const;

public:
  virtual void display(ZPainter &painter, int slice, EDisplayStyle option,
                       NeuTube::EAxis axis) const;

//  bool hasVisualEffect(TVisualEffect ve) const;
//  void addVisualEffect(TVisualEffect ve);
//  void removeVisualEffect(TVisualEffect ve);

  /*!
   * \brief save Save swc
   * \param filePath
   */
  void save(const char *filePath);
  bool load(const char *filePath);
  void save(const std::string &filePath);
  void load(const std::string &filePath);

  /*!
   * \brief Load swc from buffer
   *
   * It does not support extended json style sheet.
   *
   * \param buffer It must end with '\0'.
   */
  void loadFromBuffer(const char *buffer);

  virtual int swcFprint(FILE *fp, int start_id = 0, int parent_id = -1,
                        double z_scale = 1.0);
  virtual void swcExport(const char *filePath);

  void print(int iterOption = SWC_TREE_ITERATOR_DEPTH_FIRST) const;
  std::string toString(int iterOption = SWC_TREE_ITERATOR_DEPTH_FIRST) const;

  // convert swc to locsegchains..., return next .tb file idx
  int saveAsLocsegChains(const char *prefix, int startNum);

  /*!
   * \brief Test if a node is a part of the tree
   */
  bool contains(const Swc_Tree_Node *tn) const;

  int regularRootNumber() const;
  void addRegularRoot(Swc_Tree_Node *tn);

  Swc_Tree_Node *maxLabelNode();

  enum EComponent {
    DEPTH_FIRST_ARRAY, BREADTH_FIRST_ARRAY, LEAF_ARRAY, TERMINAL_ARRAY,
    BRANCH_POINT_ARRAY, Z_SORTED_ARRAY, BOUND_BOX, ALL_COMPONENT
  };

  bool isDeprecated(EComponent component) const;
  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);

  inline void addComment(const std::string &comment) {
    m_comment.push_back(comment);
  }

public:
  int size();
  int size(Swc_Tree_Node *start);

  /** @name SWC iterator routines
   *
   *  It is necessary to call updateIterator first before using
   *  begin() and next() to perform iteration. The iterating options are:
   *   SWC_TREE_ITERATOR_DEPTH_FIRST: depth first iteration
   *   SWC_TREE_ITERATOR_BREADTH_FIRST: breadth first iteration
   *   SWC_TREE_ITERATOR_LEAF: iterate through leaves
   *   SWC_TREE_ITERATOR_BRANCH_POINT: iterate through branch points
   *   SWC_TREE_ITERATOR_NO_UPDATE : no update, reset the internal iterator
   *     point to the begin
   *   SWC_TREE_ITERATOR_VOID: no update and no change on the current iterator
   *   SWC_TREE_ITERATOR_REVERSE: reverse the current iterator
   *
   *  An example of iterating through nodes:
   *  @code
   *      ZSwcTree tree;
   *      ... //Suppose the tree is loaded with some real nodes
   *      tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST); //Depth first
   *      for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = next()) {
   *          std::cout << "Swc node: " << SwcTreeNode::id(tn) << std::endl;
   *      }
   *  @endcode
   */
  ///@{
  /*!
   * \brief Update iterator
   * \param option Iterator option.
   * \param indexing Indexing the nodes or not.
   * \return
   */
  int updateIterator(int option = SWC_TREE_ITERATOR_DEPTH_FIRST,
                     BOOL indexing = FALSE) const;
  int updateIterator(int option, const std::set<Swc_Tree_Node*> &blocker,
                     BOOL indexing = FALSE) const;
  /*!
   * \brief Update iterator with a starting node.
   *
   * When \a start is specified, the iteration will be on the subtree started
   * from \a start.
   *
   * \param option Iterator option.
   * \param start The starting node.
   * \param indexing Indexing the nodes or not.
   * \return
   */
  int updateIterator(int option, Swc_Tree_Node *start, BOOL indexing) const;

  /*!
   * \brief Update iterator with a start and blocked branches.
   *
   * When the iterator enouncters a node in <blocker>, it would stop going
   * forward along the corresponding pathway.
   * It does not matter whether <start> is in <blocker> or not.
   *
   * \param option Iterator option.
   * \param start The starting node.
   * \param blocker The set of blocking nodes.
   * \param indexing Add indices to the nodes or not.
   * \return The number of nodes.
   */
  int updateIterator(int option, Swc_Tree_Node *start,
                     const std::set<Swc_Tree_Node*> &blocker,
                     BOOL indexing = FALSE) const;

  inline Swc_Tree_Node* begin() {
    return const_cast<const ZSwcTree*>(this)->begin();
  }

  inline Swc_Tree_Node* begin() const {
    if (m_tree == NULL) {
      return NULL;
    }

    if (m_tree->begin != NULL) {
      m_tree->iterator = m_tree->begin->next;
      m_tree->begin->tree_state = m_tree->tree_state;
    }

    return m_tree->begin;
  }

  inline Swc_Tree_Node* end() { return NULL; }
  inline Swc_Tree_Node* end() const { return NULL; }

  inline Swc_Tree_Node* next() { return Swc_Tree_Next(m_tree); }
  inline Swc_Tree_Node* next() const { return Swc_Tree_Next(m_tree); }
  ///@}

  bool labelBranch(double x, double y, double z, double thre);
  void labelBranchLevel(int rootType = -1);
  void labelBranchLevelFromLeaf();

  /*!
   * \brief Get bound box of the tree
   *
   * \param corner The array to store the bound box. The first corner is
   *        (\a corner[0], \a corner[1], corner[2]) and the last corner is
   *        (\a corner[3], \a corner[4], corner[5]).
   */
  void boundBox(double *corner) const;

  /*!
   * \brief Get bound box of the tree
   *
   * \return The bound box.
   */
   const ZCuboid& getBoundBox() const;
   using ZStackObject::getBoundBox; // warning: 'ZSwcTree::getBoundBox' hides overloaded virtual function [-Woverloaded-virtual]

  static ZSwcTree* CreateCuboidSwc(const ZCuboid &box, double radius = 1.0);
  ZSwcTree* createBoundBoxSwc(double margin = 0.0);

  /*!
   * \brief Hit test.
   */
  Swc_Tree_Node* hitTest(double x, double y, double z);

  /*!
   * \brief Plane hit test
   *
   * It tests if a point (\a x, \a y) hits the X-Y projection of the tree.
   *
   * \param x X coodinate of the hit point
   * \param y Y coodinate of the hit point
   * \return  Returns the closest node to (\a x, \a y) if there is hit.
   *          If there is no hit, it returns NULL.
   */
  Swc_Tree_Node* hitTest(double x, double y, NeuTube::EAxis axis);

  /*!
   * \brief Hit a node within an expanded region
   *
   * It is similart to hitTest except that the hitting is positive when the
   * point within a node with size expanded by margin (radius + \a margin).
   *
   * \return The closest node. Returns NULL if no node is hit.
   */
  Swc_Tree_Node* hitTest(double x, double y, double z, double margin);

  /*!
   * \brief ZStackObject hit function implementation
   */
  bool hit(double x, double y, NeuTube::EAxis axis);
  bool hit(double x, double y, double z);

  /*!
   * \brief Selecte a node
   * \param tn It is supposed to be a part of the tree. The function does not
   *      check the membership for effieciecy purpose.
   * \param appending Add the node to the selection set with in appending way
   *      or not.
   */
  void selectNode(Swc_Tree_Node *tn, bool appending);
  void deselectNode(Swc_Tree_Node *tn);
  void selectAllNode();
  void deselectAllNode();

  void inverseSelection();

  void recordSelection();
  void processSelection();
#if defined(_QT_GUI_USED_)
  void selectNode(const ZRect2d &roi, bool appending);
#endif
  Swc_Tree_Node* selectHitNode(bool appending);
  Swc_Tree_Node* deselectHitNode();

  template<class InputIterator>
  void selectNode(
      InputIterator first, InputIterator last, bool appending);

  void selectNodeConnection(Swc_Tree_Node *seed);
  void selectNodeFloodFilling(Swc_Tree_Node *seed);

  void selectHitNodeConnection();
  void selectHitNodeFloodFilling();
  void selectNeighborNode();
  void selectConnectedNode();
  void selectUpstreamNode();
  void selectDownstreamNode();
  void selectBranchNode();
  void selectSmallSubtree(double maxLength);

  const std::set<Swc_Tree_Node*>& getSelectedNode() const;
  bool hasSelectedNode() const;
  bool isNodeSelected(const Swc_Tree_Node *tn) const;

  const ZSwcTreeNodeSelector& getNodeSelector() const {
    return m_selector;
  }

  Swc_Tree_Node* getHitNode() const { return m_hitSwcNode; }
  void setHitNode(Swc_Tree_Node *tn) { m_hitSwcNode = tn; }

  void toSvgFile(const char *filePath);

  // move soma (first root) to new location
  void translateRootTo(double x, double y, double z);
  // rescale location and radius
  void rescale(double scaleX, double scaleY, double scaleZ,
               bool changingRadius = true);
  void rescale(double srcPixelPerUmXY, double srcPixelPerUmZ,
               double dstPixelPerUmXY, double dstPixelPerUmZ);
  // rescale radius of nodes in certain depth range, startdepth <= depth of node < enddepth
  // 0 is the depth of roots. enddepth can be -1 which means max depth
  void rescaleRadius(double scale, int startdepth, int enddepth);

  //r2 = r * scale + dr
  void changeRadius(double dr, double scale);

  int swcNodeDepth(Swc_Tree_Node *tn);
  // reduce node number, similar to Swc_Tree_Merge_Close_Node, but only merge Continuation node
  void reduceNodeNumber(double lengthThre);

  /*!
   * \brief Compute distance between two trees
   *
   * It computes shortest distance between the terminal centers of the object
   * and the surface of \a tree.
   *
   * \param tree The partner tree for distance calculation
   * \param source Source node closest to \a tree
   * \param A node in \a tree closest to the tree
   */
  double distanceTo(ZSwcTree *tree, Swc_Tree_Node **getSource = NULL,
                    Swc_Tree_Node **target = NULL);

  /*!
   * \brief Compute distance to a node
   *
   * \param target The pointer to store closest node
   *
   * \return The surface distance. It returns 0 if \a is not regular.
   */
  double distanceTo(Swc_Tree_Node *getSource, Swc_Tree_Node **target = NULL) const;

  double distanceTo(
      double x, double y, double z, double zScale, Swc_Tree_Node **node = NULL) const;

  /*!
   * \brief Convert the tree into an array of trees.
   *
   * It will move all regular nodes in the current tree to the returned forest.
   * After the operation, the current tree becomes empty. The user is responsible
   * to free the returned object. It returns NULL if the current tree is empty.
   *
   * \return An array of trees.
   */
  ZSwcForest* toSwcTreeArray();

  int resortId();

  void flipY(double height);

  void removeRedundantBranch(double redundacyThreshold);
  double computeRedundancy(Swc_Tree_Node *leaf);
  void removeBranch(Swc_Tree_Node *tn);

  std::vector<int> shollAnalysis(double rStart, double rEnd, double rStep,
                                 ZPoint center);

  Swc_Tree_Node *firstRegularRoot() const;
  Swc_Tree_Node *firstLeaf();

  std::vector<Swc_Tree_Node*> getTerminalArray();

  ZSwcBranch *extractBranch(int beginId, int endId);
  ZSwcBranch *extractBranch(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
  ZSwcBranch *extractBranch(int setLabel);
  ZSwcBranch *extractLongestBranch();
  ZSwcBranch *extractFurthestBranch();

  ZSwcPath getLongestPath();

  std::vector<Swc_Tree_Node*> extractLeaf(Swc_Tree_Node *start);

  Swc_Tree_Node* queryNode(int id,
                           int iterOption = SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Node* queryNode(const ZPoint &pt);

  ZPoint somaCenter();

  std::vector<double> computeAllContinuationAngle(bool rotating = false);
  std::vector<double> computeAllBranchingAngle();

  void merge(Swc_Tree *tree, bool freeInput = false);
  void merge(ZSwcTree *tree, bool freeInput = false);

  void setLabel(int v) const;
  void setType(int type);

  void translate(const ZPoint& offset);
  void translate(const ZIntPoint &offset);
  void translate(double x, double y, double z);
  void scale(double sx, double sy, double sz);
  //Rotate swc tree around a point
  void rotate(double theta, double psi, const ZPoint& center,
              bool inverse = false);

  ZPoint computeCentroid() const;

  void resample(double step);

  Swc_Tree_Node* removeRandomBranch();

  /*!
   * \brief Label a subtree
   * \param tn the root of the subtree. It must be a node in the current tree,
   * otherwise the behavior is undefined.
   */
  void labelSubtree(Swc_Tree_Node *tn, int label);

  /*!
   * \brief Add a label to the subtree
   * the new label is the current label plus \a label.
   */
  void addLabelSubtree(Swc_Tree_Node *tn, int label);

  //void labelTrunk(int flag, int setLabel, Swc_Tree_Node *start);
  void labelTrunkLevel(ZSwcTrunkAnalyzer *trunkAnalyzer);

  /*!
   * \brief Mark soma nodes
   *
   * Set type of soma nodes to <somaType> and type of other nodes to <otherType>. The thickest node
   * of each tree will be set as root.
   *
   * \param radiusThre the radius threshold of the soma nodes. Soma is marked only if the radius of
   * the thickest node is larger than or equal to <radiusThre>, otherwise set all nodes type to <otherType>
   */
  void markSoma(double radiusThre = 0, int somaType = 1, int otherType = 0);

  int regularDepth();

  ZSwcPath mainTrunk(ZSwcTrunkAnalyzer *trunkAnalyzer);
  ZSwcPath subTrunk(Swc_Tree_Node *start, int setLabel);
  ZSwcPath subTrunk(Swc_Tree_Node *start,
                    const std::set<Swc_Tree_Node*> &blocker,
                    ZSwcTrunkAnalyzer *trunkAnalyzer);

  std::vector<ZSwcPath> getBranchArray();

  //void labelBusyLevel();

  void setTypeByLabel();
  void moveToSurface(double *x, double *y, double *z);
  void moveToSurface(ZPoint *pt);

  inline void deactivateIterator() { m_iteratorReady = true; }
  inline void activateIterator() { m_iteratorReady = false; }

  Swc_Tree_Node* forceVirtualRoot();

  void setBranchSizeWeight();

  double length();
  double length(int type);

  /*!
   * \brief Compute back trace length and store the result as node weight
   *
   * \return overall length
   */
  double computeBackTraceLength();

  std::set<int> typeSet();

  bool hasGoodSourceName();

  std::vector<Swc_Tree_Node*> toSwcTreeNodeArray(bool includingVirtual = true);

  enum EIteratorOption {
    DEPTH_FIRST_ITERATOR, BREADTH_FIRST_ITERATOR, LEAF_ITERATOR, TERMINAL_ITERATOR,
    BRANCH_POINT_ITERATOR, Z_SORT_ITERATOR
  };

  /*!
   * \brief Get the node array under a certain condition.
   *
   * Only regular nodes are included for the Z_SORT_ITERATOR option.
   */
  const std::vector<Swc_Tree_Node*>& getSwcTreeNodeArray(
      EIteratorOption iteratorOption = DEPTH_FIRST_ITERATOR) const;

  void labelStack(Stack *stack);

  /*!
   * \brief Get the length of the longest segment
   * \return
   */
  double getMaxSegmentLenth();

  /*!
   * \brief Check if a tree is linear
   *
   * A linear tree is a non-empty tree that has no branch point and no break
   * point.
   */
  bool isLinear() const;


  enum EColorScheme {
    COLOR_NORMAL, COLOR_ROI_CURVE
  };

  void setColorScheme(EColorScheme scheme);

  void initHostState(int state);
  void initHostState();

  //void setHostState(Swc_Tree_Node *tn, ENodeState state) const;
  void setHostState(Swc_Tree_Node *tn) const;

  inline EStructrualMode getStructrualMode() const {
    return m_smode;
  }
  void setStructrualMode(EStructrualMode mode);

  ZClosedCurve toClosedCurve() const;

  void updateHostState();

  //Iterator classes
  class ExtIterator {
  public:
    ExtIterator(const ZSwcTree *tree);
    virtual ~ExtIterator();

    /*!
     * \brief Restart the iterator.
     */
    virtual void restart() = 0;

    /*!
     * \brief Restart the iterator and get the first node
     */
    virtual Swc_Tree_Node *begin() = 0;

    virtual bool hasNext() const = 0;

    /*!
     * \brief Get the next node.
     *
     * Note that the it returns the first node if the iterator is just started.
     * It returns NULL when it reaches the end.
     */
    virtual Swc_Tree_Node *next() = 0;

    void excludeVirtual(bool on) {
      m_excludingVirtual = on;
    }

  protected:
     void init(const ZSwcTree *tree);

  protected:
    ZSwcTree *m_tree;
    Swc_Tree_Node *m_currentNode;
    bool m_excludingVirtual;
  };

  class RegularRootIterator : public ExtIterator {
  public:
    RegularRootIterator(const ZSwcTree *tree);
    void restart();
    Swc_Tree_Node *begin();
    bool hasNext() const;
    Swc_Tree_Node *next();
  };

  class DepthFirstIterator : public ExtIterator {
  public:
    DepthFirstIterator(const ZSwcTree *tree);
    void restart();
    Swc_Tree_Node *begin();
    bool hasNext() const;
    Swc_Tree_Node* next();
  };

  class LeafIterator : public ExtIterator {
  public:
    LeafIterator(const ZSwcTree *tree);
    void restart();
    Swc_Tree_Node *begin();
    bool hasNext() const;
    Swc_Tree_Node* next();
  private:
    std::vector<Swc_Tree_Node*> m_nodeArray;
    size_t m_currentIndex;
  };

  class TerminalIterator : public ExtIterator {
  public:
    TerminalIterator(const ZSwcTree *tree);
    void restart();
    Swc_Tree_Node *begin();
    bool hasNext() const;
    Swc_Tree_Node* next();
  private:
    std::vector<Swc_Tree_Node*> m_nodeArray;
    size_t m_currentIndex;
  };

  class DownstreamIterator : public ExtIterator {
  public:
    DownstreamIterator(Swc_Tree_Node *tn);
    void restart();
    Swc_Tree_Node *begin();
    bool hasNext() const;
    Swc_Tree_Node* next();
  private:
    std::vector<Swc_Tree_Node*> m_nodeArray;
    size_t m_currentIndex;
  };

public: //static functions
  static std::vector<ZSwcTree*> loadTreeArray(std::string dirPath);
  static Swc_Tree_Node* makeArrow(const ZPoint &startPos, double startSize,
                                  int startType,
                                  const ZPoint &endPos, double endSize,
                                  int endType, bool addBreak = true);

  static ZSwcTree* generateRandomSwcTree(int n, double branchingProb,
                                         double contAngleMu,
                                         double contAngleSigma,
                                         double branchAngleMu,
                                         double branchAngleSigma);

  static bool getHostState(const Swc_Tree_Node *tn, ENodeState state);

  static ETarget GetDefaultTarget();


private:
  static void computeLineSegment(const Swc_Tree_Node *lowerTn,
                                 const Swc_Tree_Node *upperTn,
                                 QPointF &lineStart, QPointF &lineEnd,
                                 bool &visible, int dataFocus, bool isProj);
  std::pair<const Swc_Tree_Node *, const Swc_Tree_Node *>
  extractCurveTerminal() const;
  int getTreeState() const;

#ifdef _QT_GUI_USED_
  const QColor& getNodeColor(const Swc_Tree_Node *tn, bool isFocused) const;
#endif

private:
  Swc_Tree *m_tree;
  EStructrualMode m_smode;
//  TVisualEffect m_visualEffect;

  mutable bool m_iteratorReady; /* When this option is on, any iterator option changing
                           internal linked list
                           is turned off and SWC_TREE_ITERATOR_NO_UPDATE is
                           applied instead */

  mutable std::vector<Swc_Tree_Node*> m_depthFirstArray;
  mutable std::vector<Swc_Tree_Node*> m_breadthFirstArray;
  mutable std::vector<Swc_Tree_Node*> m_leafArray;
  mutable std::vector<Swc_Tree_Node*> m_terminalArray;
  mutable std::vector<Swc_Tree_Node*> m_branchPointArray;
  mutable std::vector<Swc_Tree_Node*> m_zSortedArray;
  mutable std::set<Swc_Tree_Node*> m_selectedNode;
  mutable std::set<Swc_Tree_Node*> m_prevSelectedNode;
  mutable ZSwcTreeNodeSelector m_selector;

  mutable ZCuboid m_boundBox;

  static const int m_nodeStateCosmetic;

  Swc_Tree_Node *m_hitSwcNode;
  std::vector<std::string> m_comment;

#ifdef _QT_GUI_USED_
  QColor m_rootColor;
  QColor m_terminalColor;
  QColor m_terminalFocusColor;
  QColor m_branchPointColor;
  QColor m_nodeColor;
  QColor m_planeSkeletonColor;

  QColor m_rootFocusColor;
  QColor m_branchPointFocusColor;
  QColor m_nodeFocusColor;
#endif
};

#define REGULAR_SWC_NODE_BEGIN(tn, start) \
  (Swc_Tree_Node_Is_Regular(tn) ? (start) : ((start) + 1))

template<class InputIterator>
void ZSwcTree::selectNode(
    InputIterator first, InputIterator last, bool appending)
{
  if (!appending) {
    deselectAllNode();
  }

  for (InputIterator it = first; it != last; ++it) {
    Swc_Tree_Node *tn = *it;
    selectNode(tn, true);
  }
}
#endif /* _ZSWCTREE_H_ */
