/**@file zstackdoc.h
 * @brief Stack document
 * @author Ting Zhao
 */
#ifndef _ZSTACKDOC_H_
#define _ZSTACKDOC_H_

#include <QString>
#include <QList>
#include <QUrl>
#include <set>
#include <QObject>
#include <QUndoCommand>
#include <QMap>
#include <string>
#include <QMenu>
#include <QPair>
#include <QMap>
#include <QMutex>
#include <QSet>
#include <QStack>

#include "neutube.h"
#include "zcurve.h"
#include "zobject3d.h"
#include "zswctree.h"
#include "tz_local_neuroseg.h"
#include "tz_locseg_chain.h"
#include "tz_trace_defs.h"
#include "zpunctum.h"
#include "zprogressreporter.h"
#include "dialogs/zrescaleswcdialog.h"
#include "zreportable.h"
#include "biocytin/zstackprojector.h"
#include "zstackreadthread.h"
#include "zstackfile.h"
#include "zactionactivator.h"
#include "dialogs/resolutiondialog.h"
#include "zneurontracer.h"
#include "zdocplayer.h"
#include "z3dgraph.h"
#include "zcubearray.h"
#include "zstackobjectgroup.h"
#include "tz_error.h"
#include "misc/miscutility.h"
#include "zrect2d.h"
#include "zobjectcolorscheme.h"
#include "zthreadfuturemap.h"
#include "zsharedpointer.h"
#include "zactionfactory.h"

class ZStackFrame;
class ZLocalNeuroseg;
class ZStackObject;
class ZLocsegChain;
class ZLocsegChainConn;
class QXmlStreamReader;
class QProgressBar;
class ZStack;
class ZResolution;
class ZSwcNetwork;
class ZSwcObjsModel;
class ZPunctaObjsModel;
class ZStroke2d;
class QWidget;
class ZSwcNodeObjsModel;
class ZDocPlayerObjsModel;
class ZGraphObjsModel;
class ZSurfaceObjsModel;
class ZStackDocReader;
class ZStackFactory;
class ZSparseObject;
class ZSparseStack;
class ZStackBall;
class ZUndoCommand;
class ZStackPatch;
class ZStackDocReader;
class ZDvidTileEnsemble;
class ZStackViewParam;
class Z3DWindow;
class ZStackMvc;
class ZProgressSignal;
class ZWidgetMessage;
class ZDvidSparseStack;

/*!
 * \brief The class of stack document
 *
 * Each document has at most one main stack, which defines some context of other
 * data (e.g. resolution).
 */
class ZStackDoc : public QObject, public ZReportable, public ZProgressable
{
  Q_OBJECT

public:
  ZStackDoc(QObject *parent = NULL);
  virtual ~ZStackDoc();

  //Designed for multi-thread reading
  enum TubeImportOption {
    ALL_TUBE,
    GOOD_TUBE,
    BAD_TUBE
  };

  enum LoadObjectOption {
    REPLACE_OBJECT,
    APPEND_OBJECT
  };

  enum EComponent {
    STACK, STACK_MASK, STACK_SEGMENTATION, SEGMENTATION_OBJECT,
    SEGMENTATION_GRAPH, SEGMENTATION_INDEX_MAP, SPARSE_STACK
  };

  enum EDocumentDataType {
    SWC_DATA, PUNCTA_DATA, STACK_DATA, NETWORK_DATA
  };

  enum EObjectModifiedMode {
    OBJECT_MODIFIED_SLIENT, OBJECT_MODIFIED_SIGNAL, OBJECT_MODIFIED_CACHE
  };

public: //attributes
  // isEmpty() returns true iff it has no stack data or object.
  bool isEmpty();

  /*!
   * \brief Test if the document has dense stack data
   * \return true iff it has stack array data.
   */
  bool hasStackData() const;

  /*!
   * \brief Test if there is any stack (including virtual)
   */
  bool hasStack() const;

  /*!
   * \brief hasStackPaint
   * \return true iff the document has any stack data to paint
   */
  bool hasStackPaint() const;


  bool hasStackMask();

  // hasTracable() returns true iff it has tracable data.
  bool hasTracable();

  // hasObject() returns true iff it has an object.
  bool hasObject() const;

  bool hasObject(ZStackObjectRole::TRole role) const;
  bool hasObject(ZStackObject::EType type) const;
  bool hasObject(ZStackObject::EType type, const std::string &source) const;

  ZStackObject* getObject(ZStackObject::EType type, const std::string &source) const;

  template <typename T>
  T* getObject(const std::string &source) const;

  // hasSwc() returns true iff it has an SWC object.
  bool hasSwc() const;
  bool hasPuncta() const;
  // hasDrawable() returns true iff it has a drawable object.
  bool hasDrawable() const;
  bool hasDrawable(ZStackObject::ETarget target) const;
  bool hasSparseObject() const;

  virtual bool hasSparseStack() const;
  virtual bool hasVisibleSparseStack() const;

  bool hasSelectedSwc() const;
  bool hasSelectedSwcNode() const;
  int getSelectedSwcNodeNumber() const;
  bool hasMultipleSelectedSwcNode() const;

  int getStackWidth() const;
  int getStackHeight() const;
  int getStackDepth() const;
  int stackChannelNumber() const;

  virtual void deprecateDependent(EComponent component);
  virtual void deprecate(EComponent component);
  virtual bool isDeprecated(EComponent component);

  virtual void clearData();

  /*!
   * \brief The offset from stack space to data space
   */
  ZIntPoint getStackOffset() const;
  void setStackOffset(int x, int y, int z);
  void setStackOffset(const ZIntPoint &offset);
  void setStackOffset(const ZPoint &offset);

  ZIntPoint getStackSize() const;

  inline const ZResolution& getResolution() const {
    return m_resolution;
  }

  void setResolution(double x, double y, double z, char unit);

  double getPreferredZScale() const;

  void setResolution(const Cz_Lsminfo &lsmInfo);

  /*!
   * \brief Get the data space coordinates of stack coordinates
   */
  ZIntPoint getDataCoord(const ZIntPoint &pt);
  ZIntPoint getDataCoord(int x, int y, int z);

  /*!
   * \brief Map stack coodinates to data space
   */
  void mapToDataCoord(ZPoint *pt);
  void mapToDataCoord(double *x, double *y, double *z);

  /*!
   * \brief Data coordinates to stack coordinates
   */
  void mapToStackCoord(ZPoint *pt);
  void mapToStackCoord(double *x, double *y, double *z);

  ZSharedPointer<ZStackDoc> getParentDoc() const {
    return m_parentDoc;
  }
  virtual void setParentDoc(ZSharedPointer<ZStackDoc> parentDoc);

  // Prefix for tracing project.
  const char *tubePrefix() const;

  inline QList<ZStackObject*> *drawableList() {
    return &(m_objectGroup.getObjectList());
  }

//  inline QList<ZSwcTree*>* swcList();

