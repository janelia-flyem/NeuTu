/**@file zobject3d.h
 * @brief 3D object class
 * @author Ting Zhao
 */
#ifndef ZOBJECT3D_H
#define ZOBJECT3D_H

#include "zqtheader.h"

#include <vector>
#include <string>

#include "tz_object_3d.h"
#include "zstackobject.h"
#include "tz_fmatrix.h"
#include "zpoint.h"
#include "zintpoint.h"
#include "tz_stack_utils.h"

#ifndef INT_VOXEL_TYPE
#error "Incompatible voxel type"
#endif

class ZStack;
class ZObject3dArray;
class ZJsonObject;

/*!
 * \brief The class of a 3D object
 *
 * A 3D object is defined as a set of 3D voxels.
 */
class ZObject3d : public ZStackObject {
public:
  ZObject3d(Object_3d *obj = NULL);
  ZObject3d(const std::vector<size_t> &indexArray, int width, int height,
            int dx, int dy, int dz);
  virtual ~ZObject3d();

  virtual const std::string& className() const;

  virtual void save(const char *filePath);
  virtual bool load(const char *filePath);
  virtual void display(
      ZPainter &painter, int slice, EDisplayStyle option,
      NeuTube::EAxis sliceAxis) const;

public:
  /*!
   * \brief Number of voxels of the object
   */
  inline size_t size() const { return m_voxelArray.size() / 3; }

  /*!
   * \brief Set the size of the object
   *
   * \a s Number of voxels.
   */
  inline void setSize(size_t s) { m_voxelArray.resize(s * 3); }

  inline int getX(size_t index) const { return m_voxelArray[index * 3]; }
  inline int getY(size_t index) const { return m_voxelArray[index * 3 + 1]; }
  inline int getZ(size_t index) const { return m_voxelArray[index * 3 + 2]; }

  void set(int index, int getX, int getY, int getZ);
  void set(int index, Voxel_t voxel);
  void set(int index, size_t voxelIndex, int width, int height,
           int dx, int dy, int dz);
  inline void setX(int index, int x) { m_voxelArray[index * 3] = x; }
  inline void setY(int index, int y) { m_voxelArray[index * 3 + 1] = y; }
  inline void setZ(int index, int z) { m_voxelArray[index * 3 + 2] = z; }

  inline int getLabel() const { return m_label; }
  inline void setLabel(int label) { m_label = label; }

  bool isEmpty() const;

  void append(int getX, int getY, int getZ);

  /*!
   * \brief Append an object to the current object.
   *
   * \a srcOffset is the voxel index offset of \a obj for the start of appending.
   */
  void append(const ZObject3d &obj, size_t srcOffset = 0);

  /*!
   * \brief Append an object from the backward direction
   *
   * \param srcOffset The offset counted from the end. 0 is the last voxel, 1 is
   *   the last - 1 voxel and so on.
   */
  void appendBackward(const ZObject3d &obj, size_t srcOffset = 0);

  void setLine(ZPoint start, ZPoint end);

  Object_3d* c_obj() const;
  void setFromCObj(const Object_3d *obj);

  void labelStack(Stack *stack) const;
  void labelStack(Stack *stack, int label = 1) const;
  void labelStack(Stack *stack, int label, int dx, int dy, int dz) const;

  /*!
   * \brief Label a stack
   *
   * \a offset is the offset of the object while labeling in the original
   * space.
   */
  void labelStack(Stack *stack, int label, int dx, int dy, int dz,
                  int xIntv, int yIntv, int zIntv) const;

  void labelStack(ZStack *stack) const;
  void labelStack(ZStack *stack, int label) const;

  //For tbar detection specifically
  ZPoint computeCentroid(FMatrix *matrix);

  template <typename T>
  std::vector<T> toIndexArray(int width, int height, int depth);
  template <typename T>
  std::vector<T> toIndexArray(int width, int height, int depth,
                              int offsetX, int offsetY, int offsetZ);

