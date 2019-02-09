/**@file zstackview.h
 * @brief Stack view
 * @author Ting Zhao
 */

#ifndef _ZSTACKVIEW_H_
#define _ZSTACKVIEW_H_

#include <QImage>
#include <QWidget>
#include <QPixmap>
#include <vector>
#include <QCheckBox>

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

//#include "zstackdoc.h"

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
class ZStackView : public QWidget {
  Q_OBJECT

public:
  explicit ZStackView(ZStackFrame *parent = 0);
  explicit ZStackView(QWidget *parent = 0);
  virtual ~ZStackView();

public:
  /*!
   * \brief Reset the view status
   *
   * This function is for synchorinizing the view controls such as depth slider
   * and channel controls with stack update in the document.
   */
  void reset(bool updatingScreen = true);

  enum class EMode {
    NORMAL, IMAGE_ONLY, PLAIN_IMAGE
  };

  void configure(EMode mode);

  enum class EUpdateOption {
    NONE, //No update
    QUEUED, //Put updating request in a queue
    DIRECT //Update immediately
  };

  /*!
   * \brief Update image screen
   *
   * Update the screen by assuming that all the canvas buffers are ready.
   */
  void updateImageScreen(EUpdateOption option);

  /*!
   * \brief Restore the view from a bad setting
   *
   * The definition of a bad setting is provided by ZImageWidget::isBadView().
   */
  void restoreFromBadView();

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

  QSize sizeHint() const;

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
  int getCurrentZ() const;

  //int threshold();

  void setSliceAxis(neutu::EAxis axis);
  neutu::EAxis getSliceAxis() const { return m_sliceAxis; }
  ZAffinePlane getAffinePlane() const;

  /*!
   * \brief Get stack data from the buddy document
   */
  ZStack *stackData() const;

  /*!
   * \brief set up the view after the document is ready
   */
  void prepareDocument();

  virtual void resizeEvent(QResizeEvent *event);
  virtual void showEvent(QShowEvent *event);

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

  void paintStackBuffer();
  void paintMaskBuffer();

  /*!
   * \brief Update the buffer of object canvas.
   */
  void paintObjectBuffer();
  bool paintTileCanvasBuffer();

  //void paintObjectBuffer(ZImage *canvas, ZStackObject::ETarget target);

  void paintObjectBuffer(ZStackObject::ETarget target);
  void paintObjectBuffer(ZPainter &painter, ZStackObject::ETarget target);

  void paintActiveDecorationBuffer();
  void paintDynamicObjectBuffer();


  ZStack* getStrokeMask(uint8_t maskValue);
  ZStack* getStrokeMask(neutu::EColor color);


  void exportObjectMask(const std::string &filePath);
  void exportObjectMask(neutu::EColor color, const std::string &filePath);

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


  void setViewPortFrozen(bool state);
  void setDepthFrozen(bool state);
  void blockViewChangeEvent(bool state);

  void zoomTo(int x, int y, int z);
  void zoomTo(const ZIntPoint &pt);

  void zoomTo(int x, int y, int z, int w);

  void printViewParam() const;
  void setMaxViewPort(int s) {
    m_maxViewPort = s;
  }

  void setDefaultViewPort(const QRect &rect) {
    m_defaultViewPort = rect;
  }

public: //Message system implementation
  class MessageProcessor : public ZMessageProcessor {
  public:
    void processMessage(ZMessage *message, QWidget *host) const;
  };

  void enableMessageManager();



public:
  static QImage::Format stackKindToImageFormat(int kind);
  double getCanvasWidthZoomRatio() const;
  double getCanvasHeightZoomRatio() const;
  double getProjZoomRatio() const;
  void setInfo();
  bool isImageMovable() const;

  int getZ(neutu::ECoordinateSystem coordSys) const;
  ZIntPoint getCenter(
      neutu::ECoordinateSystem coordSys = neutu::ECoordinateSystem::STACK) const;

  QRect getViewPort(neutu::ECoordinateSystem coordSys) const;
  ZStackViewParam getViewParameter() const;
  ZStackViewParam getViewParameter(
      neutu::ECoordinateSystem coordSys,
      neutu::View::EExploreAction action = neutu::View::EExploreAction::EXPLORE_UNKNOWN) const;

  QRectF getProjRegion() const;
  ZViewProj getViewProj() const;

  //Get transform from view port to proj region
  ZStTransform getViewTransform() const;

