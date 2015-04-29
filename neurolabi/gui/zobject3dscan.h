#ifndef ZOBJECT3DSCAN_H
#define ZOBJECT3DSCAN_H

#include <vector>
#include <string>
#include <set>
#include <map>
#include <utility>

#include "zqtheader.h"
#include "c_stack.h"
#include "zintcuboid.h"
#include "zstackobject.h"
#include "tz_cuboid_i.h"
#include "zhistogram.h"
#include "zvoxel.h"
#include "zstackdocument.h"

class ZObject3d;
class ZGraph;
class ZStack;
class ZJsonArray;

/*!
 * \brief The class of RLE object stripe
 */
class ZObject3dStripe {
public:
  ZObject3dStripe() : m_y(0), m_z(0), m_isCanonized(true) {
  }
  inline int getY() const { return m_y; }
  inline int getZ() const { return m_z; }
  int getMinX() const;
  int getMaxX() const;
  inline size_t getSize() const { return m_segmentArray.size() / 2; }
  inline int getSegmentNumber() const { return getSize(); }
  size_t getVoxelNumber() const;

  inline void setY(int y) { m_y = y; }
  inline void setZ(int z) { m_z = z; }

  void addSegment(int x1, int x2, bool canonizing = true);

  const int* getSegment(size_t index) const;
  int getSegmentStart(size_t index) const;
  int getSegmentEnd(size_t index) const;

  void write(FILE *fp) const;
  void read(FILE *fp);

  void drawStack(Stack *stack, int v, const int *offset = NULL) const;
  void drawStack(Stack *stack, uint8_t red, uint8_t green, uint8_t blue,
                 const int *offset = NULL) const;

  /*!
   * \brief Count the overlap area between an object and a stack
   *
   * \param stack Input stack. It's foreground is defined as any pixel with
   *        intensity > 0.
   * \param offset Offset of the object.
   * \return The number of voxels overlapped.
   */
  size_t countForegroundOverlap(Stack *stack, const int *offset = NULL) const;

  inline bool isEmpty() const { return m_segmentArray.empty(); }
  inline bool isCanonized() const { return isEmpty() || m_isCanonized; }

  void sort();
  void canonize();
  bool unify(const ZObject3dStripe &stripe, bool canonizing = true);

  void print(int indent = 0) const;

  void downsample(int xintv);
  void downsampleMax(int xintv);
  void upSample(int xIntv);

  void clearSegment();

  void translate(int dx, int dy, int dz);

  /*!
   * \brief Add z value
   *
   * Basically it is the same as translate(0, 0, \a dz);
   */
  void addZ(int dz);

  bool isCanonizedActually();

  /*!
   * \brief Test if two stripe are the same with respect to internal representation
   */
  bool equalsLiterally(const ZObject3dStripe &stripe) const;

  void dilate();

  inline void setCanonized(bool canonized) { m_isCanonized = canonized; }

  void switchYZ();

  /*!
   * \brief Test if an X is within the range of the segments
   *
   * \return true iff \a x is on one of the segments.
   */
  bool containsX(int x) const;

  /*!
   * \brief Test the stripe contains a point
   *
   * \return true iff (\a x, \a y, \a z) is on the stripe.
   */
  bool contains(int x, int y, int z) const;

  /*!
   * \brief Fill an integer array with the stripe data
   * \param array Target array, which must be preallocated with sufficient size.
   */
  void fillIntArray(int *array) const;

private:
  std::vector<int> m_segmentArray;
  int m_y;
  int m_z;
  bool m_isCanonized;
};

/*!
 * \brief The class of RLE object
 *
 * A RLE object is the run-length encoded representatino of a 3D object, which
 * is defined as a set of voxels.This class encodes an object along the X,
 * direction, i.e. a contiguous list of voxels (x_1, y, z), ..., (x_n, y, z) are
 * encoded as ((x_1, x_n), y, z).
 */
class ZObject3dScan : public ZStackObject
{
public:
  ZObject3dScan();
  virtual ~ZObject3dScan();

