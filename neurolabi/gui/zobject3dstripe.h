#ifndef ZOBJECT3DSTRIPE_H
#define ZOBJECT3DSTRIPE_H

#include <vector>
#include "c_stack.h"
#include "neutube_def.h"

/*!
 * \brief The class of RLE object stripe
 */
class ZObject3dStripe {
public:
  ZObject3dStripe() :
    m_y(0), m_z(0), m_isCanonized(true) {}
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
  void drawStack(Stack *stack, int v, NeuTube::EAxis axis,
                 const int *offset = NULL) const;

  void drawStack(Stack *stack, uint8_t red, uint8_t green, uint8_t blue,
                 const int *offset = NULL) const;

  void drawStack(Stack *stack, uint8_t red, uint8_t green, uint8_t blue,
                 double alpha, const int *offset) const;

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

  std::vector<int>& getSegmentArray() { return m_segmentArray; }

  friend ZObject3dStripe operator - (
      const ZObject3dStripe &s1, const ZObject3dStripe &s2);

private:
  std::vector<int> m_segmentArray;
  int m_y;
  int m_z;
  bool m_isCanonized;
};
#endif // ZOBJECT3DSTRIPE_H
