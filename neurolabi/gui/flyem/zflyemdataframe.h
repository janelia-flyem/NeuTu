#ifndef ZFLYEMDATAFRAME_H
#define ZFLYEMDATAFRAME_H

#include <QMdiSubWindow>
#include <string>
#include <QProgressBar>
#include <QVector>
#include <utility>

#include "zflyemdatabundle.h"
#include "zswctreematcher.h"
#include "flyemdataframeoptiondialog.h"
#include "dialogs/flyemdataquerydialog.h"
#include "dialogs/flyemdataprocessdialog.h"
#include "zprogressable.h"
#include "zqtbarprogressreporter.h"
#include "zswctreebatchmatcher.h"
#include "flyem/zflyemneuronmatchtaskmanager.h"
#include "flyem/zflyemneuronfiltertaskmanager.h"
#include "flyem/zflyemqualityanalyzertaskmanager.h"
#include "dvid/zdvidtarget.h"
#include "flyem/zflyemneuronimagefactory.h"
#include "zwindowfactory.h"

class ZSwcTrunkAnalyzer;
class ZSwcFeatureAnalyzer;
class ZFlyEmNeuronInfoPresenter;
class ZFlyEmNeuronFeaturePresenter;
class ZFlyEmNeuronConnectionPresenter;
class ZFlyEmNeuronVolumePresenter;
class ZFlyEmNeuronTopMatchPresenter;
class FlyEmDataForm;
class QStatusBar;
class FlyEmGeoSearchDialog;
class FlyEmGeoFilterDialog;
class FlyEmNeuronThumbnailDialog;
class ZFlyEmNeuronImageFactory;
class FlyEmHotSpotDialog;

class ZFlyEmDataFrame : public QMdiSubWindow, ZProgressable
{
  Q_OBJECT
public:
  explicit ZFlyEmDataFrame(QWidget *parent = 0);

public:
  ~ZFlyEmDataFrame();
  
  enum EDataForm {
    ID, NAME, TYPE, MODEL, SUMMARY, CONNECTION, FEATURE, VOLUME, TOP_MATCH,
    CONNECTION_MODEL, UNKNOWN_DATA_FORM
  };

  enum EAction {
    MATCH, SORT_SHAPE, PREDICT_CLASS, PREDICT_ERROR, UNKNOWN_ACTION
  };

  enum EScoreOption {
    SCORE_MATCH, //matching score only
    SCORE_ORTREG, //regularized by orientation
    SCORE_ORTDIV  //separated by orientation
  };

  //Load a data bundle file. It returns false if the file cannot be loaded
  //correctly.
  bool load(const std::string &filePath, bool appending = false);

  void clearData();
  void addData(ZFlyEmDataBundle *data);

  //Return the information of the flyem data
  std::string getInformationText() const;

  void updatePresenter(EDataForm target);

  //Update the query text for showing <target> for <id>
  int updateQuery(int id, int bundleIndex,
                   EDataForm target, bool appending = false);

  //void displayQueryOutput(const std::string &text, bool appending = false) const;
  void displayQueryOutput(const ZFlyEmNeuron *neuron, bool appending = false) const;
  //void dump(const std::string &message) const;
  void dump(const QString &message) const;

  void setStatusBar(QStatusBar *bar);

  void predictClass(ZFlyEmNeuron *neuron);
  void predictClass(const QVector<ZFlyEmNeuron*> &neuronArray);
  void predictClass();

  /*!
   * \brief Reassign classes to neurons
   * \param classFile The list of class names in the same order how the neurons
   *        are listed in the bundles.
   */
  void assignClass(const std::string &classFile);

  /*!
   * \brief Export unnormalized similarity matrix
   *
   * The first row is a list of IDs. The simarity between the same neuron is
   * also calculated.
   *
   * \a return true iff the export succeeds.
   */
  bool exportSimilarityMatrix(const QString &fileName, int bundleIndex = 0);

