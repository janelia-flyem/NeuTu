#ifndef ZDVIDINFO_H
#define ZDVIDINFO_H

#include <vector>
#include <string>

class ZDvidInfo
{
public:
  ZDvidInfo();

  /*!
   * \brief Set info from a json string
   *
   * Properties without correspondng fields will be set to default values.
   *
   * \param str Input json string.
   */
  void setFromJsonString(const std::string &str);

  void print() const;

  /*!
   * \brief Get block index of a certain point.
   *
   * \param x X coordinate of the point.
   * \param y Y coordinate of the point.
   * \param z Z coordinate of the point.
   *
   * \return Empty array if the point is out of range.
   */
  std::vector<int> getBlockIndex(double x, double y, double z);

  inline const std::vector<double>& getVoxelResolution() {
    return m_voxelResolution;
  }

private:
  std::vector<int> m_stackSize;
  std::vector<double> m_voxelResolution;
  std::vector<int> m_startCoordinates;
  std::vector<int> m_startBlockIndex;
  std::vector<int> m_blockSize;

  std::string m_dvidAddress;
  int m_dvidPort;
  std::string m_dvidUuid;

  const static int m_defaultBlockSize;

  //Json keys
  const static char* m_minPointKey;
  const static char* m_maxPointKey;
  const static char* m_blockSizeKey;
  const static char* m_voxelSizeKey;
  const static char* m_blockMinIndexKey;
};

#endif // ZDVIDINFO_H
