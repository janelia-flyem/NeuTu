#ifndef ZSPARSEOBJECT_H
#define ZSPARSEOBJECT_H

//#include "zdocumentable.h"
//#include "zstackdrawable.h"
#include <memory>

#include "zobject3dscan.h"
#include "zlabelcolortable.h"
#include "zopenvdbobject.h"
#include "bigdata/zstackblockgrid.h"

class ZStackSource;

class ZSparseObject : public ZObject3dScan
{
public:
  ZSparseObject();

public:

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::SPARSE_OBJECT;
  }

  //void save(const char *filePath);
  //void load(const char *filePath);
//  virtual const std::string& className() const;

  void setStackSource(std::shared_ptr<ZStackSource> source);


  /*!
   * \brief Label a stack with the internal label value.
   */
  void labelStack(ZStack *stack) const;

  void setLabel(uint64_t label) override;

  bool display(QPainter *painter, const DisplayConfig &config) const override;
#if 0
  bool display(
      QPainter */*painter*/, const DisplayConfig &/*config*/) const {
    return false;
  }
#endif
  /*
  void display(ZPainter &painter, int z, EDisplayStyle option,
               neutu::EAxis sliceAxis) const;
               */

  void append(const ZObject3dScan &obj);

public: //Intensity functions
  //void setVoxelValue(ZStack *stack);
  int getVoxelValue(int x, int y, int z) const;

//  inline ZStackBlockGrid* getStackGrid() {
//    return &m_stackGrid;
//  }

  /*
  const ZObject3dScan& getData() const {
    return m_obj;
  }
*/
private:
//  static QVector<QColor> constructColorTable();
//  const QColor& getLabelColor() const;
//  void labelImage(QImage *image) const;

private:
  //ZObject3dScan m_obj;
//  int m_label; //Label = 0 is reserved for eraser

#if defined(_USE_OPENVDB_)
  //ZOpenVdbObject m_voxelValueObject;
#endif

//  ZStackBlockGrid m_stackGrid; //Intensity values
  std::shared_ptr<ZStackSource> m_stackSource;

  const static ZLabelColorTable m_colorTable;
};

#endif // ZSPARSEOBJECT_H