  void sortByIndex();

  //static bool compareByIndex(const )

  inline void clear() { m_voxelArray.clear(); }

  void translate(const ZIntPoint &pt);
  void translate(int getX, int getY, int getZ);

  inline std::vector<int> voxelArray() { return m_voxelArray; }

  void exportSwcFile(std::string filePath);
  void exportCsvFile(std::string filePath);

  ZObject3d *clone() const;

  double averageIntensity(const Stack *stack) const;

  void print();

  inline int lastX() const { return *(m_voxelArray.end() - 3); }
  inline int lastY() const { return *(m_voxelArray.end() - 2); }
  inline int lastZ() const { return *(m_voxelArray.end() - 1); }

  void getRange(int *corner) const;

  ZObject3dArray* growLabel(const ZObject3d &seed, int growLevel = -1);

  Stack* toStack(int *offset = NULL) const;
  ZStack* toStackObject() const;
  ZStack* toLabelStack() const;
  void drawStack(ZStack *stack) const;

  /*!
   * \brief Draw a stack with downsampling ratios
   */
  void drawStack(ZStack *stack, int xIntv, int yIntv, int zIntv) const;

  /*!
   * \brief Draw a stack array
   *
   * The first stack will be drawn by the red color, the second by the green
   * color and the third by the blue color. \a offset is the offset of the
   * stacks. All the stacks in \a stackArray are supposed to have the same size.
   * \a offset is the origin of the stacks in the downsampled space.
   */
  void drawStack(const std::vector<Stack*> &stackArray, const int *offset,
                 int xIntv, int yIntv, int zIntv) const;

  bool loadStack(const Stack *stack, int threshold = 0);
  bool loadStack(const ZStack *stack, int threshold = 0);

  ZPoint getCenter() const;
  double getRadius() const;

  ZIntPoint getCentralVoxel() const;

  /*!
   * \brief Duplicate the object voxels across planes
   *
   * duplicateAcrossZ() first project all voxels into the first plane
   * (0-indexed) and then duplicate the voxels in every other plane within the
   * range of [1, \a depth - 1]. It does nothing if \a depth <= 0.
   *
   * \param depth Number of planes to copy.
   */
  void duplicateAcrossZ(int depth);

  /*!
   * \brief Reverse the object.
   *
   * The operation reverse the order of the voxels.
   */
  void reverse();

  /*!
   * \brief Upsample an object
   */
  void upSample(int xIntv, int yIntv, int zIntv);

  ZJsonObject toJsonObject() const;
  void loadJsonObject(const ZJsonObject &jsonObj);

  using ZStackObject::hit; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  bool hit(double x, double y);
  bool hit(double x, double y, double z);

  bool hasHitVoxel() const;
  ZIntPoint getHitVoxel() const;

  void getBoundBox(ZIntCuboid *box) const;

private:
  int m_conn;
  int m_label;
  std::vector<int> m_voxelArray;
  mutable Object_3d m_objWrapper;

  int m_hitVoxelIndex;
};

template <typename T>
std::vector<T> ZObject3d::toIndexArray(int width, int height, int depth)
{
  std::vector<T> indexArray(size(), -1);
  for (size_t i = 0; i < size(); i++) {
    indexArray[i] = Stack_Util_Offset(getX(i), getY(i), getZ(i), width, height, depth);
  }

  return indexArray;
}

template <typename T>
std::vector<T> ZObject3d::toIndexArray(int width, int height, int depth,
                                       int offsetX, int offsetY, int offsetZ)
{
  std::vector<T> indexArray(size(), -1);
  for (size_t i = 0; i < size(); i++) {
    indexArray[i] = Stack_Util_Offset(getX(i) + offsetX, getY(i) + offsetY,
                                      getZ(i) + offsetZ, width, height, depth);
  }

  return indexArray;
}
#endif // ZOBJECT3D_H
