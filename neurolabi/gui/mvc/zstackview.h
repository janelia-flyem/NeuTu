/**@file zstackview.h
 * @brief Stack view
 * @author Ting Zhao
 */

#ifndef ZSTACKVIEW_H_
#define ZSTACKVIEW_H_

#include <atomic>
#include <memory>

#include <QImage>
#include <QWidget>
#include <QPixmap>
#include <QCheckBox>
#include <vector>
#include <functional>

#include "zstackframe.h"
#include "widgets/zparameter.h"
#include "tz_image_lib_defs.h"
#include "neutube.h"
#include "zpaintbundle.h"
#include "common/zsharedpointer.h"
#include "zmessageprocessor.h"
#include "zpainter.h"
#include "zmultiscalepixmap.h"
#include "zarbsliceviewparam.h"
#include "mvc/mvcdef.h"
#include "concurrent/zworkerwrapper.h"
#include "data3d/zsliceviewtransform.h"
#include "zstackobjectpaintsorter.h"

class ZStackDoc;
class ZStackPresenter;
class QSlider;
class ZImageWidget;
class QVBoxLayout;
class QHBoxLayout;
class QSpinBox;
class ZImage;
class QLabel;
class ZSlider;
class QMenu;
class QPushButton;
class QProgressBar;
class QRadioButton;
class ZStack;
class ZStackViewParam;
class ZMessageManager;
class ZBodySplitButton;
class ZStackMvc;
class ZPixmap;
class ZLabeledSpinBoxWidget;
class QSpacerItem;
class ZWidgetMessage;
class ZStTransform;
class ZScrollSliceStrategy;
class ZStackViewParam;
class ZStackObjectPainter;
class ZStackViewRecorder;
class ZStackViewRecordDialog;
class ZSliceCanvas;
class ZCheckBoxGroup;
class ZH3Widget;
class ZHWidget;

/*!
 * \brief The ZStackView class shows 3D data slice by slice
 *
 * ZStackView provides a widget of showing 3D data slice by slice, which can be
 * along any of the three axes. It displays
 * the data in an image screen with the support of slice change and zooming. It
 * is designed to be the view component of the MVC framework, which can be
 * represented by the ZStackFrame class (and its derivatives) or the
 * ZStackMvc class.
 */
class ZStackView : public QWidget, ZWorkerWrapper {
  Q_OBJECT

public:
  explicit ZStackView(ZStackFrame *parent);
  explicit ZStackView(QWidget *parent = nullptr);
  ~ZStackView() override;

public:
  /*!
   * \brief Reset the view status
   *
   * This function is for synchorinizing the view controls such as depth slider
   * and channel controls with stack update in the document.
   */
//  void reset(bool updatingScreen = true);

  enum class EMode {
    NORMAL, IMAGE_ONLY, PLAIN_IMAGE
  };

  void configure(EMode mode);

  enum class EUpdateOption {
    NONE, //No update
    QUEUED, //Put updating request in a queue
    DIRECT //Update immediately
  };

  int getViewId() const;

  bool viewingInfo(neutu::mvc::ViewInfoFlags f) const;
  void setViewInfoFlag(neutu::mvc::ViewInfoFlags f);
  neutu::mvc::ViewInfoFlags getViewInfoFlag() const;

  /*!
   * \brief Update image screen
   *
   * Update the screen by assuming that all the canvas buffers are ready.
   */
  void updateImageScreen(EUpdateOption option);

  struct RefreshConfig {
    EUpdateOption updateOption = EUpdateOption::QUEUED;
    bool widgetCanvasUpdateRequired = true;
  };

  void refreshScreen(const RefreshConfig &config);

  /*!
   * \brief Restore the view from a bad setting
   *
   * The definition of a bad setting is provided by ZImageWidget::isBadView().
   */
  bool restoreFromBadView();

  /*!
   * \brief Get the parent frame
   *
   * \return NULL if the parent is not ZStackFrame.
   */
  ZStackFrame* getParentFrame() const;

  /*!
   * \brief Get the parent MVC widget
   *
   * \return NULL if the parent is not ZStackMvc.
   */
  ZStackMvc* getParentMvc() const;

  ZSharedPointer<ZStackDoc> buddyDocument() const;
  ZStackPresenter* buddyPresenter() const;

