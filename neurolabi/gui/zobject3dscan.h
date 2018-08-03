#ifndef ZOBJECT3DSCAN_H
#define ZOBJECT3DSCAN_H

#include <vector>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <utility>
#include <fstream>

#ifdef _QT_GUI_USED_
#include <QByteArray>
#endif

#include "zqtheader.h"
#include "c_stack.h"
#include "zstackobject.h"
#include "tz_cuboid_i.h"
#include "zhistogram.h"
#include "zvoxel.h"
#include "zstackdocument.h"
#include "zobject3dstripe.h"
#include "geometry/zgeometry.h"

class ZObject3d;
class ZGraph;
class ZStack;
class ZJsonArray;

/*!
 * \brief The class of RLE object
 *
 * A RLE object is the run-length encoded representatino of a 3D object, which
 * is defined as a set of voxels. This class encodes an object along the X,
 * direction, i.e. a contiguous list of voxels (x_1, y, z), ..., (x_n, y, z) are
 * encoded as ((x_1, x_n), y, z).
 */
class ZObject3dScan : public ZStackObject
{
public:
  ZObject3dScan();
  virtual ~ZObject3dScan();

  ZObject3dScan(const ZObject3dScan &obj);
  ZObject3dScan(const ZObject3dScan &&obj);

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_OBJECT3D_SCAN;
  }


  enum EComponent {
    COMPONENT_STRIPE_INDEX_MAP, COMPONENT_INDEX_SEGMENT_MAP,
    COMPONENT_ACCUMULATED_STRIPE_NUMBER,
    COMPONENT_SLICEWISE_VOXEL_NUMBER,
    COMPONENT_Z_PROJECTION,
    COMPONENT_ALL
  };

  enum EAction {
    ACTION_NONE, ACTION_CANONIZE, ACTION_SORT_YZ
  };

  static const int MAX_SPAN_HINT;

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

//  NeuTube::EAxis getSliceAxis() const { return m_sliceAxis; }
//  void setSliceAxis(NeuTube::EAxis axis) { m_sliceAxis = axis; }

  /*!
   * \brief Get the voxel number on each slice
   * \return The ith element is the #voxel at slice i.
   */
  const std::unordered_map<int, size_t> &getSlicewiseVoxelNumber() const;
  std::unordered_map<int, size_t>& getSlicewiseVoxelNumber();

  const ZObject3dStripe& getStripe(size_t index) const;
  ZObject3dStripe& getStripe(size_t index);

  void addStripe(int z, int y);
  void addStripeFast(int z, int y);
  void addStripeFast(const ZObject3dStripe &stripe);
  void addSegment(int x1, int x2, bool canonizing = true);
  void addSegmentFast(int x1, int x2);
  void addSegment(int z, int y, int x1, int x2, bool canonizing = true);
  void addStripe(const ZObject3dStripe &stripe, bool canonizing = true);

  //Turn a binary stack into scanlines
  void loadStack(const Stack *stack);
  void loadStack(const ZStack &stack);

  void print() const;
  void printInfo() const;

  bool save(const char *filePath);
  bool save(const char *filePath) const;
  bool save(const std::string &filePath) const;
  bool load(const char *filePath);
  bool load(const std::string &filePath);

  bool hit(double x, double y, double z);
  bool hit(double x, double y, neutube::EAxis axis);
  //ZIntPoint getHitPoint() const;

  ZObject3dScan& operator=(const ZObject3dScan& obj);// { return *this; }
  ZObject3dScan& operator=(const ZObject3dScan&& obj);

  void copyDataFrom(const ZObject3dScan &obj);
  void copyAttributeFrom(const ZObject3dScan &obj);

  void write(std::ostream &stream) const;
  void read(std::istream &stream);

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

#ifdef _QT_GUI_USED_
  QByteArray toDvidPayload() const;
