/**@file zstackview.h
 * @brief Stack view
 * @author Ting Zhao
 */

#ifndef _ZSTACKVIEW_H_
#define _ZSTACKVIEW_H_

#include <QImage>
#include <QWidget>
#include <QPixmap>

#include "zstackframe.h"
#include "zparameter.h"
#include <vector>
#include "tz_image_lib_defs.h"
#include "neutube.h"
#include "zpaintbundle.h"
#include "zsharedpointer.h"
#include "zmessageprocessor.h"
#include "zpainter.h"

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
class ZStackDoc;
class ZStack;
class ZStackViewParam;
class ZMessageManager;
class ZBodySplitButton;
class ZStackMvc;

/*!
 * \brief The ZStackView class shows 3D data slice by slice
 *
 * ZStackView provides a widget of showing 3D data slice by slice. It displays
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

  /*!
   * \brief Update image screen
   *
   * Update the screen by assuming that all the canvas buffers are ready.
   */
  void updateImageScreen();

  //void updateScrollControl();

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
  //inline ZImageWidget* screen() { return m_imageWidget; }

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

  /*!
   * \brief Get stack data from the buddy document
   */
  ZStack *stackData();

  //set up the view after the document is ready
  //void prepareDocument();

  virtual void resizeEvent(QResizeEvent *event);

  QStringList toStringList() const;

  //void setImageWidgetCursor(const QCursor &cursor);
  /*!
   * \brief Set the cursor of the image screen
   */
  void setScreenCursor(const QCursor &cursor);

  //void resetScreenCursor();
  int getIntensityThreshold();

  //void open3DWindow();
  void takeScreenshot(const QString &filename);

  bool isDepthChangable();

  void paintStackBuffer();
  void paintMaskBuffer();
  void paintObjectBuffer();
  bool paintTileCanvasBuffer();

  //void paintObjectBuffer(ZImage *canvas, ZStackObject::ETarget target);

  void paintObjectBuffer(ZPainter &painter, ZStackObject::ETarget target);

  void paintActiveDecorationBuffer();

  ZStack* getObjectMask(uint8_t maskValue);

  /*!
   * \brief Get object mask of a certain color
   */
  ZStack* getObjectMask(NeuTube::EColor color, uint8_t maskValue);

  ZStack* getStrokeMask(uint8_t maskValue);


  void exportObjectMask(const std::string &filePath);
  void exportObjectMask(NeuTube::EColor color, const std::string &filePath);

  inline void setSizeHintOption(NeuTube::ESizeHintOption option) {
    m_sizeHintOption = option;
  }

  inline void blockRedraw(bool state) {
    m_isRedrawBlocked = state;
  }

public:
  bool isViewPortFronzen() const;
  bool isDepthFronzen() const;

  void setViewPortFrozen(bool state);
  void setDepthFrozen(bool state);

public: //Message system implementation
  class MessageProcessor : public ZMessageProcessor {
  public:
    void processMessage(ZMessage *message, QWidget *host) const;
  };

  void enableMessageManager();

public slots:
  void updateView();
  void redraw();
  void redrawObject();
  //void updateData(int nslice, int threshold = -1);
  //void updateData();
  //void updateSlice(int nslide);
  //void viewThreshold(int threshold);
  void updateThresholdSlider();
  void updateSlider();
  void updateChannelControl();
  void processDepthSliderValueChange(int sliceIndex);

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

  void setInfo(QString info);
  void autoThreshold();
  void setThreshold(int thre);

  void displayActiveDecoration(bool display = true);
  void request3DVis();
  void requestQuick3DVis();
  void requestHighresQuick3DVis();
  void requestMerge();

signals:
  void currentSliceChanged(int);
  void viewChanged(ZStackViewParam param);
  void viewPortChanged();

public:
  static QImage::Format stackKindToImageFormat(int kind);
  double getZoomRatio() const;
  void setInfo();
  bool isImageMovable() const;

  int getZ(NeuTube::ECoordinateSystem coordSys) const;
  QRect getViewPort(NeuTube::ECoordinateSystem coordSys) const;
  ZStackViewParam getViewParameter(
      NeuTube::ECoordinateSystem coordSys = NeuTube::COORD_STACK) const;

  void setView(const ZStackViewParam &param);
  void setViewPortOffset(int x, int y);

  void paintMultiresImageTest(int resLevel);
  void customizeWidget();

  void notifyViewPortChanged();


public: //Change view parameters
  void increaseZoomRatio();
  void decreaseZoomRatio();

private:
  void clearCanvas();
  void updateCanvas();
  void updateMaskCanvas();
  void clearObjectCanvas();
  void clearTileCanvas();
  void updateObjectCanvas();
  void updateTileCanvas();
  void updateActiveDecorationCanvas();
  void updatePaintBundle();

  void connectSignalSlot();

  void hideThresholdControl();

  QSize getCanvasSize() const;

  //help functions
  void paintSingleChannelStackSlice(ZStack *stack, int slice);
  void paintMultipleChannelStackSlice(ZStack *stack, int slice);
  void paintSingleChannelStackMip(ZStack *stack);
  void paintMultipleChannelStackMip(ZStack *stack);

  void notifyViewChanged(const ZStackViewParam &param);
  void notifyViewChanged();

  void init();

private:
  //ZStackFrame *m_parent;
  ZSlider *m_depthControl;
  //QSpinBox *m_spinBox;
  QLabel *m_infoLabel;
  QLabel *m_activeLabel;
  ZImage *m_image;
  ZPainter m_imagePainter;
  ZImage *m_imageMask;
  ZImage *m_objectCanvas;
  ZPainter m_objectCanvasPainter;
  ZPainter m_tileCanvasPainter;
  ZImage *m_activeDecorationCanvas;
  QPixmap *m_tileCanvas;
  ZImageWidget *m_imageWidget;
  QVBoxLayout *m_layout;
  QHBoxLayout *m_topLayout;
  QHBoxLayout *m_secondTopLayout;
  QHBoxLayout *m_channelControlLayout;
  QHBoxLayout *m_ctrlLayout;
  bool m_scrollEnabled;

  QProgressBar *m_progress;
  ZSlider *m_thresholdSlider;
  QPushButton *m_autoThreButton;

  // used to turn on or off each channel
  std::vector<ZBoolParameter*> m_chVisibleState;

  NeuTube::ESizeHintOption m_sizeHintOption;

  ZPaintBundle m_paintBundle;
  bool m_isRedrawBlocked;
  QMutex m_mutex;

  ZBodySplitButton *m_splitButton;
  ZMessageManager *m_messageManager;

  bool m_depthFrozen;
  bool m_viewPortFrozen;
};

#endif