  QList<ZSwcTree*> getSwcList() const;
  QList<ZObject3d*> getObj3dList() const;
  QList<ZStroke2d*> getStrokeList() const;
  QList<ZLocsegChain*> getLocsegChainList() const;
  QList<ZPunctum*> getPunctumList() const;
  QList<ZSparseObject*> getSparseObjectList() const;
  QList<ZObject3dScan*> getObject3dScanList() const;
  QList<ZDvidLabelSlice*> getDvidLabelSliceList() const;
  QList<ZDvidTileEnsemble*> getDvidTileEnsembleList() const;
  QList<ZDvidSparsevolSlice*> getDvidSparsevolSliceList() const;
  virtual ZDvidSparseStack* getDvidSparseStack() const;

  bool hasSwcList();       //to test swctree
  //inline QList<ZLocsegChain*>* chainList() {return &m_chainList;}
  //inline QList<ZPunctum*>* punctaList() {return &m_punctaList;}
  inline ZSwcObjsModel* swcObjsModel() {return m_swcObjsModel;}
  inline ZDocPlayerObjsModel* seedObjsModel() { return m_seedObjsModel; }
  inline ZSwcNodeObjsModel* swcNodeObjsModel() {return m_swcNodeObjsModel;}
  inline ZPunctaObjsModel* punctaObjsModel() {return m_punctaObjsModel;}
  inline ZGraphObjsModel* graphObjsModel() { return m_graphObjsModel; }
  inline ZSurfaceObjsModel* surfaceObjsModel() { return m_surfaceObjsModel; }

  void updatePunctaObjsModel(ZPunctum *punctum);

//  std::set<Swc_Tree_Node*> getSelectedSwcTreeNodeSet() const;

  std::set<Swc_Tree_Node*> getSelectedSwcNodeSet() const;
  std::set<Swc_Tree_Node*> getUnselectedSwcNodeSet() const;

  static QList<Swc_Tree_Node*> getSelectedSwcNodeList(
      const ZSwcTree *tree);
  QList<Swc_Tree_Node*> getSelectedSwcNodeList() const;

  QMap<const Swc_Tree_Node*, const ZSwcTree*> getSelectedSwcNodeMap() const;


  inline ZSwcNetwork* swcNetwork() { return m_swcNetwork; }
  ZResolution stackResolution() const;
  std::string stackSourcePath() const;
  bool hasChainList();

  //void setStackMask(ZStack *stack);

//  void createActions();
  QAction* getAction(ZActionFactory::EAction item) const;
  virtual void makeAction(ZActionFactory::EAction item);

  void updateSwcNodeAction();

  void addData(ZStackDocReader &reader);

  bool isUndoClean() const;
  bool isSwcSavingRequired() const;
  bool isSavingRequired() const;
  void setSaved(ZStackObject::EType type, bool state);

  bool isSaved(ZStackObject::EType type) const;

  const ZUndoCommand* getLastUndoCommand() const;
  //const ZUndoCommand* getLastCommand() const;

public: //swc tree edit
  // move soma (first root) to new location
  void swcTreeTranslateRootTo(double x, double y, double z);
  // rescale location and radius
  void swcTreeRescale(double scaleX, double scaleY, double scaleZ);
  void swcTreeRescale(double srcPixelPerUmXY, double srcPixelPerUmZ, double dstPixelPerUmXY, double dstPixelPerUmZ);
  // rescale radius of nodes of certain depth
  void swcTreeRescaleRadius(double scale, int startdepth, int enddepth);
  // reduce node number, similar to Swc_Tree_Merge_Close_Node, but only merge Continuation node
  void swcTreeReduceNodeNumber(double lengthThre);
  void updateVirtualStackSize();

//  void deleteSelectedSwcNode();
  void addSizeForSelectedSwcNode(double dr);

  void estimateSwcRadius(ZSwcTree *tree, int maxIter = 1);
  void estimateSwcRadius();

public: //swc selection
  void selectSwcNodeNeighbor();
  std::string getSwcSource() const;

public:
  void loadFileList(const QList<QUrl> &urlList);
  void loadFileList(const QStringList &filePath);

  /*!
   * \brief Load data from a file into the document.
   *
   * \param filePath Path of the data file.
   * \return true iff the file is loaded successfully.
   */
  bool loadFile(const char *filePath);
  bool loadFile(const std::string filePath);
  bool loadFile(const QString &filePath);

  virtual void loadStack(Stack *stack, bool isOwner = true);
  virtual void loadStack(ZStack *zstack);

  /*!
   * \brief The reference of the main stack variable
   */
  virtual ZStack*& stackRef();
  virtual const ZStack *stackRef() const;

  void readStack(const char *filePath, bool newThread = true);
  void readSwc(const char *filePath);

  void saveSwc(QWidget *parentWidget);

  const ZStackFrame *getParentFrame() const;
  ZStackFrame* getParentFrame();

  const Z3DWindow *getParent3DWindow() const;
  Z3DWindow* getParent3DWindow();

  const ZStackMvc *getParentMvc() const;
  ZStackMvc* getParentMvc();

  template<typename T>
  QList<T*> getUserList() const;

//  void setParentFrame(ZStackFrame* parent);

  virtual ZStack* getStack() const;
  virtual ZStack *stackMask() const;
  void setStackSource(const char *filePath);
  void setStackSource(const ZStackFile &stackFile);
  void loadSwcNetwork(const QString &filePath);
  void loadSwcNetwork(const char *filePath);
  bool importImageSequence(const char *filePath);

  void loadSwc(const QString &filePath);
  void loadLocsegChain(const QString &filePath);

  void importFlyEmNetwork(const char *filePath);

  //void exportVrml(const char *filePath);
  void exportSvg(const char *filePath);
  void exportBinary(const char *prefix);
  void exportSwcTree(const char *filePath);
  void exportChainFileList(const char *filepath);
  void exportPuncta(const char *filePath);
  void exportObjectMask(const std::string &filePath);

  //Those functions do not notify object modification
  //void removeLastObject(bool deleteObject = false);
  void removeAllObject(bool deleteObject = true);
  void removeObject(
      ZStackObject *obj, bool deleteObject = false);

  void removeSelectedObject(bool deleteObject = false);
  /*
  void removeObject(
      ZStackObject::ETarget target, bool deleteObject = false);
      */
  void removeObject(
      ZStackObject::EType type, bool deleteObject = false);

  TStackObjectList takeObject(
      ZStackObject::EType type, const std::string &source);
  TStackObjectList takeObject(ZStackObject::EType type);


  /* Remove object with specific roles */
  void removeObject(ZStackObjectRole::TRole role, bool deleteObject = false);

  void removeObject(const std::string &source, bool deleteObject = false);

  void removeSelectedPuncta(bool deleteObject = false);
  void removeSmallLocsegChain(double thre);   //remove small locseg chain (geolen < thre)
  //void removeAllLocsegChain();
  //void removeAllObj3d();
  //void removeAllSparseObject();
  std::set<ZSwcTree*> removeEmptySwcTree(bool deleteObject = true);
  std::set<ZSwcTree*> getEmptySwcTreeSet() const;

