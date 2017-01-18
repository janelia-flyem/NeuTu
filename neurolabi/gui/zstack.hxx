/**@file zstack.h
 * @brief Stack class
 * @author Ting Zhao
 */
#ifndef _ZSTACK_HXX_
#define _ZSTACK_HXX_

#include <vector>
#include <string>

#ifdef _NEUTUBE_
#include "zglmutils.h"
#include "znumericparameter.h"
#endif

#include "tz_image_lib_defs.h"
#include "c_stack.h"
#include "tz_stack_document.h"
#include "zsinglechannelstack.h"
#include "zstackfile.h"
#include "tz_image_io.h"
#include "zresolution.h"
#include "zpoint.h"
#include "zintcuboid.h"
#include "neutube_def.h"


//! Stack class
/*!
 *It supports multi-channel stacks. But most operations are on the first channel
 *by default. Internally, the class hosts voxel data in a 1-D array, which is
 *aligned by the order of (width, height, depth (number of slices), channel).
 *Therefore, voxels from the same row are stored contigously in an array. The
 *value or intensity of a voxel can have one of the following types:
 *  GREY: 8-bit unsigned integer
 *  GREY16: 16-bit unsigned integer
 *  FLOAT32: 32-bit float
 *  FLOAT64: 64-bit float
 *  COLOR: 24-bit color
 *Note that the COLOR type does not specify multi-channel data. It means the
 *single voxel has 3 contiguous bytes, which can be interpreted as a RGB color.
*/

class ZStack {
public:
  //! Default constructor
  /*! Default constructor: create an empty stack. */
  ZStack();

  //! Constructor
  /*!
   Construct a stack with essential attributes
   \param kind Type of the stack. It can be one of the following values:
               GREY, GREY16, FLOAT32, FLOAT64, COLOR
   \param width Width of the stack.
   \param height Height of the stack.
   \param depth Number of slices of the stack.
   \param nchannel Channel number of the stack.
   \param isVirtual Create a virtual stack if it is true. A virtual stack does
                    not contain any pixel data.
  */
  ZStack(int kind, int width, int height, int depth, int nchannel,
         bool isVirtual = false);

  //! Constructor
  /*!
   Construct a stack with stack data.
   \param stack Stack data
   \param dealloc The function applied on \a stack while the object is
                  deconstructed. \a delloc can be NULL to keep the stack data
                  in the memory even the object is deconstructed.
   */
  ZStack(Mc_Stack *stack/*,
         C_Stack::Mc_Stack_Deallocator *dealloc = C_Stack::kill*/);

  ZStack(int kind, const ZIntCuboid &box, int nchannel, bool isVirtual = false);

#if 0
  /*! Obsolete. Do not use this constructor!
   */
  ZStack(Mc_Stack *stack,
         C_Stack::Mc_Stack_Deallocator *dealloc = C_Stack::kill);
#endif

  //! Destructor
  virtual ~ZStack();

public: /* attributes */
  //! Types of specifying subset of a stack
  enum EStackUnit {
    SINGLE_VOXEL, /*!< A single voxel of a stack*/
    SINGLE_PLANE, /*!< A single plane of a stack */
    SINGLE_ROW, /*!< A single row of a stack */
    SINGLE_CHANNEL, /*!< A single channel of a stack */
    WHOLE_DATA /*!< All data of a stack */
  };

  //! Get the C-compatible data
  /*!
   The returned pointer is owned by ZStack.
   \sa setData()
   */
  inline Mc_Stack* mc_stack() { return m_stack; }
  inline const Mc_Stack* mc_stack() const { return m_stack; }
  inline Mc_Stack* data() { return mc_stack(); }
  inline const Mc_Stack* data() const { return mc_stack(); }

  /*!
   * \brief Set stack data
   *
   * Be careful when you want to set \a dealloc to a non-default value.
   */
  void setData(Mc_Stack *stack,
               C_Stack::Mc_Stack_Deallocator *dealloc = C_Stack::kill);

  /*!
   * \brief Set data from stack
   *
   * \a stack will be destroyed after function call. It does nothing if \a stack
   * is NULL.
   */
  void consume(Stack *stack);
  void consume(ZStack *stack);

