#include "zjsonfactory.h"

#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zobject3dscan.h"
#include "zintpoint.h"
#include "zintcuboid.h"

#ifdef _QT_GUI_USED_
#include <QString>
#include "flyem/zflyembookmark.h"
#endif

ZJsonFactory::ZJsonFactory()
{
}

ZJsonArray ZJsonFactory::MakeJsonArray(
    const ZObject3dScan &obj, ZJsonFactory::EObjectForm form)
{
  ZJsonArray array;

  ZObject3dScan::ConstSegmentIterator iterator(&obj);

  while (iterator.hasNext()) {
    const ZObject3dScan::Segment &seg = iterator.next();

    switch (form) {
    case ZJsonFactory::OBJECT_SPARSE:
    {
      ZJsonArray segmentObj;
      segmentObj.append(seg.getZ());
      segmentObj.append(seg.getY());
      segmentObj.append(seg.getStart());
      segmentObj.append(seg.getEnd());
      array.append(segmentObj);
    }
      break;
    case ZJsonFactory::OBJECT_DENSE:
    {
      for (int x = seg.getStart(); x <= seg.getEnd(); ++x) {
        ZJsonArray pointObj;
        pointObj.append(x);
        pointObj.append(seg.getY());
        pointObj.append(seg.getZ());
        array.append(pointObj);
      }
    }
      break;
    }
  }

  return array;
}

ZJsonArray ZJsonFactory::MakeJsonArray(const ZIntPoint &pt)
{
  ZJsonArray array;
  array.append(pt.getX());
  array.append(pt.getY());
  array.append(pt.getZ());

  return array;
}

ZJsonArray ZJsonFactory::MakeJsonArray(const ZIntCuboid &box)
{
  ZJsonArray array;
  array.append(box.getFirstCorner().getX());
  array.append(box.getFirstCorner().getY());
  array.append(box.getFirstCorner().getZ());

  array.append(box.getLastCorner().getX());
  array.append(box.getLastCorner().getY());
  array.append(box.getLastCorner().getZ());

  return array;
}

#if defined(_QT_GUI_USED_)
ZJsonObject ZJsonFactory::MakeAnnotationJson(const ZFlyEmBookmark &bookmark)
{
  ZJsonObject json;
  ZJsonArray posJson = MakeJsonArray(bookmark.getCenter().toIntPoint());
  json.setEntry("Pos", posJson);
  json.setEntry("Kind", "Note");

  ZJsonArray tagJson;
  foreach (const QString &tag, bookmark.getTags()) {
    tagJson.append(tag.toStdString());
  }

  /*
  if (!bookmark.getUserName().isEmpty()) {
    tagJson.append("user:" + bookmark.getUserName().toStdString());
  }
  */
  if (!tagJson.isEmpty()) {
    json.setEntry("Tags", tagJson);
  }

  ZJsonObject propJson;
  propJson.setEntry(
        "body ID", QString("%1").arg(bookmark.getBodyId()).toStdString());
  propJson.setEntry("time", bookmark.getTime().toStdString());
  propJson.setEntry("status", bookmark.getStatus().toStdString());
  propJson.setEntry("comment", bookmark.getComment().toStdString());
  propJson.setEntry("type", bookmark.getTypeString().toStdString());
  if (bookmark.isChecked()) {
    propJson.setEntry("checked", "1");
  }
  if (bookmark.isCustom()) {
    propJson.setEntry("custom", "1");
  } else {
    propJson.setEntry("custom", "0");
  }
  propJson.setEntry("user", bookmark.getUserName().toStdString());

  std::map<std::string, json_t*> additionalProp =
      bookmark.getPropertyJson().toEntryMap(false);
  for (std::map<std::string, json_t*>::iterator iter = additionalProp.begin();
       iter != additionalProp.end(); ++iter) {
    const std::string &key = iter->first;
    if (!propJson.hasKey(key.c_str())) {
      propJson.setEntry(key.c_str(), iter->second);
    }
  }

  json.setEntry("Prop", propJson);

  return json;
}

ZJsonArray ZJsonFactory::MakeJsonArray(
    const std::vector<ZFlyEmBookmark *> &bookmarkArray)
{
  ZJsonArray array;
  for (std::vector<ZFlyEmBookmark*>::const_iterator
       iter = bookmarkArray.begin(); iter != bookmarkArray.end(); ++iter) {
    const ZFlyEmBookmark *bookmark = *iter;
    array.append(MakeAnnotationJson(*bookmark));
  }

  return array;
}

ZJsonArray ZJsonFactory::MakeJsonArray(const QMap<uint64_t, uint64_t> &map)
{
  ZJsonArray array;

  for (QMap<uint64_t, uint64_t>::const_iterator iter = map.begin();
       iter != map.end(); ++iter) {
    ZJsonArray pair;
    pair.append(iter.key());
    pair.append(iter.value());
    array.append(pair.getData());
  }

  return array;
}
#endif