  void removeAllSwcTree(bool deleteObject = true);

  //void addObject(ZStackObject *obj);
  void appendSwcNetwork(ZSwcNetwork &network);

  //QString toString();
  QStringList toStringList() const;
  virtual QString rawDataInfo(double cx, double cy, int z) const;
  QString getTitle() const;

  ZCurve locsegProfileCurve(int option) const;

  void cutLocsegChain(ZLocsegChain *obj, QList<ZLocsegChain*> *pResult = NULL);   //optional output cut result
  void cutSelectedLocsegChain();
  void breakLocsegChain(ZLocsegChain *obj, QList<ZLocsegChain*> *pResult = NULL);  //optional output break result
  void breakSelectedLocsegChain();

  int maxIntesityDepth(int x, int y);
  std::vector<ZStack*> projectBiocytinStack(
      Biocytin::ZStackProjector &projector);

  void updateStackFromSource();
  void setStackFactory(ZStackFactory *factory);

  void setSparseStack(ZSparseStack *spStack);

  void importSeedMask(const QString &filePath);

public: //Image processing
  static int autoThreshold(Stack* getStack);
  void autoThreshold();
  bool binarize(int threshold);
  bool bwsolid();
  bool enhanceLine();
  bool watershed();
  bool invert();
  bool subtractBackground();
  int findLoop(int minLoopSize = 100);
  void bwthin();
  bool bwperim();
  void runSeededWatershed();
  void runLocalSeededWatershed();

private:
  void localSeededWatershed();
  void seededWatershed();
  template <class InputIterator>
  void removeObjectP(InputIterator first, InputIterator last, bool deleting);

  void updateSwc();
  bool estimateSwcNodeRadius(Swc_Tree_Node *tn, int maxIter);

public: /* tracing routines */
  ZLocsegChain* fitseg(int x, int y, int z, double r = 3.0);
  ZLocsegChain* fitRpiseg(int x, int y, int z, double r = 3.0);
  ZLocsegChain* fitRect(int x, int y, int z, double r = 3.0);
  ZLocsegChain* fitEllipse(int x, int y, int z, double r = 1.0);
  ZLocsegChain* dropseg(int x, int y, int z, double r = 3.0);
  ZLocsegChain* traceTube(int x, int y, int z, double r = 3.0, int c = 0);
  ZLocsegChain* traceRect(int x, int y, int z, double r = 3.0, int c = 0);

  void refreshTraceMask();

public: /* puncta related methods */
  ZPunctum* markPunctum(int x, int y, int z, double r = 2.0);
  int pickPunctaIndex(int x, int y, int z) const;
  bool selectPuncta(int index);
  bool deleteAllPuncta();
  bool expandSelectedPuncta();
  bool shrinkSelectedPuncta();
  bool meanshiftSelectedPuncta();
  bool meanshiftAllPuncta();
  inline bool hasSelectedPuncta() {
    return m_objectGroup.hasSelected(ZStackObject::TYPE_PUNCTUM);
  }

public:
  void addLocsegChainP(ZLocsegChain *chain);
  void addLocsegChain(const QList<ZLocsegChain*> &chainList);

  void addSwcTreeP(ZSwcTree *obj);

  //void addSwcTree(ZSwcTree *obj, bool uniqueSource = true);
  void addSwcTree(ZSwcTree *obj, bool uniqueSource, bool translatingWithStack);
  void addSwcTree(const QList<ZSwcTree*> &swcList, bool uniqueSource = true);
  void addSparseObject(const QList<ZSparseObject*> &objList);
  void addPunctumP(ZPunctum *obj);
  void addPunctum(const QList<ZPunctum*> &punctaList);

  void addPunctumFast(const QList<ZPunctum*> &punctaList);


  void addObj3dP(ZObject3d *obj);
  void addObject3dScanP(ZObject3dScan *obj);
  void addStackPatchP(ZStackPatch *patch, bool uniqueSource = true);
  void addStrokeP(ZStroke2d *obj);
  void addSparseObjectP(ZSparseObject *obj);

  /*!
   * \brief Add an object in a quick way
   *
   * The function assumes that \a obj has no source and it does not exist in
   * the document. This function is useful for adding a large number of newly
   * created obejcts.
   */
  void addObjectFast(ZStackObject *obj);

  template <typename InputIterator>
  void addObjectFast(InputIterator first, InputIterator last);

  /*!
   * \brief Add a palyer
   *
   * Nothing will be done if \a role is ZDocPlayer::ROLE_NONE.
   */
//  void addPlayer(ZStackObject *obj, NeuTube::EDocumentableType type,
//                 ZDocPlayer::TRole role);
  void addPlayer(ZStackObject *obj);

  void toggleVisibility(ZStackObjectRole::TRole role);

  void updateLocsegChain(ZLocsegChain *chain);
  void importLocsegChain(const QStringList &files,
                         TubeImportOption option = ALL_TUBE,
                         LoadObjectOption objopt = APPEND_OBJECT);
  void importSwc(QStringList files, LoadObjectOption objopt = APPEND_OBJECT);
  void importPuncta(const QStringList &files,
                    LoadObjectOption objopt = APPEND_OBJECT);

  bool importPuncta(const char *filePath);

  int pickLocsegChainId(int x, int y, int z) const;
  void holdClosestSeg(int id, int x, int y, int z);
  int selectLocsegChain(int id, int x = -1, int y = -1, int z = -1,
  		bool showProfile = false);
  bool selectSwcTreeBranch(int x, int y, int z);
  bool pushLocsegChain(ZStackObject *obj);
  void pushSelectedLocsegChain();

  void showSwcFullSkeleton(bool state);

//  enum ESynapseSelection {
//    SYNAPSE_ALL, SYNAPSE_TBAR, SYNAPSE_PSD
//  };

  /*!
   * \brief importSynapseAnnotation
   * \param filePath
   * \param s 0: SYNAPSE_ALL; 1: SYNAPSE_TBAR; 2: SYNAPSE_PSD
   * \return
   */
  bool importSynapseAnnotation(const std::string &filePath,
                               int s = 0);

  ZStackObject *hitTest(double x, double y, double z);
  ZStackObject *hitTest(double x, double y, NeuTube::EAxis sliceAxis);

//  Swc_Tree_Node *swcHitTest(double x, double y) const;
//  Swc_Tree_Node *swcHitTest(double x, double y, double z) const;
//  Swc_Tree_Node *swcHitTest(const ZPoint &pt) const;
  Swc_Tree_Node *selectSwcTreeNode(int x, int y, int z, bool append = false);
  Swc_Tree_Node *selectSwcTreeNode(const ZPoint &pt, bool append = false);

  /*!
   * \brief Selecte swc tree node
   *
   * \a tn must be a part of \a tree if is not NULL. The function does not check
   * the validity.
   */
  void selectSwcTreeNode(ZSwcTree *tree, Swc_Tree_Node *tn, bool append = false);
  void deselectSwcTreeNode(ZSwcTree *tree, Swc_Tree_Node *tn);