  enum EComponent {
    COMPONENT_STRIPE_INDEX_MAP, COMPONENT_INDEX_SEGMENT_MAP,
    COMPONENT_ACCUMULATED_STRIPE_NUMBER,
    COMPONENT_SLICEWISE_VOXEL_NUMBER,
    COMPONENT_Z_PROJECTION,
    COMPONENT_ALL
  };

  bool isDeprecated(EComponent comp) const;
  void deprecate(EComponent comp);
  void deprecateDependent(EComponent comp);

  void clear();

  ZObject3d* toObject3d() const;

  bool isEmpty() const;
  size_t getStripeNumber() const;
  size_t getVoxelNumber() const;

  /*!
   * \brief Get voxel number at a certain slice
   * \param z The slice position.
   */
  size_t getVoxelNumber(int z) const;

  /*!
   * \brief Get the voxel number on each slice
   * \return The ith element is the #voxel at slice i.
   */
  const std::map<int, size_t>& getSlicewiseVoxelNumber() const;
  std::map<int, size_t>& getSlicewiseVoxelNumber();

  const ZObject3dStripe& getStripe(size_t index) const;
  ZObject3dStripe& getStripe(size_t index);

  /*
  const int* getFirstStripe() const;
  static int getY(const int *stripe);
  static int getZ(const int *stripe);
  //return number of scan lines
  static int getStripeSize(const int *stripe);
  static int* getSegment(const int *stripe);
  static void setStripeSize(int *stripe, int size);
  const int* getLastStripe() const;
  int* getLastStripe();
*/
  void addStripe(int z, int y, bool canonizing = true);
  void addSegment(int x1, int x2, bool canonizing = true);
  void addSegment(int z, int y, int x1, int x2, bool canonizing = true);
  void addStripe(const ZObject3dStripe &stripe, bool canonizing = true);

  //Turn a binary stack into scanlines
  void loadStack(const Stack *stack);
  void loadStack(const ZStack &stack);

  void print() const;

  void save(const char *filePath);
  void save(const char *filePath) const;
  void save(const std::string &filePath) const;
  bool load(const char *filePath);
  bool load(const std::string &filePath);

  bool hit(double x, double y, double z);
  bool hit(double x, double y);
  ZIntPoint getHitPoint() const;

  /*!
   * \brief Import a dvid object
   *
   * byte     Payload descriptor:
                   Bit 0 (LSB) - 8-bit grayscale
                   Bit 1 - 16-bit grayscale
                   Bit 2 - 16-bit normal
                   ...
        uint8    Number of dimensions
        uint8    Dimension of run (typically 0 = X)
        byte     Reserved (to be used later)
        uint32    # Voxels [TODO.  0 for now]
        uint32    # Spans
        Repeating unit of:
            int32   Coordinate of run start (dimension 0)
            int32   Coordinate of run start (dimension 1)
            int32   Coordinate of run start (dimension 2)
              ...
            int32   Length of run
            bytes   Optional payload dependent on first byte descriptor
   */
  bool importDvidObject(const std::string &filePath);

  void exportDvidObject(const std::string &filePath) const;

  /*!
   * \brief Import object from a byte array
   */
  bool importDvidObjectBuffer(const char *byteArray, size_t byteNumber);

  bool importDvidObjectBuffer(const std::vector<char> &byteArray);

  bool importDvidRoi(const ZJsonArray &obj);

  template<class T>
  int scanArray(const T *array, int x, int y, int z, int width, int x0 = 0);

  /*!
   * \brief Draw a stack
   * \param stack
   * \param v
   * \param offset Offset of translating the object
   */
  void drawStack(Stack *stack, int v, const int *offset = NULL) const;
  void drawStack(Stack *stack, uint8_t red, uint8_t green, uint8_t blue,
                 const int *offset = NULL) const;
  void labelStack(Stack *stack, int startLabel, const int *offset = NULL);

  void drawStack(ZStack *stack, int v) const;

