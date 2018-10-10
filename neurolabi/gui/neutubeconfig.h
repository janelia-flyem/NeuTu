#ifndef NEUTUBECONFIG_H
#define NEUTUBECONFIG_H

#include <string>
#include <vector>

#include "zmessagereporter.h"
#if defined(_QT_GUI_USED_)
#include "flyem/zflyemconfig.h"
#endif

#include "zqtheader.h"
#if _QT_GUI_USED_
#include <QDir>
#include <QSettings>
#include <QString>
#endif

class ZXmlNode;
class ZJsonObject;

class NeutubeConfig
{
public:
  enum EConfigItem {
    DATA, FLYEM_BODY_CONN_CLASSIFIER, FLYEM_BODY_CONN_TRAIN_DATA,
    FLYEM_BODY_CONN_TRAIN_TRUTH, FLYEM_BODY_CONN_EVAL_DATA,
    FLYEM_BODY_CONN_EVAL_TRUTH, SWC_REPOSOTARY, AUTO_SAVE,
    CONFIGURE_FILE, SKELETONIZATION_CONFIG, DOCUMENT, TMP_DATA,
    WORKING_DIR, LOG_DIR, LOG_DEST_DIR,
    LOG_FILE, LOG_APPOUT, LOG_WARN, LOG_ERROR, LOG_TRACE
  };

  static NeutubeConfig& getInstance() {
    static NeutubeConfig config;

    return config;
  }

  void init(const std::string &userName);

#ifdef _QT_GUI_USED_
  /*!
   * \brief Get persistent settings.
   *
   * The settings mainly contain information for GUI appearances.
   */
  static QSettings& GetSettings() {
    return getInstance().getSettings();
  }

  /*!
   * \brief Get configuration file path for FlyEM applications.
   */
  static QString GetFlyEmConfigPath();
  static void SetFlyEmConfigPath(const QString &path);
  static void UseDefaultFlyEmConfig(bool on);
  static bool UsingDefaultFlyemConfig();

  static void UseDefaultNeuTuServer(bool on);
  static bool UsingDefaultNeuTuServer();

  static void UseDefaultTaskServer(bool on);
  static bool UsingDefaultTaskServer();

  static QString GetNeuTuServer();
  static void SetNeuTuServer(const QString &path);
  static QString GetTaskServer();
  static void SetTaskServer(const QString &path);
  static bool NamingSynapse();
  static bool NamingPsd();
  static void SetNamingSynapse(bool on);
  static void SetNamingPsd(bool on);

  static void SetDataDir(const QString &dataDir);
#endif

  void enableProfileLogging(bool on);
  void setVerboseLevel(int level);
  int getVerboseLevel() const;
  bool loggingProfile() const;
  void configure(const ZJsonObject &obj);
  void enableAutoStatusCheck(bool on);
  bool autoStatusCheck() const;
  bool parallelTileFetching() const;
  bool lowtisPrefetching() const;
  void setParallelTileFetching(bool on);
  void enableLowtisPrefetching(bool on);
  bool namingSynapse() const;
  bool namingPsd() const;
  void setNamingSynapse(bool on);
  void setNamingPsd(bool on);

  void setAdvancedMode(bool on);
  bool isAdvancedMode() const;

  void setMeshSplitThreshold(size_t thre);
  size_t getMeshSplitThreshold() const;

  static void SetMeshSplitThreshold(size_t thre);
  static size_t GetMeshSplitThreshold();

  static void SetAdvancedMode(bool on);
  static bool IsAdvancedMode();

  static void EnableProfileLogging(bool on);
  static bool LoggingProfile();

  /*!
   * \brief Get the verbose level of the program.
   *
   * The higher level, the more verbose is the program. The lowest level is 0.
   */
  static int GetVerboseLevel();
  static void SetVerboseLevel(int level);
  static bool ParallelTileFetching();
  static void SetParallelTileFetching(bool on);
  static void EnableLowtisPrefetching(bool on);
  static bool LowtisPrefetching();

  /*!
   * \brief Configure from a json object.
   */
  static void Configure(const ZJsonObject &obj);

  static void EnableAutoStatusCheck(bool on);
  static bool AutoStatusCheck();

  inline void setApplicationDir(const std::string &str) {
    m_applicationDir = str;
  }

  static void SetApplicationDir(const std::string &str);

  bool load(const std::string &filePath);
  void print();