  void selectSwcTreeNode(Swc_Tree_Node *tn, bool append = false);
  void deselectSwcTreeNode(Swc_Tree_Node *tn);


  void selectHitSwcTreeNode(ZSwcTree *tree, bool append = false);
  void deselectHitSwcTreeNode(ZSwcTree *tree);

  void selectHitSwcTreeNodeConnection(ZSwcTree *tree);
  void selectHitSwcTreeNodeFloodFilling(ZSwcTree *tree);

  // (de)select objects and emit signals for 3D view and 2D view to sync
  void setPunctumSelected(ZPunctum* punctum, bool select);
  template <class InputIterator>
  void setPunctumSelected(InputIterator first, InputIterator last, bool select);
  void deselectAllPuncta();
  void setChainSelected(ZLocsegChain* chain, bool select);
  void setChainSelected(const std::vector<ZLocsegChain*> &chains, bool select);
  void deselectAllChains();
  //void deselectAllStroke();
  void setSwcSelected(ZSwcTree* tree, bool select);
  template <class InputIterator>
  void setSwcSelected(InputIterator first, InputIterator last, bool select);
  void deselectAllSwcs();
  void setSwcTreeNodeSelected(Swc_Tree_Node* tn, bool select);
  template <class InputIterator>
  void setSwcTreeNodeSelected(InputIterator first, InputIterator last, bool select);
  void deselectAllSwcTreeNodes();
  void deselectAllObject(bool recursive = true);
  void deselectAllObject(ZStackObject::EType type);

  bool isSwcNodeSelected(const Swc_Tree_Node *tn) const;

  // show/hide objects and emit signals for 3D view and 2D view to sync
  void setPunctumVisible(ZPunctum* punctum, bool visible);
  void setGraphVisible(Z3DGraph *graph, bool visible);
  void setChainVisible(ZLocsegChain* chain, bool visible);
  void setSwcVisible(ZSwcTree* tree, bool visible);
  void setSurfaceVisible(ZCubeArray *cubearray, bool visible);

  void setAutoTraceMinScore(double score);
  void setManualTraceMinScore(double score);
  void setReceptor(int option, bool cone = false);

  //void updateMasterLocsegChain();
  //bool linkChain(int id);
  //bool hookChain(int id, int option = 0);
  bool mergeChain(int id);
  bool connectChain(int id);
  bool disconnectChain(int id);
  void eraseTraceMask(const ZLocsegChain *chain);
  void mergeAllChain();

  void setWorkdir(const QString &filePath);
  void setWorkdir(const char *filePath);
  void setTubePrefix(const char *filePath);

  void test(QProgressBar *pb = NULL);

  inline QUndoStack* undoStack() const { return m_undoStack; }
  void pushUndoCommand(QUndoCommand *command);
  void pushUndoCommand(ZUndoCommand *command);

  inline std::string additionalSource() { return m_additionalSource; }
  inline void setAdditionalSource(const std::string &filePath) {
    m_additionalSource = filePath;
  }

  bool hasObjectSelected() const;

  inline const ZStackObjectGroup& getObjectGroup() const {
    return m_objectGroup; }
  inline ZStackObjectGroup& getObjectGroup() {
    return m_objectGroup; }

  inline const TStackObjectList& getObjectList(ZStackObject::EType type)
  const
  {
    return m_objectGroup.getObjectList(type);
  }

  inline TStackObjectList& getObjectList(ZStackObject::EType type) {
    return m_objectGroup.getObjectList(type);
  }

  QList<ZStackObject*> getObjectList(ZStackObjectRole::TRole role) const;

  template<typename T>
  QList<T*> getObjectList() const;

  inline const ZDocPlayerList& getPlayerList() const {
    return m_playerList;
  }

  inline ZDocPlayerList& getPlayerList() {
    return m_playerList;
  }

  QList<const ZDocPlayer*> getPlayerList(ZStackObjectRole::TRole role) const;
  QList<ZDocPlayer*> getPlayerList(ZStackObjectRole::TRole role);

  virtual const ZSparseStack* getSparseStack() const;
  virtual const ZSparseStack* getConstSparseStack() const;
  virtual ZSparseStack* getSparseStack();
  virtual ZObject3dScan* getSparseStackMask() const;

//  QSet<ZStackObject::ETarget>
//  updateActiveViewObject(const ZStackViewParam &param);

  bool hasPlayer(ZStackObjectRole::TRole role) const;

  Z3DGraph get3DGraphDecoration() const;

  void updateModelData(EDocumentDataType type);

  /*!
   * \brief Get all swc trees from the document in a single tree
   *
   * \return The user is responsible of freeing the returned object.
   */
  ZSwcTree *getMergedSwc();

  ZSwcTree* getSwcTree(size_t index);
  std::vector<ZSwcTree*> getSwcArray() const;
  bool getLastStrokePoint(int *x, int *y) const;
  bool getLastStrokePoint(double *x, double *y) const;

  void setSelected(ZStackObject *obj,  bool selecting = true);
  void toggleSelected(ZStackObject *obj);
  void selectObject(ZStackObject *obj, bool appending);

  const TStackObjectSet& getSelected(ZStackObject::EType type) const;
  TStackObjectSet &getSelected(ZStackObject::EType type);

  bool hasSelectedObject() const;

  void setVisible(ZStackObject::EType type, bool visible);
  void setVisible(ZStackObjectRole::TRole role, bool visible);
  void setVisible(ZStackObject::EType type, std::string source, bool visible);

  template <typename T>
  QList<T*> getSelectedObjectList() const;

  template <typename T>
  QList<T*> getSelectedObjectList(ZStackObject::EType type) const;

  template <typename T>
  std::set<T*> getSelectedObjectSet(ZStackObject::EType type) const;

  void clearSelectedSet();

  ZRect2d getRect2dRoi() const;
  ZIntCuboid getCuboidRoi() const;

  virtual void selectSwcNode(const ZRect2d &roi);

  void setStackBc(double factor, double offset, int channel);

public:
  inline NeuTube::Document::ETag getTag() const { return m_tag; }
  inline void setTag(NeuTube::Document::ETag tag) { m_tag = tag; }
  void setStackBackground(NeuTube::EImageBackground bg);
  inline NeuTube::EImageBackground getStackBackground() const {
    return m_stackBackground;
  }

  inline void setSelectionSlient(bool slient) {
    m_selectionSilent = slient;
  }

  inline bool isSelectionSlient() {
    return m_selectionSilent;
  }

  inline bool isReadyForPaint() const {
    return m_isReadyForPaint;
  }

  inline void setReadyForPaint(bool ready) {
    m_isReadyForPaint = ready;
  }

  void registerUser(QObject *user);

  inline bool isSegmentationReady() const {
    return m_isSegmentationReady;
  }