  QSize sizeHint() const override;

  /*!
   * \brief Get the widget of data display
   */
  inline ZImageWidget* imageWidget() const { return m_imageWidget; }

  inline QProgressBar* progressBar() { return m_progress; }

  /*!
   * \brief Get the current slice index.
   *
   * The slice index is the z offset from the current slice to the start slice.
   * Therefore the index of the first slice is always 0.
   */
  int sliceIndex() const;

  /*!
   * \brief Get the max slice index
   */
  int maxSliceIndex();

  /*!
   * \brief Set the current slice index.
   *
   * The value will be clipped if \a slice is out of range.
   */
  void setSliceIndex(int slice);

  /*!
   * \brief Set the slice index without emitting signal.
   */
  void setSliceIndexQuietly(int slice);

  /*!
   * \brief Increase or descrease of the slice index with a certain step.
   *
   * The slice index is set to the sum of the current slice and \a step, which
   * can be either positive or negative.
   */
  void stepSlice(int step);

  /*!
   * \brief Get the current Z position.
   *
   * \return The current Z position, which is defined as the slice index plus
   * the Z coordinate of the stack offset.
   */
  int getCurrentDepth() const;

  ZPlane getCutOrientation() const;

  //int threshold();

  void setSliceAxis(neutu::EAxis axis);
  neutu::EAxis getSliceAxis() const;// { return m_sliceAxis; }
//  ZAffinePlane getAffinePlane() const;

  void setRightHanded(bool r);

  void setCutPlane(neutu::EAxis axis);
  void setCutPlane(const ZPoint &v1, const ZPoint &v2);
  void setCutPlane(const ZAffinePlane &plane);
  void setZoomScale(double s);
  void setInitialScale(double s);

  /*!
   * \brief Get stack data from the buddy document
   */
  ZStack *stackData() const;

  /*!
   * \brief set up the view after the document is ready
   */
  void prepareDocument();

//  virtual void resizeEvent(QResizeEvent *event) override;
  virtual void showEvent(QShowEvent *event) override;

  /*!
   * \brief Get the information of the view as a list of strings.
   */
  QStringList toStringList() const;

  //void setImageWidgetCursor(const QCursor &cursor);
  /*!
   * \brief Set the cursor of the image screen
   */
  void setScreenCursor(const QCursor &cursor);

  /*!
   * \brief Set up the scroll strategy.
   *
   * The scroll strategy \a strategy controls the scrolling behavior of the view.
   * The \a strategy pointer is owned by the view.
   */
  void setScrollStrategy(ZScrollSliceStrategy *strategy);

  /*!
   * \brief Intensity threshold for stack data.
   */
  int getIntensityThreshold();

  /*!
   * \brief Save the current scene in an image file.
   */
  void takeScreenshot(const QString &filename);
  void takeScreenshot();

  void paintStackBuffer();
  void paintMaskBuffer();


//  bool paintTileCanvasBuffer();

  //void paintObjectBuffer(ZImage *canvas, neutu::data3d::ETarget target);

//  void paintActiveDecorationBuffer();
//  void paintDynamicObjectBuffer();


  ZStack* getStrokeMask(uint8_t maskValue);
  ZStack* getStrokeMask(neutu::EColor color);

  ZPoint getCurrentMousePosition(neutu::data3d::ESpace space);


//  void exportObjectMask(const std::string &filePath);
//  void exportObjectMask(neutu::EColor color, const std::string &filePath);

  inline void setSizeHintOption(neutu::ESizeHintOption option) {
    m_sizeHintOption = option;
  }

  inline void blockRedraw(bool state) {
    m_isRedrawBlocked = state;
  }

public:
  bool isViewPortFronzen() const;

  /*!
   * \brief Check if the view depth is frozen.
   *
   * The depth cannot be changed if it is frozen.
   */
  bool isDepthFronzen() const;

  bool isViewChangeEventBlocked() const;

  /*!
   * \brief Check if the view is scrollable on depth.
   *
   * If the depth is not scrollable, the user will not be able to scroll the
   * slice with mouse wheels.
   */
  bool isDepthScrollable();

  void ignoreScroll(bool on);
  void pauseScroll();