  inline ZFlyEmDataBundle* getDataBundle(int index = 0) {
    return (index < m_dataArray.size()) ? m_dataArray[index] : NULL;
  }

  inline const ZFlyEmDataBundle* getDataBundle(int index = 0) const {
    return (index < m_dataArray.size()) ? m_dataArray[index] : NULL;
  }

  /*!
   * \brief Compute and save morphological features of all the neurons
   *
   * \param includingLabel Add the class label as the first column
   * \return true iff the file is saved.
   */
  bool saveNeuronFeature(const QString &path, bool includingLabel);

  const std::vector<std::string>& getNeuronFeatureName();

  /*!
   * \brief Export thumbnails into a directory
   * \param Target directory
   */
  void exportThumbnail(const QString &saveDir, bool thumbnailUpdate,
                       const ZFlyEmNeuronImageFactory &imageFactory);
  void exportThumbnail();

  /*!
   * \brief Export layer features
   */
  void exportLayerFeature(const QString &savePath) const;

  void exportBundle(const QString &savePath);

  void exportSideBoundaryAnalysis(
      const QString &savePath, const QString &substackPath,
      const QString &synapsePath);

  void importBoundBox(const QString &substackPath);
  void importSynapseAnnotation(const QString &filePath, bool coordAdjust = false);

  /*!
   * \brief Upload annotations to DVID server.
   *
   * Empty annotations will be ignored.
   */
  void uploadAnnotation() const;

  /*!
   * \brief Set volume entries based on a directory
   *
   * The volume entry is set even the corresponding body file does not exist.
   * It only applies to the first bundle.
   *
   * \param dirName The volume directory path.
   */
  void setVolume(const QString &dirName);

  /*!
   * \brief Set thumbnails based on a directory
   *
   * The thumnal entry is set even the corresponding file does not exist.
   * It only applies to the first bundle.
   *
   * \param dirName The thumbnail directory path.
   */
  void setThumbnail(const QString &dirName);

  void identifyHotSpot();
  void identifyHotSpot(int id);

  void submitSkeletonizeService() const;

  const ZFlyEmNeuron *getNeuronFromIndex(
      size_t idx, int *bundleIndex = NULL) const;

  void setDvidTarget(const ZDvidTarget &target);
  const ZDvidTarget& getDvidTarget() const;

  inline ZFlyEmNeuronImageFactory* getImageFactory() {
    return &m_imageFactory;
  }

  ZFlyEmDataBundle* getMasterData();
  const ZFlyEmDataBundle* getMasterData() const;

signals:
  void volumeTriggered(const QString &path);
  
public slots:
  //Show summary information
  void showSummary() const;
  void query();
  void process();
  void test();
  //void generalProcess();
  void setParameter();
  void openVolume(const QString &path);
  /*!
   * \brief Save a bundle.
   * \param index Index of the bundle.
   * \param path Output path.
   */
  void saveBundle(int index, const QString &path);

  void showNearbyNeuron(const ZFlyEmNeuron *neuron);
  void searchNeighborNeuron(const ZFlyEmNeuron *neuron);

  void updateTypePrediction();
  void updateSearchResult();
  void updateQualityControl();

private:
  FlyEm::ZSynapseAnnotationArray *getSynapseAnnotation();
  std::string getName(int bodyId) const;
  std::string getName(int bodyId, int bundleIndex) const;
  std::string getName(const std::pair<int, int> &bodyId) const;

  const ZFlyEmNeuron* getNeuron(int id, int bundleIndex = -1) const;
  ZFlyEmNeuron* getNeuron(int id, int bundleIndex = -1);
  const ZFlyEmNeuron* getNeuron(const std::pair<int, int> &bodyId) const;

  size_t getNeuronNumber() const;

  ZSwcTree* getModel(int id, int bundleIndex = -1);
  const QColor* getColor(int id, int bundleIndex = -1) const;
  const QColor* getColor(const std::pair<int, int> &bodyId) const;

