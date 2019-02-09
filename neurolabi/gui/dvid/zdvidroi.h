#ifndef ZDVIDROI_H
#define ZDVIDROI_H

#include "zobject3dscan.h"
#include "dvid/zdvidinfo.h"

class ZDvidRoi
{
public:
  ZDvidRoi();
  ~ZDvidRoi();

  ZObject3dScan* getRoiRef() {
    return &m_roi;
  }

  ZIntPoint getBlockSize() const;
  std::string getName() const;
  size_t getVolume() const;

  void clear();
  void setName(const std::string &name);
  void setBlockSize(int s);
  void setBlockSize(int w, int h, int d);
  void setBlockSize(const ZIntPoint &blockSize);



  bool isEmpty() const;

  /*!
   * \brief Test if a point is contained in the ROI
   *
   * \return true iff (\a x, \a y, \a z) is in ROI.
   */
  bool contains(int x, int y, int z) const;
  bool contains(const ZIntPoint &pt) const;

private:
  ZObject3dScan m_roi;
  ZIntPoint m_blockSize;
  std::string m_name;
};

#endif // ZDVIDROI_H