  inline void setSegmentationReady(bool state) {
    m_isSegmentationReady = state;
  }

public:
  inline void deprecateTraceMask() { m_isTraceMaskObsolete = true; }
  void updateTraceWorkspace(int traceEffort, bool traceMasked,
                            double xRes, double yRes, double zRes);
  void updateConnectionTestWorkspace(double xRes, double yRes, double zRes,
                                     char unit, double distThre, bool spTest,
                                     bool crossoverTest);

  inline Trace_Workspace* getTraceWorkspace() const {
    return m_neuronTracer.getTraceWorkspace();
  }

  inline Connection_Test_Workspace* getConnectionTestWorkspace() const {
    return m_neuronTracer.getConnectionTestWorkspace();
  }

  void disconnectSwcNodeModelUpdate();
  void disconnectPunctaModelUpdate();
  /*
  inline ZSwcTree* previewSwc() { return m_previewSwc; }
  void updatePreviewSwc();
  */

  void clearObjectModifiedTypeBuffer(bool sync = true);
  void clearObjectModifiedTargetBuffer(bool sync = true);
  void clearObjectModifiedRoleBuffer(bool sync = true);

  EObjectModifiedMode getObjectModifiedMode();
  void beginObjectModifiedMode(EObjectModifiedMode mode);
  void endObjectModifiedMode();

  void notifyObjectModified(bool sync = true);
  void notifyObjectModified(ZStackObject::EType type);

  void bufferObjectModified(ZStackObject::EType type, bool sync = true);
  void bufferObjectModified(ZStackObject::ETarget target, bool sync = true);
  void bufferObjectModified(const QSet<ZStackObject::EType> &typeSet,
                            bool sync = true);
  void bufferObjectModified(const QSet<ZStackObject::ETarget> &targetSet,
                            bool sync = true);
  void bufferObjectModified(ZStackObject *obj, bool sync = true);
  void bufferObjectModified(const ZStackObjectRole &role, bool sync = true);
  void bufferObjectModified(ZStackObjectRole::TRole role, bool sync = true);


  void processObjectModified(ZStackObject::EType type, bool sync = true);
  void processObjectModified(ZStackObject::ETarget target, bool sync = true);
  void processObjectModified(const QSet<ZStackObject::EType> &typeSet,
                             bool sync = true);
  void processObjectModified(const QSet<ZStackObject::ETarget> &targetSet,
                             bool sync = true);
  void processObjectModified(ZStackObject *obj, bool sync = true);
  void processObjectModified(ZStackObjectRole::TRole role, bool sync = true);
  void processObjectModified(const ZStackObjectRole &role, bool sync = true);

  void processSwcModified();

  /*!
   * \brief Notify any connect slots about the modification of SWC objects
   *
   * It explicitly deprecate all intermediate components of all the SWC objects
   */
  void notifySwcModified();

  void notifyPunctumModified();
  void notifyChainModified();
  void notifyObj3dModified();
  void notifyObject3dScanModified();
  void notifyStackPatchModified();
  void notifySparseObjectModified();
  void notifyStackModified();
  void notifySparseStackModified();
  void notifyVolumeModified();
  void notifyStrokeModified();
  //void notifyAllObjectModified();
  void notify3DGraphModified();
  void notify3DCubeModified();
  void notifyTodoModified();
  void notifyActiveViewModified();
  void notifyStatusMessageUpdated(const QString &message);

  void notifyProgressStart();
  void notifyProgressEnd();
  void notifyProgressAdvanced(double dp);

  void recordSwcTreeNodeSelection();

  void notifySelectorChanged();
  void notifySwcTreeNodeSelectionChanged();

  template <typename T>
  void notifySelectionAdded(const std::set<T*> &oldSelected,
                            const std::set<T*> &newSelected);
  template <typename T>
  void notifySelectionRemoved(const std::set<T*> &oldSelected,
                              const std::set<T*> &newSelected);

  template <typename T>
  void notifySelected(const QList<T*> &selected);

  template <typename T>
  void notifyDeselected(const QList<T*> &deselected);

#define DECLARE_NOTIFY_SELECTION_CHANGED(Type)\
  void notifySelectionChanged(const QList<Type *> &selected,\
                              const QList<Type *> &deselected)

  DECLARE_NOTIFY_SELECTION_CHANGED(Swc_Tree_Node);
  DECLARE_NOTIFY_SELECTION_CHANGED(ZSwcTree);
  DECLARE_NOTIFY_SELECTION_CHANGED(ZPunctum);
  DECLARE_NOTIFY_SELECTION_CHANGED(ZLocsegChain);
  DECLARE_NOTIFY_SELECTION_CHANGED(ZStackObject);

  void notifySelectionChanged(const std::set<ZStackObject*> &selected,
                              const std::set<ZStackObject*> &deselected);
  void notifySelectionChanged(const std::set<const ZStackObject*> &selected,
                              const std::set<const ZStackObject*> &deselected);

  void notify(const ZWidgetMessage &msg);
  void notify(const QString &msg);

  void notifyUpdateLatency(int64_t t);

public:
//  inline QAction* getUndoAction() { return m_undoAction; }
//  inline QAction* getRedoAction() { return m_redoAction; }

  ZSingleSwcNodeActionActivator* getSingleSwcNodeActionActivator()  {
    return &m_singleSwcNodeActionActivator;
  }

  inline const ZStack* getLabelField() const {
    return m_labelField;
  }

  ZStack* getLabelField() {
    return m_labelField;
  }

  void setLabelField(ZStack *getStack);

  ZStack* makeLabelStack(ZStack *stack = NULL) const;

  void notifyPlayerChanged(const ZStackObjectRole &role);
  void notifyPlayerChanged(ZStackObjectRole::TRole role);

  ZProgressSignal* getProgressSignal() const {
    return m_progressSignal;
  }

  virtual void processRectRoiUpdate(ZRect2d *rect, bool appending);
  /*
  inline void setLastAddedSwcNode(Swc_Tree_Node *tn) {
    m_lastAddedSwcNode = tn;
  }

  inline Swc_Tree_Node* getLastAddedSwcNode() const {
    return m_lastAddedSwcNode;
  }*/

  void enableAutoSaving(bool on) { m_autoSaving = on; }

  class ActiveViewObjectUpdater {
  public:
    ActiveViewObjectUpdater() {}
    ActiveViewObjectUpdater(const ZSharedPointer<ZStackDoc> &doc) {
      m_doc = doc; }
    void exclude(ZStackObject::EType type) {
      m_excludeSet.insert(type);
    }
    void exclude(ZStackObject::ETarget target) {
      m_excludeTarget.insert(target);
    }

    void clearState();

    void update(const ZStackViewParam &param);
    const QSet<ZStackObject::ETarget>& getUpdatedTargetSet() {
      return m_updatedTarget;
    }

    void setDocument(const ZSharedPointer<ZStackDoc> &doc) {
      m_doc = doc;
    }

    static void SetUpdateEnabled(
        ZSharedPointer<ZStackDoc> doc, ZStackObject::EType type, bool on);

