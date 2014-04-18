#include "zflyemdatainfo.h"
#include "zerror.h"
#include "tz_error.h"

ZFlyEmDataInfo::ZFlyEmDataInfo(FlyEm::EDataSet dataSet) : m_blockMargin(0)
{
  m_stackSize.resize(3);
  m_voxelResolution.resize(3);
  m_startCoordinates.resize(3);

  configure(dataSet);
}

void ZFlyEmDataInfo::configure(FlyEm::EDataSet dataSet)
{
  m_dataSet = dataSet;
  switch (dataSet) {
  case FlyEm::DATA_FIB25:
    m_stackSize[0] = 3150;
    m_stackSize[1] = 2599;
    m_stackSize[2] = 6500;
    m_voxelResolution[0] = 10.0;
    m_voxelResolution[1] = 10.0;
    m_voxelResolution[2] = 10.0;
    m_startCoordinates[0] = 0;
    m_startCoordinates[1] = 0;
    m_startCoordinates[2] = 1490;
    m_dvidAddress = "http://emdata1.int.janelia.org:7000";
    m_dvidUuid = "a75";
    break;
  default:
    RECORD_ERROR_UNCOND("Unsupported dataset")
    break;
  }
}