#endif

  /*!
   * \brief Import object from a byte array
   */
  bool importDvidObjectBuffer(const char *byteArray, size_t byteNumber);

  static size_t CountVoxelNumber(const char *byteArray, size_t byteNumber);

  bool importDvidObjectBuffer(const std::vector<char> &byteArray);

  bool importDvidObjectBufferDs(const char *byteArray, size_t byteNumber);

  bool importDvidObjectBuffer(const char *byteArray, size_t byteNumber,
                              int xIntv, int yIntv, int zIntv);

  bool importDvidRoi(const ZJsonArray &obj, bool appending = false);
  bool importDvidRoi(const std::string &filePath);

  template<class T>
  int scanArray(const T *array, int x, int y, int z, int width,
                int x0 = 0);

  template<class T>
  int scanArray(const T *array, int x, int y, int z, int width, int dim,
                int start, neutube::EAxis axis);

  template<class T>
  int scanArrayShift(
      const T *array, int start, int y, int z, int stride, int dim);


  void addStack(Stack *stack, int v, const int *offset = NULL) const;
  void addStack(ZStack *stack, int v) const;

  /*!
   * \brief Draw a stack
   * \param stack
   * \param v
   * \param offset Offset of translating the object
   */
  void drawStack(Stack *stack, int v, const int *offset = NULL) const;
  void drawStack(Stack *stack, uint8_t red, uint8_t green, uint8_t blue,
                 const int *offset = NULL) const;
  void drawStack(Stack *stack, uint8_t red, uint8_t green, uint8_t blue,
                 double alpha,
                 const int *offset) const;
  void labelStack(Stack *stack, int startLabel, const int *offset = NULL);

  void drawStack(ZStack *stack, int v) const;
  void drawStack(ZStack *stack, uint8_t red, uint8_t green, uint8_t blue) const;

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

  /*!
   * \brief Sort the stripes in the ascending order
   *
   * s1 < s2 if
   *   z(s1) < z(s2) or
   *   z(s1) == z(s2) and y(s1) < y(s2)
   */
  void sort();
  void canonize();
  void sortedCanonize();
  void fullySortedCanonize();

  /*!
   * \brief Unify two objects
   *
   * Unify \a obj to the current object and keep the result canonized.
   */
  void unify(const ZObject3dScan &obj);

  /*!
   * \brief Concatenate two objects
   *
   * The current object will be changed to the combination of its old content
   * and \a obj. The result will not be canonized.
   */
  void concat(const ZObject3dScan &obj);

  ZObject3dScan subtract(const ZObject3dScan &obj);
  void subtractSliently(const ZObject3dScan &obj);

  friend ZObject3dScan operator - (
      const ZObject3dScan &obj1, const ZObject3dScan &obj2);

  ZObject3dScan intersect(const ZObject3dScan &obj) const;

  /*!
   * \brief Extract voxels within a cuboid
   *
   * \a remain and \a result can be NULL or any pointer other than 'this' object.
   */
  ZObject3dScan *subobject(const ZIntCuboid &box, ZObject3dScan *remain,
                          ZObject3dScan *result) const;

  /*!
   * \brief Chop the object into two parts
   *
   * The output is the same as \a result if \a result is not NULL; otherwise
   * it returns a new pointer and the caller is responsible for freeing the
   * memory.
   *
   * \return The part above z (<z)
   */
  ZObject3dScan *chopZ(int z, ZObject3dScan *remain,
                          ZObject3dScan *result) const;

  /*!
   * \brief Chop the object into two parts
   *
   * \return The part on the left (<x)
   */
  ZObject3dScan *chopX(int x, ZObject3dScan *remain, ZObject3dScan *result) const;

  ZObject3dScan *chopY(int y, ZObject3dScan *remain, ZObject3dScan *result) const;

  ZObject3dScan* chop(
      int v, neutube::EAxis axis, ZObject3dScan *remain,
      ZObject3dScan *result) const;

  /*!
   * \brief Remove voxels within a box.
   */
  void remove(const ZIntCuboid &box);

  void downsample(int xintv, int yintv, int zintv);
  void downsampleMax(int xintv, int yintv, int zintv);
  void downsampleMax(const ZIntPoint &dsIntv);
  void downsampleMin(int xintv, int yintv, int zintv);
  ZObject3dScan downsampleBorderMask(int xintv, int yintv, int zintv);

  void upSample(int xIntv, int yIntv, int zIntv);
  void upSample(const ZIntPoint &dsIntv);

  Stack* toStack(int *offset = NULL, int v = 1) const;
  Stack* toStackWithMargin(int *offset, int v, int margin) const;

  /*!
   * \brief Make a stack from the object.
   *
   * The downsample intervals of the object will be passed to the stack too.
   */
  ZStack* toStackObject(int v = 1, ZStack *result = NULL) const;

  ZStack* toStackObjectWithMargin(int v, int margin) const;

  ZStack* toVirtualStack() const;
  //ZStack* toDownsampledStack(int xIntv, int yIntv, int zIntv);

  ZIntCuboid getBoundBox() const;
  void getBoundBox(Cuboid_I *box) const;
  void boundBox(ZIntCuboid *box) const;

  template<class T>
  static std::map<uint64_t, ZObject3dScan*>* extractAllObject(
      const T *array, int width, int height, int depth, int startPlane,
      int yStep,
      std::map<uint64_t, ZObject3dScan*> *bodySet);

  template<class T>
  static std::map<uint64_t, ZObject3dScan*>* extractAllObject(
      const T *array, int width, int height, int depth, int x0, int y0, int z0,
      int yStep,
      std::map<uint64_t, ZObject3dScan*> *bodySet);

  template<class T>
  static std::map<uint64_t, ZObject3dScan*>* extractAllObject(
      const T *array, int width, int height, int depth, neutube::EAxis axis);

  template<class T>
  static std::map<uint64_t, ZObject3dScan*>* extractAllForegroundObject(
      const T *array, int width, int height, int depth, neutube::EAxis axis);

  template<class T>
  static std::map<uint64_t, ZObject3dScan*>* extractAllForegroundObject(
      const T *array, int width, int height, int depth, int x0, int y0, int z0,
      int yStep,
      std::map<uint64_t, ZObject3dScan*> *bodySet);


  //Foreground only
  static std::vector<ZObject3dScan*> extractAllObject(const ZStack &stack,
                                                      int yStep = 1);

  ZGraph* buildConnectionGraph();

  const std::vector<size_t> &getStripeNumberAccumulation() const;

  const std::map<std::pair<int, int>, size_t> &getStripeMap() const;

  std::vector<size_t> getConnectedObjectSize();
  std::vector<ZObject3dScan> getConnectedComponent(EAction ppAction);

  /*!
   * \brief Check if an object is canonized.
   *
   * Note that this property may also determine the actual content of the object.
   * In the case of empty stripes in an object, the canonized form of the object
   * will remove all empty stripes if the object is not canonized. Otherwise,
   * the empty stripe may leave there. This complicates the data structure, but
   * no better solution has been worked out because an empty stripe is often used
   * to serve as a place holder.
   */
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
//  void addZ(int dz);

  bool isCanonizedActually();

  void duplicateSlice(int depth);

  ZObject3dScan getSlice(int z) const;
  ZObject3dScan getMedianSlice() const;

  ZObject3dScan getSlice(int minZ, int maxZ) const;
  ZObject3dScan interpolateSlice(int z) const;
  ZObject3dScan getFirstSlice() const;

  void exportImageSlice(int minZ, int maxZ, const std::string outputFolder) const;
  void exportImageSlice(const std::string outputFolder) const;

  virtual void display(
      ZPainter &painter, int slice, EDisplayStyle option,
      neutube::EAxis sliceAxis) const;
  virtual const std::string& className() const;

  void dilate();
  void dilatePlane();

  void setDsIntv(int x, int y, int z);
  void setDsIntv(const ZIntPoint &intv);
  void setDsIntv(int intv);

  ZPoint getCentroid() const;
  /*!
   * \brief Get the single voxel representing the object
   *
   * \return A voxel on the object. It returns (-1, -1, -1) if the object is
   *         empty.
   */
  ZVoxel getMarker() const;

  ZHistogram getRadialHistogram(int z) const;

  ZObject3dScan makeZProjection(int destZ = 0) const;
  ZObject3dScan makeZProjection(int minZ, int maxZ, int destZ = 0);

  ZObject3dScan makeYProjection() const;

  const ZObject3dScan* getZProjection() const;

  /*!
   * \brief Test if the object contains a voxel
   *
   * \return true iff (\a x, \a y, \a z) is a part of the object.
   */
  bool contains(int x, int y, int z);
  bool contains(const ZIntPoint &pt);

  ZIntPoint getDsIntv() const {
    return m_dsIntv;
  }

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

  ZObject3dScan getSurfaceObject() const;

  ZObject3dScan getPlaneSurface(int z) const;
  ZObject3dScan getPlaneSurface() const;

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
  bool exportHdf5(const std::string &filePath, const std::string &key) const;

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


  /*
  uint64_t getLabel() const {
    return m_label;
  }

  void setLabel(uint64_t label) {
    m_label = label;
  }
  */


  class Segment {
  public:
    Segment(int z = 0, int y = 0, int x0 = 0, int x1 = -1) :
      m_x0(x0), m_x1(x1), m_y(y), m_z(z) {}
    inline int getZ() const { return m_z; }
    inline int getY() const { return m_y; }
    inline int getStart() const { return m_x0; }
    inline int getEnd() const { return m_x1; }
    inline void set(int z, int y, int x0, int x1) {
      m_x0 = x0;
      m_x1 = x1;
      m_y = y;
      m_z = z;
    }
    inline bool isEmpty() const { return m_x0 > m_x1; }
    void clear() {
      set(0, 0, 0, -1);
    }

  private:
    int m_x0;
    int m_x1;
    int m_y;
    int m_z;
  };

  class ConstSegmentIterator {
  public:
    //The iterator always starts from the position prior to the first element.
    ConstSegmentIterator(const ZObject3dScan *obj = NULL);
    const Segment& next(); //Go to next and return the elment
    const Segment& current() const;
    bool hasNext() const;
    void advance();

  private:
    void skipOverEmptyStripe();

  private:
    const ZObject3dScan *m_obj;
    size_t m_nextStripeIndex;
    int m_nextSegmentIndex;
    Segment m_seg;
  };

  class ConstVoxelIterator {
  public:
    ConstVoxelIterator(const ZObject3dScan *obj = NULL);
    const ZIntPoint next();
    bool hasNext() const;
    void advance();

  private:
    const ZObject3dScan *m_obj;
    ConstSegmentIterator m_segIter;
    int m_nextX;
  };

  std::vector<ZObject3dStripe>& getStripeArray() {
    return m_stripeArray;
  }

private:
  void init();
  void addForeground(ZStack *stack);
  void addForegroundSlice8(ZStack *stack);
  int subtractForegroundSlice8(ZStack *stack);
  void displaySolid(ZPainter &painter, int z, bool isProj, int stride = 1) const;
  void makeZProjection(ZObject3dScan *obj) const;
  void makeZProjection(ZObject3dScan *obj, int z) const;

  void pushDsIntv(int xintv, int yintv, int zintv);
  void popDsIntv(int xintv, int yintv, int zintv);


protected:
  std::vector<ZObject3dStripe> m_stripeArray;
  bool m_isCanonized;
//  uint64_t m_label;
  bool m_blockingEvent;
  ZIntPoint m_dsIntv; //Downsampling hint, mainly for display
//  NeuTube::EAxis m_sliceAxis;

  //ZIntPoint m_hitPoint;
  mutable std::vector<size_t> m_accNumberArray;
  mutable std::unordered_map<int, size_t> m_slicewiseVoxelNumber;
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

#include "zobject3dscan.hpp"

#endif // ZOBJECT3DSCAN_H