  private:
    ZSharedPointer<ZStackDoc> m_doc;
    QSet<ZStackObject::EType> m_excludeSet;
    QSet<ZStackObject::ETarget> m_excludeTarget;
    QSet<ZStackObject::ETarget> m_updatedTarget;
  };

public slots: //undoable commands
  /*!
   * \brief Add an object
   * \param obj
   * \param role
   * \param uniqueSource Replace the object with the same nonempty source if it
   *        is true. Note that if there are multiple objects with the same source
   *        existing in the doc, only the first one is replaced.
   */
  void addObject(ZStackObject *obj, bool uniqueSource = true);
  void addObject(ZStackObject *obj, int zOrder, bool uniqueSource);

  virtual bool executeAddObjectCommand(ZStackObject *obj,
                               bool uniqueSource = true);
  virtual bool executeRemoveObjectCommand(ZStackObject *obj);
  virtual bool executeRemoveSelectedObjectCommand();
  //bool executeRemoveUnselectedObjectCommand();
  virtual bool executeMoveObjectCommand(
      double x, double y, double z,
      double punctaScaleX, double punctaScaleY, double punctaScaleZ,
      double swcScaleX, double swcScaleY, double swcScaleZ);

  virtual bool executeTraceTubeCommand(double x, double y, double z, int c = 0);
  virtual bool executeRemoveTubeCommand();
  virtual bool executeAutoTraceCommand(int traceLevel, bool doResample);
  virtual bool executeAutoTraceAxonCommand();

  virtual bool executeAddSwcBranchCommand(ZSwcTree *tree, double minConnDist);
  virtual bool executeAddSwcCommand(ZSwcTree *tree);
  virtual bool executeReplaceSwcCommand(ZSwcTree *tree);
  virtual void executeSwcRescaleCommand(const ZRescaleSwcSetting &setting);