  void setViewPortFrozen(bool state);
  void setDepthFrozen(bool state);
  void blockViewChangeEvent(bool state);

  void zoomTo(int x, int y, int z);
  void zoomTo(int x, int y, int z, int w);
  void zoomTo(const ZIntPoint &pt);
  void zoomTo(const ZIntPoint &pt, int w);
  void zoomTo(const ZPoint &pt, int w, bool showingHint);

  void rotateView(double da, double db);

  void printViewParam() const;
  void setMaxViewPort(int s) {
    m_maxViewPort = s;
  }

  void setDefaultViewPort(const QRect &rect) {
    m_defaultViewPort = rect;
  }

  ZStackViewRecorder* getRecorder();
  void configureRecorder();

public: //Message system implementation
  class MessageProcessor : public ZMessageProcessor {
  public:
    void processMessage(ZMessage *message, QWidget *host) const;
  };

  void enableMessageManager();



public:
  static QImage::Format stackKindToImageFormat(int kind);
//  double getCanvasWidthZoomRatio() const;
//  double getCanvasHeightZoomRatio() const;
//  double getProjZoomRatio() const;
  void setInfo();
  bool isImageMovable() const;

  int getZ(neutu::ECoordinateSystem coordSys) const;

  ZPoint getCutCenter() const;
  ZAffineRect getCutRect() const;
//  ZIntPoint getCenter(
//      neutu::ECoordinateSystem coordSys = neutu::ECoordinateSystem::STACK) const;

  ZAffineRect getViewPort() const;
  ZStackViewParam getViewParameter() const;

//  QRectF getProjRegion() const;
  ZViewProj getViewProj() const;

  //Get transform from view port to proj region
//  ZStTransform getViewTransform() const;

  ZPoint transform(
      const ZPoint &pt, neutu::data3d::ESpace src, neutu::data3d::ESpace dst) const;

  /*!
   * \brief Set the viewport offset
   *
   * (\a x, \a y) is supposed to the world coordinates of the first corner of
   * the viewport. The real position will be adjusted to fit in the canvas.
   */
//  void setViewPortOffset(int x, int y);

  void setViewPortCenter(int x, int y, int z, neutu::EAxisSystem system);
  void setViewPortCenter(const ZIntPoint &center, neutu::EAxisSystem system);

  void resetViewParam(const ZArbSliceViewParam &param);

  ZIntPoint getViewCenter() const;

  void paintMultiresImageTest(int resLevel);
  void customizeWidget();

//  void addHorizontalWidget(QWidget *widget);
//  void addHorizontalWidget(QSpacerItem *spacer);

//  void notifyViewPortChanged();

  bool isViewChanged(const ZSliceViewTransform &t);
  bool isViewChanged(const ZStackViewParam &param) const;
  void processViewChange(bool redrawing, bool depthChanged);

  bool signalingViewChange() const;
  void enableViewChangeSignal(bool on);
//  void processViewChange(bool redrawing);
//  void processViewChange(const ZStackViewParam &param);

  void setHoverFocus(bool on);
  void setSmoothDisplay(bool on);

//  void notifyViewChanged(const ZStackViewParam &param);

  /*!
   * \brief Get the size of the image window.
   *
   * It's the size of the widget in which any dot can be painted by an image
   * pixel.
   */
  QSize getScreenSize() const;

  void addToolButton(QPushButton *button);
  void removeToolButton(QPushButton *button);
  void setWidgetReady(bool ready);

  void toggleAllControls();

  size_t getFrameCount() const;

public: //Change view parameters
  void moveViewPort(const QPoint& src, const QPointF &dst);
  void moveViewPort(const ZPoint &src, int a, int b);
  void moveViewPort(int dx, int dy);

  void increaseZoomRatio();
  void decreaseZoomRatio();
  void increaseZoomRatio(int x, int y, bool usingRef = true);
  void decreaseZoomRatio(int x, int y, bool usingRef = true);

//  void zoomWithWidthAligned(int x0, int x1, int cy);
//  void zoomWithWidthAligned(int x0, int x1, double pw, int cy, int cz);
//  void zoomWithHeightAligned(int y0, int y1, double ph, int cx, int cz);
//  void notifyViewChanged(
//      NeuTube::View::EExploreAction action = NeuTube::View::EXPLORE_UNKNOWN);
//  void highlightPosition(int x, int y, int z);
//  void highlightPosition(const ZIntPoint &pt);

