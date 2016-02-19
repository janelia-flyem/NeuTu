#include "zflyemdatainfo.h"
#include "zerror.h"
#include "tz_error.h"
#include "zstring.h"
#include "dvid/zdvidinfo.h"

ZFlyEmDataInfo::ZFlyEmDataInfo(FlyEm::EDataSet dataSet) : m_blockMargin(0)
{
  m_stackSize.resize(3);
  m_voxelResolution.resize(3);
  m_startCoordinates.resize(3);

  configure(dataSet);
}

void ZFlyEmDataInfo::configure(const ZDvidInfo &info)
{
  m_stackSize = info.getStackSize();
  m_voxelResolution[0] = info.getVoxelResolution().voxelSizeX();
  m_voxelResolution[1] = info.getVoxelResolution().voxelSizeY();
  m_voxelResolution[2] = info.getVoxelResolution().voxelSizeZ();
  m_startCoordinates[0] = info.getStartCoordinates().getX();
  m_startCoordinates[1] = info.getStartCoordinates().getY();
  m_startCoordinates[2] = info.getStartCoordinates().getZ();
}

void ZFlyEmDataInfo::configure(FlyEm::EDataSet dataSet)
{
  m_dataSet = dataSet;
  switch (dataSet) {
  case FlyEm::DATA_FIB25:
    m_stackSize[0] = 3150;
    m_stackSize[1] = 2599;
    m_stackSize[2] = 6520;
    m_voxelResolution[0] = 10.0;
    m_voxelResolution[1] = 10.0;
    m_voxelResolution[2] = 10.0;
    m_startCoordinates[0] = 0;
    m_startCoordinates[1] = 0;
    m_startCoordinates[2] = 1490;
    m_blockMargin = 10;
    m_dvidAddress = "http://emdata2.int.janelia.org";
    m_dvidPort = 9000;
    m_dvidUuid = "fa9";
    break;
  case FlyEm::DATA_FIB25_7C:
    m_stackSize[0] = 6445;
    m_stackSize[1] = 6642;
    m_stackSize[2] = 8089;
    m_voxelResolution[0] = 8.0;
    m_voxelResolution[1] = 8.0;
    m_voxelResolution[2] = 8.0;
    m_startCoordinates[0] = 0;
    m_startCoordinates[1] = 0;
    m_startCoordinates[2] = 0;
    m_blockMargin = 10;
    m_dvidAddress = "http://emdata2.int.janelia.org";
    m_dvidPort = -1;
    m_dvidUuid = "2b6c";
    break;
  default:
    RECORD_ERROR_UNCOND("Unsupported dataset")
    break;
  }
}

void ZFlyEmDataInfo::setDvidPort(int port)
{
  m_dvidPort = port;
}

std::string ZFlyEmDataInfo::getDvidAddressWithPort() const
{
  if (m_dvidPort >= 0) {
    return getDvidAddress() + ":" + ZString::num2str(m_dvidPort);
  }

  return getDvidAddress();
}