  /*!
   * \brief Mask a stack with the foreground defined by the object.
   */
  void maskStack(ZStack *stack);

  /*!
   * \brief Count overlap between the object and a stack
   *
   * Count the overlap between the object and \a stack. Any voxel in the object
   * has postive value in \a stack is counted.
   *
   * \param offset The offset of the object to \a stack if it is not NULL.
   * \return Number of voxels in the overlapping region. It returns 0 if \a stack
   * is NULL.
   */
  size_t countForegroundOverlap(Stack *stack, const int *offset = NULL);

  //Sort the stripes in the ascending order
  // s1 < s2 if
  //   z(s1) < z(s2) or
  //   z(s1) == z(s2) and y(s1) < y(s2)
  void sort();
  void canonize();
  void unify(const ZObject3dScan &obj);
  void concat(const ZObject3dScan &obj);

  ZObject3dScan subtract(const ZObject3dScan &obj);

  void downsample(int xintv, int yintv, int zintv);
  void downsampleMax(int xintv, int yintv, int zintv);

  void upSample(int xIntv, int yIntv, int zIntv);

  Stack* toStack(int *offset = NULL, int v = 1) const;
  ZStack* toStackObject(int v = 1) const;
  ZStack* toVirtualStack() const;
  //ZStack* toDownsampledStack(int xIntv, int yIntv, int zIntv);

  ZIntCuboid getBoundBox() const;
  void getBoundBox(Cuboid_I *box) const;
  void getBoundBox(ZIntCuboid *box);

  template<class T>
  static std::map<int, ZObject3dScan*>* extractAllObject(
      const T *array, int width, int height, int depth, int startPlane,
      int yStep,
      std::map<int, ZObject3dScan*> *bodySet);

  template<class T>
  static std::map<int, ZObject3dScan*>* extractAllObject(
      const T *array, int width, int height, int depth, int x0, int y0, int z0,
      int yStep,
      std::map<int, ZObject3dScan*> *bodySet);

  template<class T>
  static std::map<int, ZObject3dScan*>* extractAllForegroundObject(
      const T *array, int width, int height, int depth, int x0, int y0, int z0,
      int yStep,
      std::map<int, ZObject3dScan*> *bodySet);

  //Foreground only
  static std::vector<ZObject3dScan*> extractAllObject(const ZStack &stack,
                                                      int yStep = 1);

  ZGraph* buildConnectionGraph();

  const std::vector<size_t> &getStripeNumberAccumulation() const;

  const std::map<std::pair<int, int>, size_t>& getStripeMap() const;

  std::vector<size_t> getConnectedObjectSize();
  std::vector<ZObject3dScan> getConnectedComponent();

  inline bool isCanonized() const { return isEmpty() || m_isCanonized; }
  inline void setCanonized(bool canonized) { m_isCanonized = canonized; }

  const std::map<size_t, std::pair<size_t, size_t> >&
  getIndexSegmentMap() const;
  bool getSegment(size_t index, int *z, int *y, int *x1, int *x2);
  size_t getSegmentNumber() const;

  void translate(int dx, int dy, int dz);
  void translate(const ZIntPoint &dp);

  /*!
   * \brief Add z value
   *
   * Basically it is the same as translate(0, 0, \a dz);
   */
  void addZ(int dz);

  bool isCanonizedActually();

  void duplicateAcrossZ(int depth);

  ZObject3dScan getSlice(int z) const;
  ZObject3dScan getSlice(int minZ, int maxZ) const;
  ZObject3dScan interpolateSlice(int z) const;

  virtual void display(
      ZPainter &painter, int slice, EDisplayStyle option) const;
  virtual const std::string& className() const;

  void dilate();
  void dilatePlane();

  ZPoint getCentroid() const;
  /*!
   * \brief Get the single voxel representing the object
   *
   * \return A voxel on the object. It returns (-1, -1, -1) if the object is
   *         empty.
   */
  ZVoxel getMarker() const;

  ZHistogram getRadialHistogram(int z) const;