  void updateContrastProtocal();

  ZPoint getAnchorPoint(neutu::data3d::ESpace space) const;
  std::set<neutu::data3d::ETarget> updateViewData();
  void updateSceneWithViewData();

public: //View parameters for arbitrary plane
//  ZStackViewParam getViewParameter(const ZArbSliceViewParam &param) const;
//  ZArbSliceViewParam getSliceViewParam() const;
  ZSliceViewTransform getSliceViewTransform() const;
  void setSliceViewTransform(const ZSliceViewTransform &t);

  void updateDataInfo(const QPoint &widgetPos);

  void updateObjectCanvasVisbility(bool visible);
//  void setSliceViewParam(const ZArbSliceViewParam &param);
//  void showArbSliceViewPort();

  void invalidateObjectSorter();
  void updateObjectSorter();

protected:
//  ZIntCuboid getViewBoundBox() const;
  ZIntCuboid getCurrentStackRange() const;
  int getDepth() const;

  void clearCanvas();
//  template<typename T>
//  void resetCanvasWithStack(T &canvas, ZPainter *painter);

//  virtual void resetCanvasWithStack(
//      ZMultiscalePixmap &canvas, ZPainter *painter);

//  void reloadTileCanvas();
//  bool reloadObjectCanvas(bool repaint = false);
  void reloadCanvas();

//  ZSliceCanvas* updateCanvas(ZSliceCanvas *canvas);

  void connectSignalSlot();

  /*!
   * \brief Get the size of the canvas.
   *
   * It is usually the same as the size of a stack slice.
   */
//  QSize getCanvasSize() const;

  //help functions
  virtual void paintSingleChannelStackSlice(ZStack *stack, int slice);
  void paintMultipleChannelStackSlice(ZStack *stack, int slice);
  void paintSingleChannelStackMip(ZStack *stack);
  void paintMultipleChannelStackMip(ZStack *stack);

  std::set<neutu::data3d::ETarget> updateViewData(const ZStackViewParam &param);

  void init();

//  ZPainter* getPainter(neutu::data3d::ETarget target);
//  ZPixmap* getCanvas(neutu::data3d::ETarget target);
//  void setCanvasVisible(neutu::data3d::ETarget target, bool visible);
//  void resetDepthControl();

  void setSliceRange(int minSlice, int maxSlice);

  bool event(QEvent *event) override;

  void enableOffsetAdjustment(bool on);

public slots:
  /*!
   * \brief Update view settings from the stack box.
   *
   * It resets view parameters according the current bounding box of the stack.
   */
  void updateViewBox();

  /*!
   * \brief Redraw the whole scene.
   */
  void redraw(EUpdateOption option = EUpdateOption::QUEUED);

  /*!
   * \brief Redraw objects.
   *
   * It redraws objects in the object canvases.
   */
  void redrawObject();

  void updateStackRange();
  void processStackChange(bool rangeChanged);
  void updateThresholdSlider();
  void updateSlider();
  void updateStackInfo();
  void updateChannelControl();
  void processDepthSliderValueChange();
  void processDepthSliderValueChange(int sliceIndex);
  void updateStackWidget();

  void paintStack();
  void paintMask();
//  void paintObject();
//  void paintObject(QList<ZStackObject *> selected,
//                   QList<ZStackObject *> deselected);
  void paintObject(const ZStackObjectInfoSet &selected,
                   const ZStackObjectInfoSet &deselected);
  void paintActiveDecoration();
//  void paintActiveTile();

  void mouseReleasedInImageWidget(QMouseEvent *event);
  void mousePressedInImageWidget(QMouseEvent *event);
  void mouseMovedInImageWidget(QMouseEvent *event);
  void mouseDoubleClickedInImageWidget(QMouseEvent *event);
  void mouseRolledInImageWidget(QWheelEvent *event);

  bool popLeftMenu(const QPoint &pos);
  bool popRightMenu(const QPoint &pos);

  bool showContextMenu(QMenu *menu, const QPoint &pos);

  QMenu* leftMenu();
  QMenu* rightMenu();