  /*!
   * \brief Set the viewport offset
   *
   * (\a x, \a y) is supposed to the world coordinates of the first corner of
   * the viewport. The real position will be adjusted to fit in the canvas.
   */
  void setViewPortOffset(int x, int y);

  void setViewPortCenter(int x, int y, int z, neutu::EAxisSystem system);
  void setViewPortCenter(const ZIntPoint &center, neutu::EAxisSystem system);

  void setViewProj(int x0, int y0, double zoom);
  void setViewProj(const QPoint &pt, double zoom);
  void setViewProj(const ZViewProj &vp);
  void updateViewParam(const ZStackViewParam &param);
  void updateViewParam(const ZArbSliceViewParam &param);
  void resetViewParam(const ZArbSliceViewParam &param);

  ZIntPoint getViewCenter() const;

  void paintMultiresImageTest(int resLevel);
  void customizeWidget();

  void addHorizontalWidget(QWidget *widget);
  void addHorizontalWidget(QSpacerItem *spacer);

//  void notifyViewPortChanged();

  bool isViewChanged(const ZStackViewParam &param) const;
  void processViewChange(bool redrawing, bool depthChanged);
  void processViewChange(bool redrawing);
//  void processViewChange(const ZStackViewParam &param);

  void setHoverFocus(bool on);
  void setSmoothDisplay(bool on);

  void notifyViewChanged(const ZStackViewParam &param);
  void notifyViewChanged();

  /*!
   * \brief Get the size of the image window.
   *
   * It's the size of the widget in which any dot can be painted by an image
   * pixel.
   */
  QSize getScreenSize() const;

  void addToolButton(QPushButton *button);
  void removeToolButton(QPushButton *button);


public: //Change view parameters
  void move(const QPoint& src, const QPointF &dst);
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
  void highlightPosition(int x, int y, int z);
  void highlightPosition(const ZIntPoint &pt);

  void updateContrastProtocal();

public: //View parameters for arbitrary plane
  ZStackViewParam getViewParameter(const ZArbSliceViewParam &param) const;
  ZArbSliceViewParam getSliceViewParam() const;

  void updateDataInfo(const QPoint &widgetPos);
//  void setSliceViewParam(const ZArbSliceViewParam &param);
//  void showArbSliceViewPort();

protected:
  ZIntCuboid getViewBoundBox() const;
  ZIntCuboid getCurrentStackRange() const;
  int getDepth() const;

  void clearCanvas();
  template<typename T>
  void resetCanvasWithStack(T &canvas, ZPainter *painter);

  virtual void resetCanvasWithStack(
      ZMultiscalePixmap &canvas, ZPainter *painter);

//  void reloadTileCanvas();
//  bool reloadObjectCanvas(bool repaint = false);
  void reloadCanvas();

  virtual void updateImageCanvas();
  void updateMaskCanvas();
  void clearObjectCanvas();
  void clearTileCanvas();
  void updateObjectCanvas();
  void updateTileCanvas();
  void updateDynamicObjectCanvas();
  void updateActiveDecorationCanvas();
  void updatePaintBundle();
  void updateCanvas(ZStackObject::ETarget target);

  ZPainter* getTileCanvasPainter();
  ZPainter* getObjectCanvasPainter();

  ZPixmap* updateProjCanvas(ZPixmap *canvas, ZPainter *painter);
  ZPixmap* updateViewPortCanvas(ZPixmap *canvas);

  void connectSignalSlot();

  /*!
   * \brief Get the size of the canvas.
   *
   * It is usually the same as the size of a stack slice.
   */
  QSize getCanvasSize() const;

  //help functions
  virtual void paintSingleChannelStackSlice(ZStack *stack, int slice);
  void paintMultipleChannelStackSlice(ZStack *stack, int slice);
  void paintSingleChannelStackMip(ZStack *stack);
  void paintMultipleChannelStackMip(ZStack *stack);

  std::set<ZStackObject::ETarget> updateViewData(const ZStackViewParam &param);
  std::set<ZStackObject::ETarget> updateViewData();

  void init();

  ZPainter* getPainter(ZStackObject::ETarget target);
  ZPixmap* getCanvas(ZStackObject::ETarget target);
  void setCanvasVisible(ZStackObject::ETarget target, bool visible);
//  void resetDepthControl();

  void setSliceRange(int minSlice, int maxSlice);

