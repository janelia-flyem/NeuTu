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
#include "geometry/zcuboid.h"
#include "common/zsharedpointer.h"

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
  virtual ~ZStroke2d() override;

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::STROKE;
  }

  enum class EOperation {
    NONE,
    DELETE, CHANGE_BRUSH_LABEL,
    BRUSH_ON, BRUSH_OFF,
    INCREASE_BRUSH_WIDTH,
    DECREASE_BRUSH_WIDTH,
  };

public:
  virtual void save(const char *filePath);
  virtual bool load(const char *filePath);

  bool display(
      QPainter *painter, const DisplayConfig &config) const override;

  /*
  void display(ZPainter &painter, int slice, zstackobject::EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;
  bool display(QPainter *rawPainter, int z, zstackobject::EDisplayStyle option,
               zstackobject::EDisplaySliceMode sliceMode, neutu::EAxis sliceAxis) const override;
*/

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
  void set(double x, double y, double z);
  void set(const ZPoint &pt);
  void updateWithLast(double x, double y, double z);
  void updateWithLast(const ZPoint &pt);

  /*!
   * \brief Append a point and update the depth too
   *
   * Add a point (\a x, \a y, \a z) from the model space and update the depth
   * to contain this point.
   */
  void append(double x, double y, double z);
  void append(const ZPoint &pt);

  void setLabel(uint64_t label) override;

  /*!
   * \brief Toggle the label.
   */
  void toggleLabel(bool toggling);

  void clear();

  bool isEmpty() const;

  ZStroke2d* clone() const override;

  void addWidth(double dw);

  void setEraser(bool enabled);
  inline bool isEraser() const { return getLabel() == 0; }
  inline void setFilled(bool isFilled) {
    m_isFilled = isFilled;
  }
  void setZ(double z);
  inline double getZ() const { return m_z; }

  /*!
   * \brief Set z-rounding flag
   *
   * Set the flag that automatically rounds Z when it is on. It also rounds the
   * current Z value. Like normal rounding, it sets Z to the closest integer,
   * but for any middle point between two integers, it always goes to the larger
   * one.
   */
  void setZRounding(bool on);

  double inline getWidth() const { return m_width; }

  bool getLastPoint(int *x, int *y) const;
  bool getLastPoint(double *x, double *y) const;
  bool getPoint(double *x, double *y, size_t index) const;
  ZPoint getPoint(size_t index) const;
  ZPoint getFirstPoint() const;
  ZPoint getLastPoint() const;

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

  ZCuboid getBoundBox() const override;

  ZObject3d* toObject3d() const;

  void labelStack(ZStack *stack) const;
  void labelStack(ZStack *stack, int ignoringValue) const;

  void labelProjStack(ZStack *stack) const;
  void labelProjStack(ZStack *stack, int value) const;

  ZJsonObject toJsonObject() const;
  void loadJsonObject(const ZJsonObject &obj);

  bool isSliceVisible(int z, neutu::EAxis sliceAxis) const override;

  inline void setPenetrating(bool p) {
    m_isPenetrating = p;
  }
  bool isPenetrating() const {
    return m_isPenetrating;
  }

  inline void hideStart(bool s) {
    m_hideStart = s;
  }

  bool hitTest(double x, double y, neutu::EAxis axis) const;
  bool hitTest(double x, double y, double z) const;

//  using ZStackObject::hit; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  bool hit(double x, double y, neutu::EAxis axis) override;
  bool hit(double x, double y, double z) override;

  void boundBox(ZIntCuboid *box) const override;

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

  void setLabel_(uint64_t label);

private:
  std::vector<QPointF> m_pointArray;
  double m_width;

//  int m_label; //Label = 0 is reserved for eraser
  uint64_t m_originalLabel; //for label toggling
  double m_z;
  bool m_roundingZ = true;

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
