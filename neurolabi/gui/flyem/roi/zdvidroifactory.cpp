#include "zdvidroifactory.h"

#include "zobject3dscan.h"
#include "zmeshfactory.h"

#include "logging/zlog.h"

#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"

#include "flyem/flyemdatareader.h"

ZDvidRoiFactory::ZDvidRoiFactory()
{

}

void ZDvidRoiFactory::setSource(ESource source)
{
  m_source = source;
}

void ZDvidRoiFactory::setDvidTarget(const ZDvidTarget &target)
{
  if (target.isValid()) {
    ZDvidReader *reader = new ZDvidReader;
    if (reader->open(target)) {
      m_reader = std::shared_ptr<ZDvidReader>(reader);
    } else {
      m_reader.reset();
    }
  }
}

ZMesh* ZDvidRoiFactory::makeRoiMesh(const std::string &name) const
{
  ZMesh *mesh = nullptr;
  if (m_reader) {
    switch (m_source) {
    case ESource::ROI:
    {
      ZObject3dScan roi;
      m_reader->readRoi(name, &roi);
      if (!roi.isEmpty()) {
        ZMeshFactory mf;
        mf.setOffsetAdjust(true);
        mesh = mf.makeMesh(roi);
      }
    }
      break;
    case ESource::MESH:
      mesh = FlyEmDataReader::ReadRoiMesh(
            *m_reader, name,
            [](const std::string &msg) { LKWARN(neutu::TOPIC_NULL) << msg; });
      break;
    }
  }

  return mesh;
}