  virtual bool executeSwcNodeExtendCommand(const ZPoint &center);
  virtual bool executeSwcNodeExtendCommand(const ZPoint &center, double radius);
  virtual bool executeSwcNodeSmartExtendCommand(const ZPoint &center);
  virtual bool executeSwcNodeSmartExtendCommand(const ZPoint &center, double radius);
  virtual bool executeSwcNodeChangeZCommand(double z);
  virtual bool executeSwcNodeEstimateRadiusCommand();
  virtual bool executeMoveSwcNodeCommand(double dx, double dy, double dz);
  virtual bool executeScaleSwcNodeCommand(
      double sx, double sy, double sz, const ZPoint &center);
  virtual bool executeRotateSwcNodeCommand(double theta, double psi, bool aroundCenter);
  virtual bool executeTranslateSelectedSwcNode();
  virtual bool executeDeleteSwcNodeCommand();
  virtual bool executeDeleteUnselectedSwcNodeCommand();
  virtual bool executeConnectSwcNodeCommand();
  virtual bool executeChangeSelectedSwcNodeSize();
  virtual bool executeConnectSwcNodeCommand(Swc_Tree_Node *tn);
  virtual bool executeConnectSwcNodeCommand(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
  virtual bool executeSmartConnectSwcNodeCommand(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
  virtual bool executeSmartConnectSwcNodeCommand();
  virtual bool executeBreakSwcConnectionCommand();
  virtual bool executeAddSwcNodeCommand(const ZPoint &center, double radius,
                                        ZStackObjectRole::TRole role);
  virtual bool executeSwcNodeChangeSizeCommand(double dr);
  virtual bool executeMergeSwcNodeCommand();
  virtual bool executeTraceSwcBranchCommand(double x, double y, double z);
  virtual bool executeTraceSwcBranchCommand(double x, double y);
  virtual bool executeInterpolateSwcZCommand();
  virtual bool executeInterpolateSwcRadiusCommand();
  virtual bool executeInterpolateSwcPositionCommand();
  virtual bool executeInterpolateSwcCommand();
  virtual bool executeBreakForestCommand();
  virtual bool executeGroupSwcCommand();
  virtual bool executeSetRootCommand();
  virtual bool executeRemoveTurnCommand();
  virtual bool executeResolveCrossoverCommand();
  virtual bool executeInsertSwcNode();
  virtual bool executeSetBranchPoint();
  virtual bool executeConnectIsolatedSwc();
  virtual bool executeResetBranchPoint();

  virtual bool executeMoveAllSwcCommand(double dx, double dy, double dz);
  virtual bool executeScaleAllSwcCommand(double sx, double sy, double sz,
                                 bool aroundCenter = false);
  virtual bool executeRotateAllSwcCommand(
      double theta, double psi, bool aroundCenter = false);

  virtual bool executeBinarizeCommand(int thre);
  virtual bool executeBwsolidCommand();
  virtual bool executeEnhanceLineCommand();
  virtual bool executeWatershedCommand();
  virtual void executeRemoveRectRoiCommand();

  void advanceProgressSlot(double dp);
  void startProgressSlot();
  void endProgressSlot();

  //bool executeAddStrokeCommand(ZStroke2d *stroke);
  //bool executeAddStrokeCommand(const QList<ZStroke2d*> &strokeList);

public slots:
  void selectAllSwcTreeNode();
  void autoSaveSlot();
  bool saveSwc(const std::string &filePath);
  void loadReaderResult();
  void selectDownstreamNode();
  void selectSwcNodeConnection(Swc_Tree_Node *lastSelectedNode = NULL);
  void selectSwcNodeFloodFilling(Swc_Tree_Node *lastSelectedNode);
  void selectUpstreamNode();
  void selectBranchNode();
  void selectTreeNode();
  void selectConnectedNode();
  void inverseSwcNodeSelection();
  void selectNoisyTrees();

  /*!
   * \brief Select neighboring swc nodes.
   *
   * Add the neighbors of the current selected nodes into the selection set.
   */
  void selectNeighborSwcNode();

  void showSeletedSwcNodeLength(double *resolution = NULL);
  void showSeletedSwcNodeScaledLength();
  void showSwcSummary();

  void setPunctaVisible(bool visible);
  void hideSelectedPuncta();
  void showSelectedPuncta();

  void emptySlot();

  void reloadStack();

  void reloadData(ZStackDocReader &reader);

  void removeUser(QObject *user);
  void removeAllUser();

  void notifyZoomingToSelectedSwcNode();
  void notifyZoomingTo(double x, double y, double z);
  void notifyZoomingTo(const ZIntPoint &pt);

//  void processRectRoiUpdateSlot();

signals:
  void addingObject(ZStackObject *obj, bool uniqueSource = true);
  void messageGenerated(const QString &message, bool appending = true);
  void messageGenerated(const ZWidgetMessage&);
  void locsegChainSelected(ZLocsegChain*);
  void stackDelivered(Stack *stack, bool beOwner);
  void frameDelivered(ZStackFrame *frame);
  void stackModified();
  void sparseStackModified();
  void labelFieldModified();
  void stackReadDone();
  void stackLoaded();
  void punctaModified();
  void swcModified();
  void seedModified();
  void chainModified();
  void obj3dModified();
  void object3dScanModified();
  void stackPatchModified();
  void sparseObjectModified();
  void strokeModified();
  void graph3dModified();
  void cube3dModified();
  void todoModified();
  void objectModified();
  void objectModified(ZStackObject::ETarget);
  void objectModified(QSet<ZStackObject::ETarget>);

  void stackTargetModified();
  void swcNetworkModified();
  void activeViewModified();

  void objectSelectionChanged(QList<ZStackObject*> selected,
                              QList<ZStackObject*> deselected);
  void punctaSelectionChanged(QList<ZPunctum*> selected,
                              QList<ZPunctum*> deselected);
  void chainSelectionChanged(QList<ZLocsegChain*> selected,
                             QList<ZLocsegChain*> deselected);
  void swcSelectionChanged(QList<ZSwcTree*> selected,
                           QList<ZSwcTree*> deselected);
  void swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*> selected,
                                   QList<Swc_Tree_Node*> deselected);
  void objectSelectorChanged(ZStackObjectSelector selector);

  void swcTreeNodeSelectionChanged();

  void punctumVisibleStateChanged();
  void graphVisibleStateChanged();
  void surfaceVisibleStateChanged();
  void chainVisibleStateChanged(ZLocsegChain* chain, bool visible);
  void swcVisibleStateChanged(ZSwcTree* swctree, bool visible);
  void cleanChanged(bool);
  void holdSegChanged();
  void statusMessageUpdated(QString message) const;

  void thresholdChanged(int thre);

  /*!
   * \brief A signal indicating modification of volume rendering
   */
  void volumeModified();

  void progressStarted();
  void progressEnded();
  void progressAdvanced(double dp);
//  void newDocReady(const ZStackDocReader &reader);

  void zoomingToSelectedSwcNode();

  void zoomingTo(int x, int y, int z);
  void updatingLatency(int);

protected:
  virtual void autoSave();
  virtual void customNotifyObjectModified(ZStackObject::EType type);
  void removeRect2dRoi();
  virtual std::vector<ZStack*> createWatershedMask(bool selectedOnly) const;
  void updateWatershedBoundaryObject(ZStack *out, ZIntPoint dsIntv);

private:
  void init();

  void connectSignalSlot();
  void initNeuronTracer();
  //void initTraceWorkspace();
  //void initConnectionTestWorkspace();
  //void loadTraceMask(bool traceMasked);
  int xmlConnNode(QXmlStreamReader *xml, QString *filePath, int *spot);
  int xmlConnMode(QXmlStreamReader *xml);
  ZSwcTree* nodeToSwcTree(Swc_Tree_Node* node) const;
  ResolutionDialog* getResolutionDialog();

  static void expandSwcNodeList(QList<Swc_Tree_Node*> *swcList,
                                const std::set<Swc_Tree_Node*> &swcSet);
  static void expandSwcNodeList(QList<Swc_Tree_Node*> *swcList,
                                const std::set<Swc_Tree_Node*> &swcSet,
                                const Swc_Tree_Node *excluded);
  template<typename T>
  const T* getFirstUserByType() const;

private:
  //Main stack
  ZStack *m_stack;
  ZSparseStack *m_sparseStack; //Serve as main data when m_stack is virtual.

  ZResolution m_resolution;

  ZDocPlayerList m_playerList;

  //Special object
  ZSwcNetwork *m_swcNetwork;

  ZStackObjectGroup m_objectGroup;
  //Swc_Tree_Node *m_lastAddedSwcNode;

  //model-view structure for obj list and edit
  ZSwcObjsModel *m_swcObjsModel;
  ZSwcNodeObjsModel *m_swcNodeObjsModel;
  ZPunctaObjsModel *m_punctaObjsModel;
  ZDocPlayerObjsModel *m_seedObjsModel;
  ZGraphObjsModel *m_graphObjsModel;
  ZSurfaceObjsModel *m_surfaceObjsModel;

  //Parent frame
  ZStackFrame *m_parentFrame;
  ZStack *m_labelField;

  /* workspaces */
  bool m_isTraceMaskObsolete;
  ZNeuronTracer m_neuronTracer;

  //Meta information
  ZStackFile m_stackSource;
  std::string m_additionalSource;

  //Thread
  ZStackReadThread m_reader;

  //Actions
  //  Undo/Redo
  QUndoStack *m_undoStack;
//  QAction *m_undoAction;
//  QAction *m_redoAction;

  //  Action map
  QMap<ZActionFactory::EAction, QAction*> m_actionMap;

  ZSingleSwcNodeActionActivator m_singleSwcNodeActionActivator;

  NeuTube::Document::ETag m_tag;
  NeuTube::EImageBackground m_stackBackground;

  ResolutionDialog *m_resDlg;
  ZStackFactory *m_stackFactory;

  ZActionFactory *m_actionFactory;

  bool m_selectionSilent;
  bool m_isReadyForPaint;
  bool m_isSegmentationReady;
  bool m_autoSaving;

  //QMutex m_mutex;

  QList<QObject*> m_userList;

  ZProgressSignal *m_progressSignal;

  QSet<ZStackObject::ETarget> m_objectModifiedTargetBuffer;
  QMutex m_objectModifiedTargetBufferMutex;
  QSet<ZStackObject::EType> m_objectModifiedTypeBuffer;
  QMutex m_objectModifiedTypeBufferMutex;
  ZStackObjectRole m_objectModifiedRoleBuffer;
  QMutex m_objectModifiedRoleBufferMutex;
  QStack<EObjectModifiedMode> m_objectModifiedMode;
  QMutex m_objectModifiedModeMutex;

  QMutex m_playerMutex;

  QSet<ZStackObject::EType> m_unsavedSet;
  bool m_changingSaveState;

protected:
  ZObjectColorScheme m_objColorSheme;
  ZSharedPointer<ZStackDoc> m_parentDoc;
  ZThreadFutureMap m_futureMap;
};

typedef ZSharedPointer<ZStackDoc> ZStackDocPtr;

//   template  //
template <class InputIterator>
void ZStackDoc::setPunctumSelected(InputIterator first, InputIterator last, bool select)
{
  QList<ZPunctum*> selected;
  QList<ZPunctum*> deselected;
  for (InputIterator it = first; it != last; ++it) {
    ZPunctum *punctum = *it;
    if (punctum->isSelected() != select) {
      punctum->setSelected(select);
      m_objectGroup.setSelected(punctum, select);
      if (select) {
        //m_selectedPuncta.insert(punctum);
        selected.push_back(punctum);
      } else {
        //m_selectedPuncta.erase(punctum);
        deselected.push_back(punctum);
      }
    }
  }
  notifySelectionChanged(selected, deselected);
}

template <class InputIterator>
void ZStackDoc::setSwcSelected(InputIterator first, InputIterator last, bool select)
{
  QList<ZSwcTree*> selected;
  QList<ZSwcTree*> deselected;

  QList<Swc_Tree_Node *> tns;
  for (InputIterator it = first; it != last; ++it) {
    ZSwcTree *tree = *it;
    if (tree->isSelected() != select) {
      m_objectGroup.setSelected(tree, select);
      if (select) {
        selected.append(tree);

        // deselect its nodes
        if (tree->hasSelectedNode()) {
          for (std::set<Swc_Tree_Node*>::const_iterator
               iter = tree->getSelectedNode().begin();
               iter != tree->getSelectedNode().end(); ++iter) {
            Swc_Tree_Node* tn = const_cast<Swc_Tree_Node*>(*iter);
            tns.append(tn);
          }
          tree->deselectAllNode();
        }
        /*
        for (std::set<Swc_Tree_Node*>::iterator it = m_selectedSwcTreeNodes.begin();
             it != m_selectedSwcTreeNodes.end(); ++it) {
          if (tree == nodeToSwcTree(*it))
            tns.push_back(*it);
        }
        */
      } else {
        deselected.push_back(tree);
      }
    }
  }

  notifyDeselected(tns);
  notifySelectionChanged(selected, deselected);
}

template <typename T>
void ZStackDoc::notifySelectionAdded(const std::set<T*> &oldSelected,
                                     const std::set<T*> &newSelected)
{
  QList<T*> selected;
  std::set<T*> addedSet = misc::setdiff(newSelected, oldSelected);
  for (typename std::set<T*>::const_iterator iter = addedSet.begin();
       iter != addedSet.end(); ++iter) {
    selected.append(const_cast<T*>(*iter));
  }
  /*


  for (typename std::set<T*>::const_iterator iter = newSelected.begin();
       iter != newSelected.end(); ++iter) {
    if (oldSelected.count(*iter) == 0) {
      selected.append(const_cast<T*>(*iter));
    }
  }
*/
  notifySelected(selected);
  //notifySelectionChanged(selected, QList<T*>());
}

template <typename T>
void ZStackDoc::notifySelectionRemoved(const std::set<T*> &oldSelected,
                                       const std::set<T*> &newSelected)
{
  QList<T*> deselected;
  std::set<T*> removedSet = misc::setdiff(oldSelected, newSelected);
  for (typename std::set<T*>::const_iterator iter = removedSet.begin();
       iter != removedSet.end(); ++iter) {
    deselected.append(const_cast<T*>(*iter));
  }

  notifyDeselected(deselected);
}

template <typename T>
void ZStackDoc::notifySelected(const QList<T*> &selected)
{
  notifySelectionChanged(selected, QList<T*>());
}

template <typename T>
void ZStackDoc::notifyDeselected(const QList<T*> &deselected)
{
  notifySelectionChanged(QList<T*>(), deselected);
}


template <class InputIterator>
void ZStackDoc::setSwcTreeNodeSelected(
    InputIterator first, InputIterator last, bool select)
{
  for (InputIterator it = first; it != last; ++it) {
    Swc_Tree_Node *tn = *it;
    setSwcTreeNodeSelected(tn, select);
  }
}

template <typename T>
QList<T*> ZStackDoc::getSelectedObjectList() const
{
  T phony;
  return getSelectedObjectList<T>(phony.getType());
}

template <typename T>
QList<T*> ZStackDoc::getSelectedObjectList(ZStackObject::EType type) const
{
  QList<T*> objList;
  TStackObjectSet& objSet = const_cast<TStackObjectSet&>(getSelected(type));

  for (TStackObjectSet::const_iterator iter = objSet.begin();
       iter != objSet.end(); ++iter) {
    T *obj = dynamic_cast<T*>(*iter);
    TZ_ASSERT(obj != NULL, "Unmatched type");
    if (obj != NULL) {
      objList.append(obj);
    }
  }

  return objList;
}

template <typename T>
std::set<T*> ZStackDoc::getSelectedObjectSet(ZStackObject::EType type) const
{
  std::set<T*> objList;
  TStackObjectSet& objSet = const_cast<TStackObjectSet&>(getSelected(type));

  for (TStackObjectSet::const_iterator iter = objSet.begin();
       iter != objSet.end(); ++iter) {
    T *obj = dynamic_cast<T*>(*iter);
    TZ_ASSERT(obj != NULL, "Unmatched type");
    if (obj != NULL) {
      objList.insert(obj);
    }
  }

  return objList;
}

template <typename T>
QList<T*> ZStackDoc::getUserList() const
{
  QList<T*> userList;
  for (QList<QObject*>::const_iterator iter = m_userList.begin();
       iter != m_userList.end(); ++iter) {
    T *user = dynamic_cast<T*>(*iter);
    if (user != NULL) {
      userList.append(user);
    }
  }

  return userList;
}

template <typename InputIterator>
void ZStackDoc::addObjectFast(InputIterator first, InputIterator last)
{
  beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  for (InputIterator iter = first; iter != last; ++iter) {
    ZStackObject *obj = *iter;
    addObjectFast(obj);
  }
  endObjectModifiedMode();
  notifyObjectModified();
}

template <class InputIterator>
void ZStackDoc::removeObjectP(
    InputIterator first, InputIterator last, bool deleting)
{
//  TStackObjectList objList = m_objectGroup.take(type);
  m_objectGroup.take(first, last);
  for (TStackObjectList::iterator iter = first; iter != last; ++iter) {
//    role.addRole(m_playerList.removePlayer(*iter));
    ZStackObject *obj = *iter;

#ifdef _DEBUG_
    std::cout << "Removing object: " << obj << std::endl;
#endif

    bufferObjectModified(obj);
    m_playerList.removePlayer(obj);

    if (deleting) {
      delete obj;
    }
  }

  notifyObjectModified();
#if 0
//  QSet<ZStackObject::EType> typeSet;
//  QSet<ZStackObject::ETarget> targetSet;

//  ZStackObjectRole role;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (InputIterator iter = first; iter != last; ++iter) {
    ZStackObject *obj = *iter;
//    role.addRole(m_playerList.removePlayer(obj));
//    typeSet.insert(obj->getType());
//    targetSet.insert(obj->getTarget());
    processObjectModified(obj);
    if (deleting) {
      delete obj;
    }
  }
  endObjectModifiedMode();

  notifyObjectModified();
#endif
  /*
  if (first != last) {
    processObjectModified(typeSet);
    processObjectModified(targetSet);
    notifyPlayerChanged(role);
  }
  */
}

template<typename T>
QList<T*> ZStackDoc::getObjectList() const
{
  return m_objectGroup.getObjectList<T>();
}

template <typename T>
T* ZStackDoc::getObject(const std::string &source) const
{
  return dynamic_cast<T*>(getObject(T::GetType(), source));
}

#if 0
template <typename T>
void ZStackDoc::registerUser(T *user)
{
  if (!m_userList.contains(user)) {
    m_userList.append(user);
    connect(user, SIGNAL(destroyed(QObject*)), this, SLOT(removeUser(QObject*)));
#ifdef _DEBUG_
    connect(user, SIGNAL(destroyed()), this, SLOT(emptySlot()));
    connect(user, SIGNAL(destroyed(QObject*)), this, SLOT(emptySlot()));
#endif
  }
}
#endif

#endif