  /*!
   * \brief Get the path of a certain item.
   *
   * \a item can be:
   *   CONFIGURE_FILE: general configuration
   *   DOCUMENT: documentation
   *   AUTO_SAVE: autosaving path
   *   SKELETONIZATION_CONFIG: configuration for skeletonization parameters
   *   TMP_DATA: folder for saving temporary data
   *   WORKING_DIR: working directory
   *   LOG_DIR: logging directory
   *   LOG_FILE: prefix for logging files
   *   LOG_TRACE: prefix for tracing files
   */
  std::string getPath(EConfigItem item) const;

  /*!
   * \brief Get the application directory
   *
   * It is supposed to be where the executable is located.
   */
  inline const std::string& getApplicatinDir() const {
    return m_applicationDir; }

  inline std::string getConfigPath() const {
    return getApplicatinDir() + "/config.xml"; }
  inline std::string getHelpFilePath() const {
    return getApplicatinDir() + "/doc/shortcut.html"; }

  inline const std::vector<std::string> &getBodyConnectionFeature() {
    return m_bodyConnectionFeature;
  }

  inline double getSegmentationClassifThreshold() {
    return m_segmentationClassifThreshold;
  }

  inline const std::string& getApplication() const {
    return m_application;
  }

  inline const std::string& getSoftwareName() const {
    return m_softwareName;
  }

  void setDefaultSoftwareName();
  void setTestSoftwareName();

  static std::string GetSoftwareName();
  static void SetDefaultSoftwareName();
  static void SetTestSoftwareName();

  std::string getUserName() const;
  static std::string GetUserName();

  void setUserName(const std::string &name);
  static void SetUserName(const std::string &name);

  inline bool isStereoEnabled() {
    return m_isStereoOn;
  }

  void updateAutoSaveDir();
  static void UpdateAutoSaveDir();

#ifdef _QT_GUI_USED_
  inline QSettings& getSettings() {
    return m_settings;
  }
#if 0
  inline QDebug& getTraceStream() {
    return *m_traceStream;
  }

  static QDebug& GetTraceStream() {
    return NeutubeConfig::getInstance().getTraceStream();
  }
#endif
#endif

  class MainWindowConfig {
  public:
    MainWindowConfig();

#ifdef _QT_GUI_USED_
    void loadXmlNode(const ZXmlNode *node);
#endif

    inline void enableTracing(bool tracingOn) { m_tracingOn = tracingOn; }
    inline void enableMarkPuncta(bool on) { m_isMarkPunctaOn = on; }
    inline void enableSwcEdit(bool on) { m_isSwcEditOn = on; }
    inline void enableMergeImage(bool on) { m_isMergeImageOn = on; }

    inline bool isTracingOn() const { return m_tracingOn; }
    inline bool isTracingOff() const { return !isTracingOn(); }
    inline bool isMarkPunctaOn() const { return m_isMarkPunctaOn; }
    inline bool isMarkPunctaOff() const { return !isMarkPunctaOn(); }
    inline bool isSwcEditOn() const { return m_isSwcEditOn; }
    inline bool isSwcEditOff() const { return !isSwcEditOn(); }
    inline bool isMergeImageOn() const { return m_isMergeImageOn; }
    inline bool isMergeImageOff() const { return !isMergeImageOn(); }

    inline bool isExpandNeuronNetworkOn() const {
      return m_isExpandNeuronNetworkOn;
    }
    inline bool isExpandTracingResultOn() const {
      return m_isExpandTracingResultOn;
    }

    inline bool isExpandV3dApoOn() const {
      return m_isExpandV3dApoOn;
    }

    inline bool isExpandV3dMarkerOn() const{
      return m_isExpandV3dMarkerOn;
    }

    inline bool isExpandSwcWith3DWindow() const {
      return m_isExpandSwcWith3DWindow;
    }
    inline void setExpandSwcWith3DWindow(bool state) {
      m_isExpandSwcWith3DWindow = state;
    }

    inline bool isProcessBinarizeOn() const {
      return m_isProcessBinarizeOn;
    }
    inline void setProcessBinarize(bool state) {
      m_isProcessBinarizeOn = state;
    }

    inline bool isMaskToSwcOn() const {
      return m_isMaskToSwcOn;
    }
    inline void setMaskToSwc(bool state) {
      m_isMaskToSwcOn = state;
    }

    inline bool isBinaryToSwcOn() const {
      return m_isBinaryToSwcOn;
    }
    inline void setBinaryToSwc(bool state) {
      m_isBinaryToSwcOn = state;
    }

