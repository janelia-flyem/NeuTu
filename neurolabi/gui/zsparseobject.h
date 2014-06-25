#ifndef ZSPARSEOBJECT_H
#define ZSPARSEOBJECT_H

#include "zdocumentable.h"
#include "zstackdrawable.h"
#include "zobject3dscan.h"
#include "zlabelcolortable.h"
#include "zopenvdbobject.h"

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

  void setLabel(int label);

  void display(ZPainter &painter, int z, Display_Style option) const;

  void append(const ZObject3dScan &obj);

  void setVoxelValue(ZStack *stack);

  int getVoxelValue(int x, int y, int z) const;

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

#if defined(_USE_OPENVDB_)
  ZOpenVdbObject m_voxelValueObject;
#endif

  const static ZLabelColorTable m_colorTable;
};

#endif // ZSPARSEOBJECT_H