  ZObject3dScan makeZProjection() const;
  ZObject3dScan makeZProjection(int minZ, int maxZ);

  ZObject3dScan makeYProjection() const;

  const ZObject3dScan* getZProjection() const;

  /*!
   * \brief Test if the object contains a voxel
   *
   * \return true iff (\a x, \a y, \a z) is a part of the object.
   */
  bool contains(int x, int y, int z);

  /*!
   * \brief Get minimal Z
   *
   * \return The minimal Z value of the object. If the object is empty,
   *         it returns 0.
   */
  int getMinZ() const;

  /*!
   * \brief Get maximal Z
   *
   * \return The maximal Z value of the object. If the object is empty,
   *         it returns 0.
   */
  int getMaxZ() const;

  /*!
   * \brief Get minimal Y
   *
   * \return The minimal Y value of the object. If the object is empty,
   *         it returns 0.
   */
  int getMinY() const;

  /*!
   * \brief Get maximal Y
   *
   * \return The maximal Y value of the object. If the object is empty,
   *         it returns 0.
   */
  int getMaxY() const;

  /*!
   * \brief Test if two objects are the same with respect to internal representation
   */
  bool equalsLiterally(const ZObject3dScan &obj) const;

  size_t getSurfaceArea() const;

  /*!
   * \brief Get the complement of the object
   *
   * The complement object is defined as the background of the bound box painted
   * with the original object.
   */
  ZObject3dScan getComplementObject();

  /*!
   * \brief Find all holes as a single object.
   * \return An object composed of all holes of the original object.
   */
  ZObject3dScan findHoleObject();

  /*!
   * \brief Find all holes as a single object
   * \return An array of objects, each representing a hole.
   */
  std::vector<ZObject3dScan> findHoleObjectArray();

  /*!
   * \brief Fill the holes of the object
   */
  void fillHole();

  /*!
   * \brief Make a stack from a list of objects
   *
   * \a offset is used to store the corner coordinates if it is not NULL.
   */
  template<class InputIterator>
  static Stack* makeStack(InputIterator startObject, InputIterator endObject,
                          int *offset = NULL);

  /*!
   * \brief Switch the Y and Z axis
   */
  void switchYZ();

  /*!
   * \brief Get the ~95% spread area of the object at a centain slice
   */
  double getSpread(int z) const;

  /*!
   * Compute the plane covariance
   */
  std::vector<double> getPlaneCov() const;

  typedef int TEvent; //What's event?
  void processEvent(TEvent event);
  void blockEvent(bool blocking);


  /*!
   * \brief Get the integer array representation of the object
   *
   * Format:
   *   data[0]: Number of stripes
   *   Stripes ...:
   *      stripe[0]: z
   *      stripe[1]: y
   *      stripe[2]: number of segments
   */
  std::vector<int> toIntArray() const;

  /*!
   * \brief Load from a data array
   *
   * The object will be cleared if data is NULL.
   *
   * \param data The data array arranged as the writing order
   * \param length Number of elements in \a data
   *
   * \return true if the data is loaded correctly
   */
  bool load(const int *data, size_t length);

  /*!
   * \brief Load object from an HDF5 file.
   *
   * The object becomes empty if the import failed.
   *
   * \param filePath HDF5 file path
   * \param key Data path of the object
   *
   * \return true iff the object is loaded successfully
   */
  bool importHdf5(const std::string &filePath, const std::string &key);

  /*!
   * \brief Saven object to an HDF5 file.
   *
   * If \a filePath exists, the function will try to write the object with the
   * appending mode; otherwise it will try to create a new HDF5 file.
   *
   * \param filePath HDF5 file path
   * \param key Data path of the object
   *
   * \return true iff the object is saved successfully
   */
  //bool exportHdf5(const std::string &filePath, const std::string &key) const;

  /*!
   * \brief Check if two objects have overlap
   *
   * 26-neighborhood.
   */
  bool hasOverlap(ZObject3dScan &obj);