    inline bool isThresholdControlOn() const {
      return m_isThresholdControlOn;
    }
    inline void setThresholdControl(bool state) {
      m_isThresholdControlOn = state;
    }

  private:
    bool m_tracingOn;
    bool m_isMarkPunctaOn;
    bool m_isSwcEditOn;
    bool m_isMergeImageOn;
    bool m_isExpandSwcWith3DWindow;
    bool m_isExpandNeuronNetworkOn;
    bool m_isExpandTracingResultOn;
    bool m_isExpandV3dApoOn;
    bool m_isExpandV3dMarkerOn;
    bool m_isProcessBinarizeOn;
    bool m_isMaskToSwcOn;
    bool m_isBinaryToSwcOn;
    bool m_isThresholdControlOn;
  };

  class Z3DWindowConfig {
  public:
    Z3DWindowConfig();
#ifdef _QT_GUI_USED_
    void loadXmlNode(const ZXmlNode *node);
#endif
    inline bool isUtilsOn() const { return m_isUtilsOn; }
    inline bool isVolumeOn() const { return m_isVolumeOn; }
    inline bool isGraphOn() const { return m_isGraphOn; }
    inline bool isSwcsOn() const { return m_isSwcsOn; }
    inline bool isTubesOn() const { return m_isTubesOn; }
    inline bool isPunctaOn() const { return m_isPunctaOn; }
    inline bool isMeshOn() const { return m_isMeshOn; }
    inline bool isTensorOn() const { return m_isTensorOn; }
    inline bool isAxisOn() const { return m_isAxisOn; }
    inline bool isBackgroundOn() const { return m_isBackgroundOn; }

    inline void enableUtils(bool on) { m_isUtilsOn = on; }
    inline void enableVolume(bool on) { m_isVolumeOn = on; }
    inline void enableGraph(bool on) { m_isGraphOn = on; }
    inline void enableSwcs(bool on) { m_isSwcsOn = on; }
    inline void enableTubes(bool on) { m_isTubesOn = on; }
    inline void enablePuncta(bool on) { m_isPunctaOn = on; }
    inline void enableTensor(bool on) { m_isTensorOn = on; }
    inline void enableMesh(bool on) { m_isMeshOn = on; }
    inline void enableAxis(bool on) { m_isAxisOn = on; }
    inline void enableBackground(bool on) { m_isBackgroundOn = on; }

    class SwcTabConfig {
    public:
      SwcTabConfig();
#ifdef _QT_GUI_USED_
      void loadXmlNode(const ZXmlNode *node);
#endif
      inline std::string getPrimitive() const { return m_primitive; }
      inline std::string getColorMode() const { return m_colorMode; }
      inline double getZScale() const { return m_zscale; }

    private:
      std::string m_primitive;
      std::string m_colorMode;
      double m_zscale;
    };

    class GraphTabConfig {
    public:
      GraphTabConfig();
#ifdef _QT_GUI_USED_
      void loadXmlNode(const ZXmlNode *node);
#endif
      inline bool isVisible() const { return m_isVisible; }
      inline double getOpacity() const { return m_opacity; }

    private:
      bool m_isVisible;
      double m_opacity;
    };

    const SwcTabConfig& getSwcTabConfig() const {
      return m_swcTabConfig;
    }

    const GraphTabConfig& getGraphTabConfig() const {
      return m_graphTabConfig;
    }

  private:
    bool m_isUtilsOn;
    bool m_isVolumeOn;
    bool m_isGraphOn;
    bool m_isSwcsOn;
    bool m_isTubesOn;
    bool m_isPunctaOn;
    bool m_isMeshOn;
    bool m_isTensorOn;
    bool m_isAxisOn;
    bool m_isBackgroundOn;
    SwcTabConfig m_swcTabConfig;
    GraphTabConfig m_graphTabConfig;
  };

  class ObjManagerConfig {
  public:
    ObjManagerConfig();
#ifdef _QT_GUI_USED_
    void loadXmlNode(const ZXmlNode *node);
#endif
    inline bool isSwcOn() const { return m_isSwcOn; }
    inline bool isCategorizedSwcNodeOn() const { return m_isSwcNodeOn; }
    inline bool isPunctaOn() const { return m_isPunctaOn; }
    inline bool isMeshOn() const { return m_isMeshOn; }

  private:
    bool m_isSwcOn;
    bool m_isSwcNodeOn;
    bool m_isPunctaOn;
    bool m_isMeshOn = true;
  };