  //! Get the C-compatible data
  /*!
   *The returned pointer is owned by ZStack.
   */
  const Stack* c_stack(int c = 0) const;
  Stack* c_stack(int c = 0);
  ZSingleChannelStack* singleChannelStack(int c = 0);
  const ZSingleChannelStack* singleChannelStack(int c = 0) const;

  /*!
   * \brief Get a single channel
   *
   * The data pointer is shared but the user is responsible for deleting the
   * returned pointer.
   */
  ZStack* getSingleChannel(int c) const;

  //! Width of the stack.
  inline int width() const {
    return mc_stack() == NULL ? 0 : C_Stack::width(mc_stack()); }
  //! Height of the stack.
  inline int height() const {
    return mc_stack() == NULL ? 0 : C_Stack::height(mc_stack()); }
  //! Number of slices of the stack.
  inline int depth() const {
    return mc_stack() == NULL ? 0 : C_Stack::depth(mc_stack()); }
  //! Voxel type of the stack.
  inline int kind() const {
    return mc_stack() == NULL ? 0 : C_Stack::kind(mc_stack()); }
  //! Channel number of the stack.
  inline int channelNumber() const {
    return mc_stack() == NULL ? 0 : C_Stack::channelNumber(m_stack);
  }

  //! Component of the stack
  enum EComponent {
    MC_STACK, SINGLE_CHANNEL_VIEW
  };

  //! Get the total byte number of the stack or its subset
  size_t getByteNumber(EStackUnit unit = WHOLE_DATA) const;

  //! Get the total voxel number of the stack or its subset
  size_t getVoxelNumber(EStackUnit unit = WHOLE_DATA) const;

  //! Voxel value at a given position and channel.
  double value(int x, int y, int z, int c = 0) const;

  //! Get voxel value given an idex
  double value(size_t index, int c = 0) const;

  //! Set voxel value at a given position and channel.
  void setValue(int x, int y, int z, int c, double v);

  /*!
   * \brief Get the intensity value as an integer
   *
   * A float value will be rounded to return. It returns 0 if the coordinates
   * are out of range. The position is adjusted by the stack offset.
   */
  int getIntValue(int x, int y, int z, int c = 0) const;

  int getIntValue8WithXCheckOnly(int x, int y, int z, int c = 0) const;

  /*!
   * \brief Get the intensity value as an integer
   *
   * (\a x, \a y, \a z) are coordinates relative to the stack origin.
   */
  int getIntValueLocal(int x, int y, int z, int c = 0) const;

  int getIntValue(size_t index, int c = 0) const;

  /*!
   * \brief Set the intensity value of a voxel.
   *
   * It does nothing if the coordinates are out of range.
   * The position is adjusted by the stack offset. for any value bigger than
   * the voxel maximum / minimum, it is set to maximum / minimum.
   */
  void setIntValue(int x, int y, int z, int c, int v);

  /*!
   * \brief Add value to the intensity of a voxel
   *
   * The resulted value will always be clipped to the largest possible pixel
   * valuel.
   */
  void addIntValue(int x, int y, int z, int c, int v);



  /** @name raw data access
   *  array8(), array16(), array32(), array64() or arrayc() can be used to otain
   *  the raw data array of the stack. The choice of the function depends on the
   *  voxel type. Those functions do not perform kind check.
   */
  ///@{
  /** Array for 8-bit unsigned integer. */
  inline uint8_t* array8(int c = 0) {
    return const_cast<uint8_t*>(
          const_cast<const ZStack*>(this)->array8(c)); }

  inline const uint8_t* array8(int c = 0) const {
    return (uint8_t*) rawChannelData(c);
  }

  /** Array for 16-bit unsigned integer. */
  inline uint16_t* array16(int c = 0) {
    return const_cast<uint16_t*>(
          const_cast<const ZStack*>(this)->array16(c)); }

  inline const uint16_t* array16(int c = 0) const{
    return (uint16_t*) rawChannelData(c);
  }

  /** Array for single precision float. */
  inline float* array32(int c = 0) {
    return const_cast<float*>(
          const_cast<const ZStack*>(this)->array32(c)); }

  inline const float* array32(int c = 0) const {
    return (float*) rawChannelData(c);
  }