  void setInfo(const QString &info);
  void setStackInfo(const QString &info);
  void autoThreshold();
  void setThreshold(int thre);
  void setDepth(int z);
//  void setZQuitely(int z);

  void displayActiveDecoration(bool display = true);
  void request3DVis();
  void requestQuick3DVis();
//  void requestHighresQuick3DVis();
  void requestMerge();
  void requestSetting();
  void requestAutoTracing();
  void closeChildFrame();


  void setViewPort(const ZAffineRect &rect);
  void maximizeViewPort();

  void updateZSpinBoxValue();

  void paintObject(neutu::data3d::ETarget target);
  void paintObject(const std::set<neutu::data3d::ETarget> &targetSet);

  void dump(const QString &msg);

  void hideThresholdControl();

  void resetViewProj();

  void enableCustomCheckBox(
      int index, const QString &text, bool defaultValue,
      QObject *receiver, const char *slot);

  void processCanvasUpdate(
      neutu::data3d::ETarget target, std::shared_ptr<ZSliceCanvas> canvas);

  void notifyViewChanged();
  void syncTransformControl();
  void updateDataInfo();

  void notifySliceSliderPressed();
  void notifySliceSliderReleased();

  void setBlinking(bool on);
  void showAxes(bool on);

signals:
//  void currentSliceChanged(int);
  void viewChanged(const ZStackViewParam &param);
  void viewChanged(int viewId);
//  void viewPortChanged();
  void messageGenerated(const ZWidgetMessage &message);
  void changingSetting();
  void sliceSliderPressed(ZStackView*);
  void sliceSliderReleased(ZStackView*);
  void closingChildFrame();
  void autoTracing();
  void widgetCanvasUpdated(ZPixmap *canvas);
  void canvasUpdated(neutu::data3d::ETarget target,
                     std::shared_ptr<ZSliceCanvas> canvas);
  void sliceAxisChanged();
//  void widgetCanvasUpdated(ZImage *canvas);

private:
  void hideLayout(QLayout *layout, bool removing);

  void updateSliceFromZ(int z);
  void recordViewParam();
//  void updateSliceViewParam();
  void prepareCanvasPainter(ZPixmap *canvas, ZPainter &canvasPainter);

  std::shared_ptr<ZSliceCanvas> getClearCanvas(neutu::data3d::ETarget target);
//  ZSliceCanvas* getClearVisibleCanvas(neutu::data3d::ETarget target);

//  void paintObjectBuffer(ZSliceCanvas &canvas, neutu::data3d::ETarget target);

  void updateObjectBuffer(
      std::shared_ptr<ZSliceCanvas> canvas, neutu::data3d::ETarget target);

  void updateObjectBuffer(
      std::shared_ptr<ZSliceCanvas> canvas, const QList<ZStackObject*> &objList);
  void updateObjectBuffer(
      std::shared_ptr<ZSliceCanvas> canvas, QPainter *painter,
      const QList<ZStackObject*> &objList);
  void updateObjectBuffer(
      std::shared_ptr<ZSliceCanvas> canvas, neutu::data3d::ETarget target,
      const QList<ZStackObject*> &objList);
  void updateObjectBuffer(
      neutu::data3d::ETarget target, const QList<ZStackObject*> &objList);
  void updateObjectBuffer(neutu::data3d::ETarget target);

  template<template<class...> class Container>
  void updateObjectBuffer(const Container<neutu::data3d::ETarget> &targetList);

//  ZStack* getObjectMask(uint8_t maskValue);

  //Stack space offset
  int getZ0() const;
  ZIntPoint getStackOffset() const;

  /*!
   * \brief Get object mask of a certain color
   */
//  ZStack* getObjectMask(neutu::EColor color, uint8_t maskValue);

  void configurePainter(ZStackObjectPainter &painter);

  void logViewParam();
//  void setCentralView(int width, int height);
//  void syncArbViewCenter();

//  void requestWidgetCanvasUpdate();
//  void addWidgetCanvasTask();
//  void notifyWidgetCanvasUpdate(ZPixmap *canvas);

//  void addNonblockCanvasTask(
//      neutu::data3d::ETarget target, const QList<ZStackObject *> &objList);
  void addNonblockCanvasTask(
      std::shared_ptr<ZSliceCanvas> canvas, neutu::data3d::ETarget target,
      const QList<ZStackObject *> &objList);
  void notifyCanvasUpdate(
      neutu::data3d::ETarget target, std::shared_ptr<ZSliceCanvas> canvas);
//  void notifyWidgetCanvasUpdate(ZImage *canvas);