  /*!
   * \brief Check if an object is ajacent to another
   */
  bool isAdjacentTo(ZObject3dScan &obj);

  uint64_t getLabel() const {
    return m_label;
  }

  void setLabel(uint64_t label) {
    m_label = label;
  }

  class Segment {
  public:
    Segment(int z = 0, int y = 0, int x0 = 0, int x1 = 0) :
      m_x0(x0), m_x1(x1), m_y(y), m_z(z) {}
    inline int getZ() const { return m_z; }
    inline int getY() const { return m_y; }
    inline int getStart() const { return m_x0; }
    inline int getEnd() const { return m_x1; }
    inline void set(int z = 0, int y = 0, int x0 = 0, int x1 = 0) {
      m_x0 = x0;
      m_x1 = x1;
      m_y = y;
      m_z = z;
    }

  private:
    int m_x0;
    int m_x1;
    int m_y;
    int m_z;
  };

  class ConstSegmentIterator {
  public:
    ConstSegmentIterator(const ZObject3dScan *obj = NULL);
    const Segment& next();
    bool hasNext() const;
    void advance();

  private:
    const ZObject3dScan *m_obj;
    size_t m_stripeIndex;
    int m_segmentIndex;
    Segment m_seg;
  };

private:
  void addForeground(ZStack *stack);
  void displaySolid(ZPainter &painter, int z, bool isProj, int stride = 1) const;
  void makeZProjection(ZObject3dScan *obj) const;

protected:
  std::vector<ZObject3dStripe> m_stripeArray;
  /*
  int m_stripeNumber;
  std::vector<int> m_sripeSize;
  */

  bool m_isCanonized;
  uint64_t m_label;
  //mutable int *m_lastStripe;

  bool m_blockingEvent;

  ZIntPoint m_hitPoint;
  mutable std::vector<size_t> m_accNumberArray;
  mutable std::map<int, size_t> m_slicewiseVoxelNumber;
  mutable std::map<std::pair<int, int>, size_t> m_stripeMap;
  mutable std::map<size_t, std::pair<size_t, size_t> > m_indexSegmentMap;
  mutable ZObject3dScan *m_zProjection;

  //SWIG has some problem recognizing const static type
#ifndef SWIG
  const static TEvent EVENT_OBJECT_MODEL_CHANGED; //Note that change of model implies change of view
  const static TEvent EVENT_OBJECT_UNCANONIZED;
  const static TEvent EVENT_OBJECT_CANONIZED;
  const static TEvent EVENT_OBJECT_VIEW_CHANGED;
  const static TEvent EVENT_NULL;
#endif
};


template<class T>
int ZObject3dScan::scanArray(
    const T *array, int x, int y, int z, int width, int x0)
{
  if (array == NULL) {
    return 0;
  }

  if (x < 0 || x >= width) {
    return 0;
  }

  int length = 0;
  T v = array[x];

  if (isEmpty()) {
    addStripe(z, y, false);
  } else {
    if (m_stripeArray.back().getY() != y || m_stripeArray.back().getZ() != z) {
      addStripe(z, y, false);
    }
  }

  while (array[x + length] == v) {
    ++length;
    if (x + length >= width) {
      break;
    }
  }

  x += x0;
  addSegment(x, x + length - 1, false);

  return length;
}

template<class T>
std::map<int, ZObject3dScan*>* ZObject3dScan::extractAllObject(
    const T *array, int width, int height, int depth, int startPlane,
    int yStep,
    std::map<int, ZObject3dScan*> *bodySet)
{
  if (bodySet == NULL) {
    bodySet = new std::map<int, ZObject3dScan*>;
  }

  ZObject3dScan *obj = NULL;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; y += yStep) {
      int x = 0;
      while (x < width) {
        int v = array[x];
        std::map<int, ZObject3dScan*>::iterator iter = bodySet->find(v);
        if (iter == bodySet->end()) {
          obj = new ZObject3dScan;
          obj->blockEvent(true);
          obj->setLabel(v);
          //(*bodySet)[v] = obj;
          bodySet->insert(std::map<int, ZObject3dScan*>::value_type(v, obj));
        } else {
          obj = iter->second;
        }
        int length = obj->scanArray(array, x, y, z + startPlane, width);

        x += length;
      }
      array += width * yStep;
    }
  }

  for (std::map<int, ZObject3dScan*>::iterator iter = bodySet->begin();
       iter != bodySet->end(); ++iter) {
    ZObject3dScan *obj = iter->second;
    obj->blockEvent(false);
  }

  return bodySet;
}

