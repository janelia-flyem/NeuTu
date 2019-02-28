#include "flyemdatawriter.h"

#include "geometry/zintpoint.h"
#include "zobject3dscan.h"
#include "dvid/zdvidwriter.h"
#include "zflyembodyannotation.h"
#include "flyemdatareader.h"

FlyEmDataWriter::FlyEmDataWriter()
{

}

void FlyEmDataWriter::UpdateBodyStatus(
    ZDvidWriter &writer, const ZIntPoint &pos, const std::string &newStatus)
{
  uint64_t bodyId = writer.getDvidReader().readBodyIdAt(pos);
  ZFlyEmBodyAnnotation annot =
      FlyEmDataReader::ReadBodyAnnotation(writer.getDvidReader(), bodyId);
#ifdef _DEBUG_
  std::cout << "Old annotation:" << std::endl;
  annot.print();
#endif
  annot.setStatus(newStatus);
  writer.writeBodyAnntation(annot);

#ifdef _DEBUG_
  ZFlyEmBodyAnnotation newAnnot =
      FlyEmDataReader::ReadBodyAnnotation(writer.getDvidReader(), bodyId);
  std::cout << "New annotation:" << std::endl;
  newAnnot.print();
#endif
}

void FlyEmDataWriter::RewriteBody(ZDvidWriter &writer, uint64_t bodyId)
{
  uint64_t newBodyId = 0;

  if (writer.good()) {
    ZObject3dScan obj;
    writer.getDvidReader().readBody(bodyId, false, &obj);

    if (!obj.isEmpty()) {
      newBodyId = writer.writeSplit(obj, bodyId, 0);
      //      std::cout << newBodyId << std::endl;

      if (newBodyId > 0) {
        ZFlyEmBodyAnnotation annotation =
            FlyEmDataReader::ReadBodyAnnotation(writer.getDvidReader(), bodyId);

        if (!annotation.isEmpty()) {
          writer.deleteBodyAnnotation(bodyId);
          annotation.setBodyId(newBodyId);
          writer.writeBodyAnntation(annotation);
        }
      }
    }
  }
}
