#ifndef ZSPARSEOBJECT_H
#define ZSPARSEOBJECT_H

#include "zdocumentable.h"
#include "zstackdrawable.h"
#include "zobject3dscan.h"
#include "zlabelcolortable.h"

class ZSparseObject : public ZObject3dScan
{
public:
  ZSparseObject();

public:
  //void save(const char *filePath);
  //void load(const char *filePath);
  virtual const std::string& className() const;

  /*!
   * \brief Label a stack with the internal label value.
   */
  void labelStack(ZStack *stack) const;

  /*!
   * \brief Convert the stroke to a stack.
   *
   * Only GREY type is supported. If m_label is bigger than 255, label % 255 is
   * taken.
   */
  //ZStack *toStack() const;

  void setLabel(int label);

  /*
  const ZObject3dScan& getData() const {
    return m_obj;
  }
*/
private:
  static QVector<QColor> constructColorTable();
  const QColor& getLabelColor() const;
  void labelImage(QImage *image) const;

private:
  //ZObject3dScan m_obj;
  int m_label; //Label = 0 is reserved for eraser

  const static ZLabelColorTable m_colorTable;
};

#endif // ZSPARSEOBJECT_H