  /** Array for double precision float. */
  inline double* array64(int c = 0) {
    return const_cast<double*>(
          const_cast<const ZStack*>(this)->array64(c)); }

  inline const double* array64(int c = 0) const {
    return (double*) rawChannelData(c);
  }

  /** Array for RGB color. */
  inline color_t* arrayc(int c = 0) {
    return const_cast<color_t*>(
          const_cast<const ZStack*>(this)->arrayc(c)); }

  inline const color_t* arrayc(int c = 0) const {
    return (color_t*) rawChannelData(c);
  }
  ///@}

  inline uint8_t value8(size_t index) const {
    return ((uint8_t*) rawChannelData())[index];
  }

  inline uint8_t value8(size_t index, int c) const {
    return ((uint8_t*) rawChannelData(c))[index];
  }

  /*!
   * \brief Test if a stack is empty
   *
   * \return true iff the size is 0.
   */
  bool isEmpty() const;

  /*!
   * \brief Test if a stack is virtual.
   * \return A stack is virtual iff the stack is not empty and its data array
   * is null
   */
  bool isVirtual() const;

  /*!
   * \brief Test if a stack has data.
   *
   * \return A stack has data if it is not empty and virtual.
   */
  bool hasData() const;


  // make mc_stack
  static Mc_Stack* makeMcStack(
      const Stack *stack1, const Stack *stack2, const Stack *stack3);

  //Source of the stack. Usually it is the file where the image is originally
  //read
  //from.
  /*!
   * \brief Get the source path of the stack.
   *
   * The source path of the stack is defined as the path from which the stack is
   * loaded.
   */
  std::string sourcePath() const;

  //Preferred z scale is the preferred scale ratio between z-axis and xy-plane
  //for anisotropic operations
//  inline double preferredZScale() const { return m_preferredZScale; }

  //Minimal value of the stack.
  double min();
  double min(int c) const;
  //Maximum value of the stack.
  double max();
  double max(int c) const;

  /*!
   * \brief Determine if a point is within a stack
   *
   * The input coordinates are supposed to be global.
   */
  bool contains(int x, int y, int z) const;
  bool contains(const ZPoint &pt) const;
  bool contains(const ZIntPoint &pt) const;
  bool contains(double x, double y) const;

  /*!
   * \brief containsRaw
   *
   * It return true if (\a x, \a y, \a z) is in the raw stack box, which is
   * defined as (0, width - 1) x (0, height - 1) x (0, depth - 1), with \a z
   * treated a little specially. If \a z is negative, only \a x and \a y are
   * tested, as if testing (\a x, \a y, 0).
   */
  bool containsRaw(double x, double y, double z) const;
  bool containsRaw(const ZPoint &pt) const;

  /*!
   * \brief Reshape the stack.
   *
   * If the reshaped the volume is the same as the stack voxel number, the stack
   * is reshaped to (\a width x \a height x \a depth) and the function retuns
   * true. Otherwise it returns false and nothing is done.
   */
  bool reshape(int width, int height, int depth);

  int autoThreshold(int ch = 0) const;

  std::vector<double> color(size_t index) const;
  std::vector<double> color(int x, int y, int z) const;

  bool equalColor(size_t index, const std::vector<double> &co) const;
  bool equalColor(size_t index, const std::vector<uint8_t> &co) const;
  bool equalColor(size_t index, const uint8_t *co, size_t length) const;
  bool equalColor(size_t index, size_t channelOffset,
                  const uint8_t *co, size_t length) const;

  void setValue(size_t index, int c, double value);

  /*!
   * \brief Set all voxel values to 0.
   */
  void setZero();

  /*!
   * \brief Set all voxel values to 1.
   */
  void setOne();

  void swapValue(int v1, int v2);

  //Maximum voxel value along a z-parallel line passing (<x>, <y>).
  int maxIntensityDepth(int x, int y, int c = 0) const;

  //If the stack can be thresholded.
  bool isThresholdable();

  //If the stack is tracable
  bool isTracable();

  //If the source of the stack is an swc
  bool isSwc();

  /*!
   * \brief Get raw data point
   *
   * \param c Channel.
   * \param slice Slice index.
   */
  void *getDataPointer(int c, int slice) const;