  class ViewParamRecordOnce {
  public:
    ViewParamRecordOnce(ZStackView *view) : m_view(view) {
      view->m_viewParamRecorded = false;
      view->m_viewParamRecordOnce = true;
    }
    ~ViewParamRecordOnce() {
      m_view->m_viewParamRecorded = false;
      m_view->m_viewParamRecordOnce = false;
    }

  private:
    ZStackView *m_view = nullptr;
  };

  void tryAutoRecord();

  void setDepthRangeQuietly(int minZ, int maxZ);

private slots:
  void processTransformChange();
  void processSceneUpdate();

protected:
  int m_viewId;
  //ZStackFrame *m_parent;
  ZSlider *m_depthControl;
  //QSpinBox *m_spinBox;
  QLabel *m_infoLabel;
  QLabel *m_stackLabel;
  QLabel *m_activeLabel;
  ZImage *m_image = NULL;
//  ZPainter m_imagePainter;
  ZImage *m_imageMask = NULL;

//  ZSliceCanvas *m_tileCanvas = NULL;
  ZImageWidget *m_imageWidget;
  ZLabeledSpinBoxWidget *m_depthSpinBox;
  ZCheckBoxGroup *m_checkBoxControl;

  QVBoxLayout *m_layout = nullptr;
//  QHBoxLayout *m_topLayout;
//  QHBoxLayout *m_secondTopLayout;
  ZH3Widget *m_topWidget = nullptr;
  ZH3Widget *m_secondTopWidget = nullptr;
  QHBoxLayout *m_channelControlLayout = nullptr;
  QHBoxLayout *m_toolLayout = nullptr;
  QHBoxLayout *m_ctrlLayout = nullptr;
//  QHBoxLayout *m_zControlLayout;
  ZHWidget *m_bottomWidget = nullptr;
  bool m_scrollEnabled;

  QProgressBar *m_progress = nullptr;
  ZSlider *m_thresholdSlider = nullptr;
  QPushButton *m_autoThreButton = nullptr;

//  QVector<QCheckBox*> m_customCheckBoxList;

  // used to turn on or off each channel
  std::vector<ZBoolParameter*> m_chVisibleState;

  neutu::ESizeHintOption m_sizeHintOption;

//  ZPaintBundle m_paintBundle;
  ZStackObjectPaintSorter m_paintSorter;
  bool m_isRedrawBlocked;
//  QMutex m_mutex;

  ZBodySplitButton *m_splitButton;
  ZMessageManager *m_messageManager;

  bool m_depthFrozen;
  bool m_viewPortFrozen;
  bool m_viewChangeEventBlocked;
  bool m_signalingViewChange = true;
  bool m_ignoringScroll = false;
  bool m_blinking = false;
  int64_t m_scrollPausedTime = 0;
  size_t m_frameCount = 0;

  ZScrollSliceStrategy *m_sliceStrategy;

  QRect m_defaultViewPort;
//  ZStackViewParam m_oldViewParam;
  bool m_viewParamRecorded = false;
  bool m_viewParamRecordOnce = false;
//  ZArbSliceViewParam m_sliceViewParam;
//  ZSliceViewTranform m_mainTransform;
//  ZSliceViewTranform m_prevMainTransform;
  int m_maxViewPort = 0;

  neutu::mvc::ViewInfoFlags m_viewFlags =
      neutu::mvc::ViewInfoFlags(neutu::mvc::ViewInfoFlag::RAW_STACK_COORD) |
      neutu::mvc::ViewInfoFlag::DATA_COORD |
      neutu::mvc::ViewInfoFlag::WINDOW_SCALE |
      neutu::mvc::ViewInfoFlag::IMAGE_VALUE;

//  ZIntCuboid m_currentStackRange;
  ZPoint m_lastModelPosForDataInfo;

  std::shared_ptr<ZStackViewRecorder> m_recorder;
  ZStackViewRecordDialog *m_recordDlg = nullptr;

  static std::atomic<int> m_nextViewId;
};

#endif
