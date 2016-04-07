#ifndef ZSPARSEOBJECT_H
#define ZSPARSEOBJECT_H

#include "zdocumentable.h"
#include "zstackdrawable.h"
#include "zobject3dscan.h"
#include "zlabelcolortable.h"
#include "zopenvdbobject.h"
#include "bigdata/zstackblockgrid.h"

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

  void display(ZPainter &painter, int z, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

  void append(const ZObject3dScan &obj);


public: //Intensity functions
  //void setVoxelValue(ZStack *stack);
  int getVoxelValue(int x, int y, int z) const;

  inline ZStackBlockGrid* getStackGrid() {
    return &m_stackGrid;
  }

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
  //ZOpenVdbObject m_voxelValueObject;
#endif

  ZStackBlockGrid m_stackGrid; //Intensity values

  const static ZLabelColorTable m_colorTable;
};

#endif // ZSPARSEOBJECT_H
