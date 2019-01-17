#ifndef ZSTROKE2D_H
#define ZSTROKE2D_H

#include <QPointF>
#include <QVector>
#include <vector>
#include <QColor>
#include <QImage>

#include "zstackobject.h"
#include "c_stack.h"
#include "zlabelcolortable.h"
#include "zcuboid.h"
#include "zsharedpointer.h"

class ZStack;
class ZObject3d;
class ZJsonObject;
class ZStroke2d;

typedef ZSharedPointer<ZStroke2d> ZStroke2dPtr;

/*!
 * \brief The class of plane strokes.
 *
 * ZStroke2d represents the class of plane strokes, each composed of a polyline
 * with a certain width. A plane stroke also has a Z postion, which is an
 * integer value specifying which plane the stroke is located in a 3D stack.
 */
class ZStroke2d : public ZStackObject {
public:
  ZStroke2d();
//  ZStroke2d(const ZStroke2d &stroke);
  virtual ~ZStroke2d();

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::STROKE;
  }

  enum EOperation {
    OPERATION_NULL,
    OPERATION_DELETE, OPERATION_CHANGE_BRUSH_LABEL,
    OPERATION_BRUSH_ON, OPERATION_BRUSH_OFF,
    OPERATION_INCREASE_BRUSH_WIDTH,
    OPERATION_DECREASE_BRUSH_WIDTH,
  };

public:
  virtual void save(const char *filePath);
  virtual bool load(const char *filePath);

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutube::EAxis sliceAxis) const;
  bool display(QPainter *rawPainter, int z, EDisplayStyle option,
               EDisplaySliceMode sliceMode, neutube::EAxis sliceAxis) const;

  void labelBinary(Stack *stack) const;

  /*!
   * \brief Label a stack with the internal label value.
   */
  void labelGrey(Stack *stack) const;

  /*!
   * \brief Label a stack with a given label value.
   *
   * The label is automatically set to 0 if it is negative or 255 if it is
   * larger than 255. \a stack must have GREY kind; otherwise nothing will be
   * done.
   */
  void labelGrey(Stack *stack, int label) const;
  void labelGrey(Stack *stack, int label, int ignoringValue) const;

//  virtual const std::string& className() const;

  inline void setWidth(double width) { m_width = width; }
  void append(double x, double y);
  void set(const QPoint &pt);
  void set(double x, double y);
  void setLast(double x, double y);
  void setLabel(uint64_t label);

  /*!
   * \brief Toggle the label.
   */
  void toggleLabel(bool toggling);

  void clear();

  bool isEmpty() const;

  ZStroke2d* clone();

  void addWidth(double dw);

  void setEraser(bool enabled);
  inline bool isEraser() const { return getLabel() == 0; }
  inline void setFilled(bool isFilled) {
    m_isFilled = isFilled;
  }
  inline void setZ(int z) { m_z = z; }
  inline int getZ() const { return m_z; }

  double inline getWidth() const { return m_width; }

  bool getLastPoint(int *x, int *y) const;
  bool getLastPoint(double *x, double *y) const;
  bool getPoint(double *x, double *y, size_t index) const;

  inline size_t getPointNumber() const { return m_pointArray.size(); }

  void print() const;

  /*!
   * \brief Translate the stroke
   */
  void translate(const ZPoint &offset);
  void translate(const ZIntPoint &offset);
  void scale(double sx, double sy, double sz);
  void downsample(const ZIntPoint &dsIntv);

  /*!
   * \brief Convert the stroke to a stack.
   *
   * Only GREY type is supported. If m_label is bigger than 255, label % 255 is
   * taken.
   */
  ZStack *toStack() const;

  ZStack *toBinaryStack() const;

  ZCuboid getBoundBox() const;

  ZObject3d* toObject3d() const;

  void labelStack(ZStack *stack) const;
  void labelStack(ZStack *stack, int ignoringValue) const;

  void labelProjStack(ZStack *stack) const;
  void labelProjStack(ZStack *stack, int value) const;

  ZJsonObject toJsonObject() const;
  void loadJsonObject(const ZJsonObject &obj);

  bool isSliceVisible(int z, neutube::EAxis sliceAxis) const;

  inline void setPenetrating(bool p) {
    m_isPenetrating = p;
  }
  bool isPenetrating() const {
    return m_isPenetrating;
  }

  inline void hideStart(bool s) {
    m_hideStart = s;
  }

  bool hitTest(double x, double y, neutube::EAxis axis) const;
  bool hitTest(double x, double y, double z) const;

//  using ZStackObject::hit; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  bool hit(double x, double y, neutube::EAxis axis);
  bool hit(double x, double y, double z);

  void boundBox(ZIntCuboid *box) const;

  static QColor GetLabelColor(int label);

  void decimate();

private:
  static QVector<QColor> constructColorTable();
  const QColor& getLabelColor() const;
  void labelImage(QImage *image) const;
  ZStack* toLabelStack(int label) const;

  void decimate(size_t first, size_t last, double eps,
                std::vector<bool> &marker);
  double pointLinesegDistance(
      const QPointF &x, const QPointF &x0, const QPointF x1);

private:
  std::vector<QPointF> m_pointArray;
  double m_width;

//  int m_label; //Label = 0 is reserved for eraser
  uint64_t m_originalLabel; //for label toggling
  int m_z;

  //bool m_isEraser;
  //Customized styles
  bool m_isFilled;
  bool m_hideStart;
  bool m_isPenetrating; //Visible on any slice

  static const double m_minWidth;
  static const double m_maxWidth;
  static const double m_zFadeSpan;

  const static ZLabelColorTable m_colorTable;
  //const static QVector<QColor> m_colorTable;
  //const static QColor m_blackColor;
};

#endif // ZSTROKE2D_H
