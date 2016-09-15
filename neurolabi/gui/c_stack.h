#ifndef C_STACK_H
#define C_STACK_H

#include <string>
#include <vector>

#include "tz_image_lib_defs.h"
#include "tz_mc_stack.h"
#include "tz_cuboid_i.h"
#include "tz_stack_watershed.h"
#include "zinthistogram.h"

//! Functions for C-compatible stack
/*!
 * Two types of stack structure are used:
 *   Stack: single-channel stack
 *   Mc_Stack: Multiple-channel stack
 */

namespace C_Stack {
//General
//! Coordinate to index
/*!
 * Get the array index of the coordinates (\a x, \a y, \a z) in the stack with
 * the size (\a width, \a height, \a depth).
 *
 * \return Array index if the coordinates are in the stack; -1 if the
 *         coordinates are out of range.
*/
ssize_t offset(int x, int y, int z, int width, int height, int depth);

/*!
 * \brief Neighborhood check.
 */
int neighborTest(int conn, int width, int height, int depth, size_t index,
                 int *isInBound);
int neighborTest(int conn, int width, int height, int depth,
                 int x, int y, int z, int *isInBound);

void neighborOffset(int conn, int width, int height, int neighbor[]);

//Functions for Stack

//Stack deallocator, mainly used for constructing a ZStack object
typedef void Stack_Deallocator(Stack*);

/*!
 * \brief Create a stack
 * \param kind Voxel type of the stack.
 * \param width Width of the stack.
 * \param height Height of the stack.
 * \param depth Number of slices of the stack.
 * \return A pointer to the stack. The user is reponsible for freeing it by
 *         calling the kill().
 * \sa kill()
 */
Stack *make(int kind, int width, int height, int depth);

Stack *make(float *data, int kind, int width, int height, int depth);
Stack *make(double *data, int kind, int width, int height, int depth);


//Kill from memory pool
/*!
 * \brief Kill a stack (free up all associated memory).
 * \param stack The stack object to kill. It must be created by make().
 * \sa make()
 */
void kill(Stack *stack);

//Free all associated memory
void systemKill(Stack *stack);

//Delete the stack by 'delete' in C++
inline void cppDelete(Stack *stack) { delete stack; }

int stackUsage();

/** @name Make copies
 */
///@{
//Clone a stack
/*!
 * \brief Clone a stack.
 * \param stack The object to be cloned.
 * \return The cloned object. The user is responsible for freeing it.
 */
Stack* clone(const Stack *stack);

//Stack* copy(const Stack *stack);
void copyValue(const Stack *src, Stack *dst);
void copyPlaneValue(Stack *stack, const void *array, int slice);
///@}

//Attributes of a stack
/*!
 * \brief Width of the stack
 *
 * \a stack shouldn't be NULL.
 */
inline int width(const Stack *stack) { return stack->width; }

/*!
 * \brief Height of the stack
 *
 * \a stack shouldn't be NULL.
 */
inline int height(const Stack *stack) { return stack->height; }

/*!
 * \brief Depth (number of planes) of the stack
 *
 * \a stack shouldn't be NULL.
 */
inline int depth(const Stack *stack) { return stack->depth; }

/*!
 * \brief Pixel data kind of the stack
 *
 *  \a stack shouldn't be NULL. The kind can be:
 *    GREY: unsigned 8-bit
 *    GREY16: unsigned 16-bit
 *    COLOR: 24-bit RGB
 *    FLOAT32: single-precision float
 *    FLOAT64: double-predision float
 */
inline int kind(const Stack *stack) { return stack->kind; }

inline void setWidth(Stack *stack, int width) { stack->width = width; }
inline void setHeight(Stack *stack, int height) { stack->height = height; }
inline void setDepth(Stack *stack, int depth) { stack->depth = depth; }
inline void setKind(Stack *stack, int kind) { stack->kind = kind; }
void setAttribute(Stack *stack, int kind, int width, int height, int depth);
inline size_t planeByteNumber(const Stack *stack) {
  return static_cast<size_t>(stack->kind) * stack->width * stack->height;
}
inline size_t area(const Stack *stack) {
  return (size_t) width(stack) * height(stack);
}

size_t voxelNumber(const Stack *stack);

inline size_t allByteNumber(const Stack *stack) {
  return planeByteNumber(stack) * depth(stack) * kind(stack);
}

//Voxel access
/*!
 * \brief Pointer to the raw data
 */
inline uint8_t* array8(const Stack *stack) { return (uint8_t*) stack->array; }

uint16_t* guardedArray16(const Stack *stack);
float* guardedArrayFloat32(const Stack *stack);

/*!
 * \brief Voxel value at a certain index
 *
 * It returns the value in the first channel for COLOR kind.
 */
double value(const Stack *stack, size_t index);

/*!
 * \brief Voxel value at a certain position
 *
 * \param stack Source stack
 * \param x X coordinate
 * \param y Y coordinate
 * \param z Z coordinate
 * \param c Color channel
 */
double value(const Stack *stack, int x, int y, int z, int c = 0);

void setPixel(Stack *stack, int x, int y, int z, int c, double v);
void setZero(Stack *stack);

void setOne(Stack *stack);

/*!
 * \brief View a single slice
 */
Stack sliceView(const Stack *stack, int slice);
Stack sliceView(const Mc_Stack *stack, int slice, int channel);

/*!
 * \brief View one or more slices
 *
 * The start plane is set to 0 if \a startPlane is negative and the end plane
 * is set to the last plane of \a stack if it is out of range.
 *
 * \param stack Source stack.
 * \param startPlane Start plane.
 * \param endPlane End plane.
 */
Stack sliceView(const Stack *stack, int startPlane, int endPlane);

Stack* channelExtraction(const Stack *stack, int channel);

/*!
 * \brief Set values of a stack by memory copying
 *
 * \param stack Target stack.
 * \param offset Start of the stack buffer for assignment
 * \param buffer Source buffer
 * \param length Size of the source buffer
 *
 * \return true if the values are set successfully
 */
bool setValue(Stack *stack, size_t offset, const void *buffer, size_t length);

bool setValue(Mc_Stack *stack, size_t offset, const void *buffer, size_t length);

//Stack manipulation
//Crop a stack
Stack* crop(const Stack *stack,int left,int top,int front,
            int width,int height,int depth, Stack *desstack);

Stack* crop(const Stack *stack, const Cuboid_I &box, Stack *desstack);

//Crop a stack using its bound box
Stack* boundCrop(const Stack *stack, int margin = 0);

/*!
 * \brief Crop a stack using its bound box
 *
 * It stores the offset to the original stack in \a offset if \a offset is not
 * NULL.
 */
Stack* boundCrop(const Stack *stack, int margin, int *offset);

Stack* resize(const Stack* stack,int width,int height,int depth);
Stack* translate(Stack *stack, int kind, int in_place);

/*!
 * \brief Downsample a stack with maximum assignment.
 *
 * \param stack source stack
 * \param xintv X interval
 * \param yintv Y interval
 * \param zintv Z interval
 * \param result object to hold the result. It must have have enough
 *    preallocated memory to hold the results if it is not NULL.
 *    If it is NULL, a new object is returned.
 * \return The result.
 */
Stack* downsampleMax(const Stack *stack, int xintv, int yintv, int zintv,
                     Stack *result = NULL);
Stack* downsampleMin(const Stack *stack, int xintv, int yintv, int zintv,
                     Stack *result = NULL);

void print(const Stack *stack);

/*!
 * \brief Print the voxel values of a stack
 */
void printValue(const Stack *stack);

void printValue(const Mc_Stack *stack);

//Stack statistics
double min(const Stack *stack);
double max(const Stack *stack);
double sum(const Stack *stack);
int* hist(const Stack *stack);
ZIntHistogram* hist(const Stack *stack, ZIntHistogram *out);
double mean(const Stack *stack);
double mode(const Stack *stack);

//Miscellanea
size_t closestForegroundPixel(const Stack *stack, double x, double y, double z);

//Functions for Mc_Stack
typedef void Mc_Stack_Deallocator(Mc_Stack*);
Mc_Stack *make(int kind, int width, int height, int depth, int channelNumber);

Mc_Stack* clone(const Mc_Stack *stack);

inline int width(const Mc_Stack *stack) { return stack->width; }
inline int height(const Mc_Stack *stack) { return stack->height; }
inline int depth(const Mc_Stack *stack) { return stack->depth; }
inline int kind(const Mc_Stack *stack) { return stack->kind; }
inline int channelNumber(const Mc_Stack *stack) { return stack->nchannel; }
inline size_t area(const Mc_Stack *stack) {
  return (size_t) width(stack) * height(stack);
}

inline uint8_t* array8(const Mc_Stack *stack) {
  return (uint8_t*) stack->array; }

inline size_t voxelNumber(const Mc_Stack *stack) {
  return (size_t) width(stack) * height(stack) * depth(stack);
}

inline size_t planeByteNumber(const Mc_Stack *stack) {
  return static_cast<size_t>(stack->kind) * stack->width * stack->height;
}

inline size_t volumeByteNumber(const Mc_Stack *stack) {
  return planeByteNumber(stack) * depth(stack);
}

inline size_t elementNumber(const Mc_Stack *stack) {
  return ((size_t) width(stack)) * height(stack) * depth(stack) *
      channelNumber(stack);
}

inline size_t allByteNumber(const Mc_Stack *stack) {
  return volumeByteNumber(stack) * channelNumber(stack);
}

void setAttribute(Mc_Stack *stack, int kind, int width, int height, int depth,
                  int channel);

void setChannelNumber(Mc_Stack *stack, int nchannel);
inline void cppDelete(Mc_Stack *stack) { delete stack; }
void systemKill(Mc_Stack *stack);
void freePointer(Mc_Stack *stack);

void kill(Mc_Stack *stack);

void copyPlaneValue(Mc_Stack *stack, const void *array, int channel, int slice);
void copyChannelValue(Mc_Stack *mc_stack, int chan, const Stack *stack);

bool hasSameValue(Mc_Stack *stack, size_t index1, size_t index2);
bool hasSameValue(Mc_Stack *stack, size_t index1, size_t index2,
                  size_t channelOffset);

bool hasValue(Mc_Stack *stack, size_t index, uint8_t *buffer);
bool hasValue(Mc_Stack *stack, size_t index,
              uint8_t *buffer, size_t channelOffset);

void view(const Stack *src, Mc_Stack *dst);
void view(const Mc_Stack *src, Stack *dst, int channel);
void view(const Mc_Stack *src, Mc_Stack *dst, int channel);
void view(const Stack *src, Image_Array *dst);

Mc_Stack* translate(const Mc_Stack *stack, int targetKind);


Stack* extractChannel(const Stack *stack, int c);

void setStackValue(Stack *stack, const std::vector<size_t> &indexArray,
                   double value);
/*!
 * \brief Set all voxel values of a stack to 0
 */
void setZero(Mc_Stack *stack);

/*!
 * \brief Set all voxel values of a stack to 1
 */
void setOne(Mc_Stack *stack);

std::vector<size_t> getNeighborIndices(
    const Stack *stack, const std::vector<size_t> &indexArray,
    int conn, double value);
size_t countForegoundNeighbor(const Stack *stack, size_t index, int conn);


double min(const Mc_Stack *stack);
double max(const Mc_Stack *stack);

ssize_t indexFromCoord(int x, int y, int z, int width, int height, int depth);
void indexToCoord(size_t index, int width, int height, int *x, int *y, int *z);

void write(const std::string &filePath, const Stack *stack);
void write(const std::string &filePath, const Mc_Stack *stack,
           const char *meta = NULL);
Mc_Stack* read(const std::string &filePath, int channel = -1);
Stack* readSc(const std::string &filePath);
Mc_Stack* readMrawFromBuffer(const char *buffer, int channel = -1);
char* toMrawBuffer(const Mc_Stack *stack, size_t *length);
void readStackOffset(const std::string &filePath, int *x, int *y, int *z);

Mc_Stack* resize(const Mc_Stack *stack, int width, int height, int depth);

//Routines for substack
/*!
 * \brief Set a subregion of a stack to 0
 *
 * Set the region of (\a x0, \a y0, \a z0) | \a sw x \a sh x \a sd to 0 in
 * \a stack.
 */
void setZero(Mc_Stack *stack, int x0, int y0, int z0, int sw, int sh, int sd);

/*!
 * \brief Set the value of a block of a stack
 *
 * \a stack and \a block must have the same kind. The value of \a block will be
 * copied to \a stack at the starting position of (\a x0, \a y0, \a z0). Any
 * value equal to \a valueIgnored in \a block is ignored, i.e. the values of
 * the corresponding positions in \a stack are untouched.
 */
//void setBlockValue(Stack *stack, const Stack *block, int x0, int y0, int z0);
void setBlockValue(Stack *stack, const Stack *block, int x0, int y0, int z0,
                   int srcValueIgnored = -1, int dstValueIgnored = -1,
                   double alpha = 1.0);

/*!
 * \brief Check if a stack is binary
 *
 * A stack is binary if it has GREY kind and the maximum voxel value is 1.
 */
bool isBinary(const Stack *stack);


Image* makeMinProjZ(const Stack* stack, int minZ, int maxZ);
Image* makeMaxProjZ(const Stack* stack, int minZ, int maxZ);

//Paint routines
void drawPatch(Stack *canvas, const Stack *patch,
               int dx, int dy, int dz, int transparentValue);

int digitWidth(int n);
int integerWidth(int n, int interval);
int drawDigit(Stack *canvas, int n, int dx, int dy, int dz);
void drawInteger(Stack *canvas, int n, int dx, int dy, int dz, int interval = 10);

//Experimenting APIs
Stack* computeGradient(const Stack *stack);
void shrinkBorder(const Stack *stack, int r, int nnbr = 6);
Stack* watershed(const Stack *stack, Stack_Watershed_Workspace *ws, Stack *out);

}

#endif // C_STACK_H