  /*!
   * \brief Get data pointer starting from a certain loation
   *
   * (\a x, \a y, \a z) is the global coorindates of the starting point.
   */
  const uint8_t* getDataPointer(int x, int y, int z) const;

  /*!
   * \brief Print information of the stack
   */
  void printInfo() const;

  /*!
   * \brief Make a clone of the stack
   */
  ZStack* clone() const;

  /*!
   * \brief Source of the stack.
   */
  const ZStackFile& source() const { return m_source; }

  void deprecateDependent(EComponent component);
  void deprecateSingleChannelView(int channel);
  void deprecate(EComponent component);
  bool isDeprecated(EComponent component) const;
  bool isSingleChannelViewDeprecated(int channel) const;

public: /* data operation */
  //Clean all associated memory except the source
  void clear();
  //void cleanChannel(int c = 0);   //remove content of this channel
  void removeChannel(int c = 0); //remove channel
  //Load stack from Stack, split channel if necessary
  bool load(Stack *stack, bool isOwner = true);

  //Load stack from a file
  bool load(const std::string &filepath, bool initColor = true);
  //bool loadImageSequence(const char *filePath);

  //bool importJsonFile(const std::string &filePath);

  //Load stack from several single channel stack, stack can be null
  bool load(const Stack *ch1, const Stack *ch2, const Stack *ch3);

  // return output file name, some image format might not support some data, so the real file name might be changed
  std::string save(const std::string &filepath) const;
  void setSource(const std::string &filepath, int channel = -1);
  void setSource(const ZStackFile &file);
  void setSource(Stack_Document *stackDoc);
//  void setResolution(double x, double y, double z, char unit);

  // get number of channel for lsm,tiff,raw
  static int getChannelNumber(const std::string &filepath);

  inline const void *rawChannelData() const { return m_stack->array; }
  inline void* rawChannelData() {
    return const_cast<void*>(static_cast<const ZStack&>(*this).rawChannelData());
  }

  const void *rawChannelData(int c) const;
  void* rawChannelData(int c);
  //Stack* channelData(int c);

public: /* operations */

  void* projection(ZSingleChannelStack::Proj_Mode mode,
                   ZSingleChannelStack::Stack_Axis axis = ZSingleChannelStack::Z_AXIS,
                   int c = 0);


  void* projection(NeuTube::EImageBackground bg,
                   ZSingleChannelStack::Stack_Axis axis = ZSingleChannelStack::Z_AXIS,
                   int c = 0);


  void bcAdjustHint(double *scale, double *offset, int c = 0);
  bool isBinary();
  bool updateFromSource();

  bool hasSameValue(size_t index1, size_t index2);
  bool hasSameValue(size_t index1, size_t index2, size_t channelOffset);

  ZStack* createSubstack(const std::vector<std::vector<double> > &selected);

  double averageIntensity(ZStack *mask);

  /*!
   * \brief Copy values from a buffer to the stack.
   */
  void loadValue(const void *buffer, size_t length, int ch = 0);

  void loadValue(const void *buffer, size_t length, void *loc);

  void setOffset(int dx, int dy);
  void setOffset(int dx, int dy, int dz);
  void setOffset(const ZIntPoint &pt);
  void setZOffset(int z);
  inline const ZIntPoint& getOffset() const { return m_offset; }
  inline ZIntPoint& getOffset() { return m_offset; }

  /*!
   * \brief Translate the stack.
   *
   * Add (\a dx, \a dy, \a dz) to the stack offset.
   */
  void translate(int dx, int dy, int dz);

  void translate(const ZIntPoint &pt);

  /*!
   * \brief Test if a stack has non-zero offset
   *
   * \return true iff the offset of any dimension is not zero
   */
  bool hasOffset() const;

  /*!
   * \brief Paste the values of a stack to another stack
   *
   * The stacks are aligned with their offsets while pasting. The values of
   * \a dst are set to those in the source stack. Any value out of the source
   * range is untouched. The stacks must have same kind, which should be either
   * GREY or GREY16. \a valueIgnored is to mask \a dst.
   *
   * \return true iff the pasting can be performed.
   */
  bool paste(ZStack *dst, int valueIgnored = -1, double alpha = 1.0) const;