  inline const MainWindowConfig& getMainWindowConfig() const {
    return m_mainWindowConfig;
  }

  inline const Z3DWindowConfig& getZ3DWindowConfig() const {
    return m_z3dWindowConfig;
  }

  inline const ObjManagerConfig& getObjManagerConfig() const {
    return m_objManagerConfig;
  }

  inline ZMessageReporter* getMessageReporter() {
    return m_messageReporter;
  }

  inline bool isSettingOn() const { return m_isSettingOn; }

  inline int getAutoSaveInterval() const { return m_autoSaveInterval; }
  void setWorkDir(const std::string str);
  bool isAutoSaveEnabled() const { return m_autoSaveEnabled; }

  inline bool usingNativeDialog() const { return m_usingNativeDialog; }
  inline bool usingDvidBrowseDialog() const { return m_usingDvidBrowseDialog; }

#if 0
  const ZFlyEmConfig &getFlyEmConfig() const { return m_flyemConfig; }
  ZFlyEmConfig &getFlyEmConfig() { return m_flyemConfig; }
#endif

private:
  NeutubeConfig();
  NeutubeConfig(const NeutubeConfig&);
  ~NeutubeConfig();
  void operator=(const NeutubeConfig&);

  void updateLogDir();

private:
  std::string m_application;
  std::string m_softwareName;
  std::string m_applicationDir;
  std::string m_docUrl;

  //Obsolete FlyEM config
  std::string m_segmentationClassifierPath;
  std::string m_segmentationTrainingTestPath;
  std::string m_segmentationTrainingTruthPath;
  std::string m_segmentationEvaluationTestPath;
  std::string m_segmentationEvaluationTruthPath;
  std::string m_swcRepository;
  double m_segmentationClassifThreshold;
  std::vector<std::string> m_bodyConnectionFeature;
  //////////////////////

  std::string m_dataPath;
  std::string m_developPath;
  std::string m_userName;

  MainWindowConfig m_mainWindowConfig;
  Z3DWindowConfig m_z3dWindowConfig;
  ObjManagerConfig m_objManagerConfig;
  bool m_isSettingOn;
  bool m_isStereoOn;
  //std::string m_autoSaveDir;
  std::string m_workDir;
  std::string m_logDir;
  std::string m_logDestDir;
  int m_autoSaveInterval;
  int m_autoSaveMaxSwcCount;
  bool m_autoSaveEnabled;
  bool m_usingNativeDialog;
  bool m_usingDvidBrowseDialog;
  bool m_loggingProfile;
  int m_verboseLevel;
  bool m_advancedMode = false;
  size_t m_meshSplitThreshold = 5000000;

  ZMessageReporter *m_messageReporter; //Obsolete

#ifdef _QT_GUI_USED_
//  QDebug *m_traceStream;
  QSettings m_settings;
#endif
};

#define GET_DATA_DIR (NeutubeConfig::getInstance().getPath(NeutubeConfig::DATA))
#if defined(PROJECT_PATH)
#  define GET_TEST_DATA_DIR (std::string(PROJECT_PATH) + "/../data")
#endif

#ifndef GET_TEST_DATA_DIR
#  define GET_TEST_DATA_DIR GET_DATA_DIR
#endif

#define GET_BENCHMARK_DIR (GET_TEST_DATA_DIR + "/_benchmark")
#define GET_FLYEM_DATA_DIR (GET_TEST_DATA_DIR + "/_flyem")

#define GET_MESSAGE_REPORTER (NeutubeConfig::getInstance().getMessageReporter())
#define GET_APPLICATION_NAME (NeutubeConfig::getInstance().getApplication())
#define GET_APPLICATION_DIR (NeutubeConfig::getInstance().getApplicatinDir())
#define GET_SOFTWARE_NAME (NeutubeConfig::getInstance().getSoftwareName())
#define GET_DOC_DIR (NeutubeConfig::getInstance().getApplicatinDir() + "/doc")
#define GET_TMP_DIR (NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA))

#if defined(_QT_GUI_USED_)
#  define GET_FLYEM_CONFIG (ZFlyEmConfig::getInstance())
#  define GET_NETU_SERVICE (GET_FLYEM_CONFIG.getNeutuService())
#endif

#define ZOUT(out, level) \
  if (NeutubeConfig::GetVerboseLevel() < level) {} \
  else out

#endif // NEUTUBECONFIG_H