  FlyEmDataForm *getMainWidget() const;
  void prepareClassPrediction(ZFlyEmNeuron *neuron);

  bool initTaskManager(ZMultiTaskManager *taskManager);

  std::vector<std::vector<double> > computeLayerFeature(
      const ZFlyEmNeuron &neuron) const;

private:
  void parseCommand(const std::string &command);

  void parseCommand(const std::string &sourceType,
                    const std::string &sourceValue,
                    const std::string &action);

  void parseQuery(const std::string &sourceType,
                  const std::string &sourceValue,
                  const std::string &targetType, bool usingRegexp);

  void updateSource(const std::string &sourceType,
                    const std::string &sourceValue,
                    bool usingRegexp);

  void showModel() const;
  void showConnection() const;
  enum EMatchingPool {
    MATCH_ALL_NEURON, MATCH_WITHOUT_SELF, MATCH_KNOWN_CLASS
  };

  std::vector<double> getMatchingScore(
      const ZFlyEmNeuron *neuron, EMatchingPool pool = MATCH_KNOWN_CLASS);

  std::vector<double> getMatchingScore(
      int id, int bundleIndex, EMatchingPool pool = MATCH_KNOWN_CLASS);

  void clearQueryOutput();

  QVector<const ZFlyEmNeuron*> getTopMatch(
      const ZFlyEmNeuron *neuron, EMatchingPool pool = MATCH_KNOWN_CLASS);

  QProgressBar* getProgressBar();

  /*!
   * \brief Get the source path of a data bundle
   *
   * \return It returns empty string if \a index is out of range.
   */
  const QString getDataBundleSource(int index = 0) const;

  bool isDataBundleIndexValid(int index) const;

private:
  //Main data
  QVector<ZFlyEmDataBundle*> m_dataArray; //The first bundle is treated as the master
  
  //View
  FlyEmDataForm *m_centralWidget;

  //parsing results
  EDataForm m_target;
  std::vector<std::pair<int, int> > m_sourceIdArray;
  EAction m_action;
  std::string m_condition;

  //configuration
  ZSwcTreeMatcher m_matcher;
  ZSwcTrunkAnalyzer *m_trunkAnalyzer;
  ZSwcFeatureAnalyzer *m_featureAnalyzer;
  ZSwcFeatureAnalyzer *m_helperFeatureAnalyzer;
  double m_resampleStep;
  int m_matchingLevel;

  FlyEmDataFrameOptionDialog m_optionDialog;
  FlyEmDataQueryDialog m_queryDialog;
  FlyEmDataProcessDialog m_processDialog;
  //bool m_checkOrientation;
  EScoreOption m_scoreOption;

  ZFlyEmNeuronInfoPresenter *m_infoPresenter;
  ZFlyEmNeuronFeaturePresenter *m_featurePresenter;
  ZFlyEmNeuronConnectionPresenter *m_connectionPresenter;
  ZFlyEmNeuronVolumePresenter *m_volumePresenter;
  ZFlyEmNeuronTopMatchPresenter *m_topMatchPresenter;

  ZQtBarProgressReporter m_specialProgressReporter;

  FlyEmGeoFilterDialog *m_geoSearchDlg;
  FlyEmNeuronThumbnailDialog *m_thumbnailDlg;
  FlyEmHotSpotDialog *m_hotSpotDlg;

  ZFlyEmNeuronMatchTaskManager *m_matchManager;
  ZFlyEmNeuronFilterTaskManager *m_filterManager;
  ZFlyEmQualityAnalyzerTaskManager *m_qualityManager;
  //ZSwcTreeBatchMatcher *m_batchMatcher;

  QVector<ZFlyEmNeuron*> m_foregroundNeuronArray;

  ZDvidTarget m_dvidTarget;
  ZFlyEmNeuronImageFactory m_imageFactory;
  ZWindowFactory m_3dWindowFactory;
};

#endif // ZFLYEMDATAFRAME_H