  /*!
   * \brief Get the bound box of the stack.
   *
   * The result is stored in \a box. Not the box covers the whole stack, not
   * just the foreground.
   */
  void getBoundBox(Cuboid_I *box) const;

  ZIntCuboid getBoundBox() const;

  void setBlockValue(int x0, int y0, int z0, const ZStack *stack);

public: /* processing routines */
  bool binarize(int threshold = 0);
  bool bwsolid();
  bool bwperim();
  bool enhanceLine();
  void extractChannel(int c);
  Stack* copyChannel(int c);
  bool watershed(int c = 0);
//  inline const ZResolution& getResolution() const { return m_resolution; }

  void pushDsIntv(int dx, int dy, int dz);

  ZIntPoint getDsIntv() const {
    return m_dsIntv;
  }

  void setDsIntv(const ZIntPoint &dsIntv) {
    m_dsIntv = dsIntv;
  }

  /*!
   * \brief Downsample the stack with maximum assignment.
   *
   * The offset postion is adjusted accordingly.
   */
  void downsampleMax(int xintv, int yintv, int zintv);

  /*!
   * \brief Downsample the stack with mininum assignment.
   *
   * The offset postion is adjusted accordingly.
   */
  void downsampleMin(int xintv, int yintv, int zintv);

  void crop(const ZIntCuboid &cuboid);
  ZStack* makeCrop(const ZIntCuboid &cuboid) const;

  void swapData(ZStack *stack);

public:
  void initChannelColors();

  struct LsmInfo {
    LsmInfo() {}
    Cz_Lsminfo m_basicInfo;
    Lsm_Channel_Colors m_lsmChannelInfo;
    Lsm_Time_Stamp_Info m_lsmTimeStampInfo;
    std::vector<std::string> m_lsmChannelNames;
    std::vector<double> m_lsmTimeStamps;
    std::vector<int> m_lsmChannelDataTypes;
  };

  // read lsm file, fill Cz_Lsminfo, Lsm_Channel_Colors and channel names and colors
#ifdef _NEUTUBE_
  std::vector<ZVec3Parameter*>& channelColors() {
    initChannelColors(); return m_channelColors; }
  glm::vec3 getChannelColor(size_t ch) {
    initChannelColors(); return m_channelColors[ch]->get(); }

  bool loadLSMInfo(const QString &filepath);
  void logLSMInfo();
  void setChannelColor(int ch, double r, double g, double b);
  const Cz_Lsminfo& getLSMInfo() const { return m_lsmInfo.m_basicInfo; }
#endif

private:
  ZStack(const ZStack &src); //uncopyable

  void init();
  bool canMerge(const Stack *s1, const Stack *s2);
  void setChannelNumber(int nchannel);
  //shift one channel
  void shiftLocation(
      int *offset, int c = 0, int width = -1, int height = -1, int depth = -1);

  // subtract most common value of the histogram from this stack, use Stack_Sub_Common
  void subMostCommonValue(int c);
  // get average of all channels
  Stack* averageOfAllChannels();
  double saturatedIntensity() const;


private:
  Mc_Stack *m_stack; //Master data
  C_Stack::Mc_Stack_Deallocator *m_dealloc; //Dellocator of the master data
  ZIntPoint m_offset;
  ZIntPoint m_dsIntv; //Downsampling ratio from original space

  ZStackFile m_source;

  mutable std::vector<Stack> m_stackView;
  mutable std::vector<ZSingleChannelStack*> m_singleChannelStack;
  mutable char m_buffer[1]; //Buffer of text field of temporary stack

  //float color for each channel

//  bool m_isLSMFile;

#ifdef _NEUTUBE_
  std::vector<ZVec3Parameter*> m_channelColors;

  LsmInfo m_lsmInfo;
//  Cz_Lsminfo m_lsmInfo;
//  Lsm_Channel_Colors m_lsmChannelInfo;
//  Lsm_Time_Stamp_Info m_lsmTimeStampInfo;
//  std::vector<QString> m_lsmChannelNames;
//  std::vector<double> m_lsmTimeStamps;
//  std::vector<int> m_lsmChannelDataTypes;
#endif
};


#endif