template<class T>
std::map<int, ZObject3dScan*>* ZObject3dScan::extractAllObject(
    const T *array, int width, int height, int depth, int x0, int y0, int z0,
    int yStep,
    std::map<int, ZObject3dScan*> *bodySet)
{
  if (bodySet == NULL) {
    bodySet = new std::map<int, ZObject3dScan*>;
  }

  ZObject3dScan *obj = NULL;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; y += yStep) {
      int x = 0;
      while (x < width) {
        int v = array[x];
        std::map<int, ZObject3dScan*>::iterator iter = bodySet->find(v);
        if (iter == bodySet->end()) {
          obj = new ZObject3dScan;
          obj->setLabel(v);
          //(*bodySet)[v] = obj;
          bodySet->insert(std::map<int, ZObject3dScan*>::value_type(v, obj));
        } else {
          obj = iter->second;
        }
        int length = obj->scanArray(array, x, y + y0, z + z0, width, x0);

        x += length;
      }
      array += width * yStep;
    }
  }

  return bodySet;
}

template<class T>
std::map<int, ZObject3dScan*>* ZObject3dScan::extractAllForegroundObject(
    const T *array, int width, int height, int depth, int x0, int y0, int z0,
    int yStep, std::map<int, ZObject3dScan*> *bodySet)
{
  if (bodySet == NULL) {
    bodySet = new std::map<int, ZObject3dScan*>;
  }

  ZObject3dScan *obj = NULL;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; y += yStep) {
      int x = 0;
      while (x < width) {
        int v = array[x];
        if (v > 0) {
          std::map<int, ZObject3dScan*>::iterator iter = bodySet->find(v);
          if (iter == bodySet->end()) {
            obj = new ZObject3dScan;
            obj->setLabel(v);
            //(*bodySet)[v] = obj;
            bodySet->insert(std::map<int, ZObject3dScan*>::value_type(v, obj));
          } else {
            obj = iter->second;
          }
          int length = obj->scanArray(array, x, y + y0, z + z0, width, x0);

          x += length;
        } else {
          ++x;
        }
      }
      array += width * yStep;
    }
  }

  return bodySet;
}


template<class InputIterator>
Stack* ZObject3dScan::makeStack(InputIterator startObject,
                                InputIterator endObject, int *offset)
{
  if (startObject != endObject) {
    Cuboid_I boundBox;
    startObject->getBoundBox(&boundBox);

    InputIterator iter = startObject;
    ++iter;
    //Get Bound box
    for (; iter != endObject; ++iter) {
      Cuboid_I subBoundBox;
      iter->getBoundBox(&subBoundBox);
      Cuboid_I_Union(&boundBox, &subBoundBox, &boundBox);
    }

    int width, height, depth;
    Cuboid_I_Size(&boundBox, &width, &height, &depth);
    //Create stack
    Stack *stack = C_Stack::make(GREY, width, height, depth);
    C_Stack::setZero(stack);

    int stackOffset[3] = {0, 0, 0};
    for (int i = 0; i < 3; ++i) {
      stackOffset[i] = -boundBox.cb[i];
    }

    int v = 1;
    for (iter = startObject; iter != endObject; ++iter) {
      iter->drawStack(stack, v++, stackOffset);
    }

    if (offset != NULL) {
      for (int i = 0; i < 3; ++i) {
        offset[i] = boundBox.cb[i];
      }
    }

    return stack;
  }

  return NULL;
}

#endif // ZOBJECT3DSCAN_H