  bool event(QEvent *event);

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
  void paintObject();
  void paintObject(QList<ZStackObject *> selected,
                   QList<ZStackObject *> deselected);
  void paintActiveDecoration();
  void paintActiveTile();

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
  void setZ(int z);
  void setZQuitely(int z);

  void displayActiveDecoration(bool display = true);
  void request3DVis();
  void requestQuick3DVis();
  void requestHighresQuick3DVis();
  void requestMerge();
  void requestSetting();
  void requestAutoTracing();
  void closeChildFrame();


  void setView(const ZStackViewParam &param);
  void setViewPort(const QRect &rect);
  void maximizeViewPort();

  void updateZSpinBoxValue();

  void paintObject(ZStackObject::ETarget target);
  void paintObject(const std::set<ZStackObject::ETarget> &targetSet);

  void dump(const QString &msg);

  void hideThresholdControl();

  void setDynamicObjectAlpha(int alpha);
  void resetViewProj();

  void enableCustomCheckBox(
      int index, const QString &text, QObject *receiver, const char *slot);

signals:
//  void currentSliceChanged(int);
  void viewChanged(ZStackViewParam param);
//  void viewPortChanged();
  void messageGenerated(const ZWidgetMessage &message);
  void changingSetting();
  void sliceSliderPressed();
  void sliceSliderReleased();
  void closingChildFrame();
  void autoTracing();

private:
  void hideLayout(QLayout *layout, bool removing);

  void updateSliceFromZ(int z);
  void recordViewParam();
  void updateSliceViewParam();
  void prepareCanvasPainter(ZPixmap *canvas, ZPainter &canvasPainter);

  ZStack* getObjectMask(uint8_t maskValue);

  //Stack space offset
  int getZ0() const;
  ZIntPoint getStackOffset() const;

  /*!
   * \brief Get object mask of a certain color
   */
  ZStack* getObjectMask(neutu::EColor color, uint8_t maskValue);

  void configurePainter(ZStackObjectPainter &painter);

  void logViewParam();
//  void setCentralView(int width, int height);

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

protected:
  //ZStackFrame *m_parent;
  ZSlider *m_depthControl;
  //QSpinBox *m_spinBox;
  QLabel *m_infoLabel;
  QLabel *m_stackLabel;
  QLabel *m_activeLabel;
  ZImage *m_image = NULL;
//  ZPainter m_imagePainter;
  ZImage *m_imageMask = NULL;
//  ZPixmap *m_objectCanvas;
  ZPixmap *m_dynamicObjectCanvas = NULL;
  double m_dynamicObjectOpacity;

  ZPixmap *m_objectCanvas = NULL;
  ZPainter m_objectCanvasPainter;

  neutu::EAxis m_sliceAxis;

  ZPainter m_tileCanvasPainter;
  ZPixmap *m_activeDecorationCanvas = NULL;
//  ZMultiscalePixmap m_tileCanvas;
  ZPixmap *m_tileCanvas = NULL;
  ZImageWidget *m_imageWidget;
  ZLabeledSpinBoxWidget *m_zSpinBox;

  QVBoxLayout *m_layout;
  QHBoxLayout *m_topLayout;
  QHBoxLayout *m_secondTopLayout;
  QHBoxLayout *m_channelControlLayout;
  QHBoxLayout *m_toolLayout = nullptr;
  QHBoxLayout *m_ctrlLayout;
  QHBoxLayout *m_zControlLayout;
  bool m_scrollEnabled;

  QProgressBar *m_progress;
  ZSlider *m_thresholdSlider;
  QPushButton *m_autoThreButton;

  QVector<QCheckBox*> m_customCheckBoxList;

  // used to turn on or off each channel
  std::vector<ZBoolParameter*> m_chVisibleState;

  neutu::ESizeHintOption m_sizeHintOption;

  ZPaintBundle m_paintBundle;
  bool m_isRedrawBlocked;
//  QMutex m_mutex;

  ZBodySplitButton *m_splitButton;
  ZMessageManager *m_messageManager;

  bool m_depthFrozen;
  bool m_viewPortFrozen;
  bool m_viewChangeEventBlocked;

  ZScrollSliceStrategy *m_sliceStrategy;

  QRect m_defaultViewPort;
  ZStackViewParam m_oldViewParam;
  bool m_viewParamRecorded = false;
  bool m_viewParamRecordOnce = false;
  ZArbSliceViewParam m_sliceViewParam;
  int m_maxViewPort = 0;

  ZIntCuboid m_currentStackRange;
};

#endif
