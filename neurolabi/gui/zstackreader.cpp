#include "zstackreader.h"

#include "geometry/zintcuboid.h"
#include "zstack.hxx"
#include "zfiletype.h"
#include "zjsonobject.h"
#include "zstackfactory.h"
#include "zjsonobjectparser.h"

#if defined(_ENABLE_LIBDVIDCPP_)
#include "dvid/zdvidreader.h"
#endif

#if defined(_QT_GUI_USED_)

#  ifdef _QT5_
#include <QUrlQuery>
#  endif

#include <QUrl>
#include <QFileInfo>
#include <QDir>

#endif

ZStackReader::ZStackReader()
{
}

ZStack* ZStackReader::ReadDvid(const QUrl &url)
{
  ZStack *stack = nullptr;

#if defined(_ENABLE_LIBDVIDCPP_)
  QString urlPath = url.path();
  QStringList parts = urlPath.split('/', QString::SkipEmptyParts);

  ZIntCuboid range = GetRange(url);

  if (parts.size() == 2 && !range.isEmpty()) {
    ZDvidTarget target;
    target.set(url.host().toStdString(), parts[0].toStdString(), url.port(-1));


    target.setGrayScaleName(parts[1].toStdString());
    target.setSegmentationName("*");
    ZDvidReader reader;
    if (reader.open(target)) {
      stack = reader.readGrayScale(range);
    }
  }
#else
  Q_UNUSED(url);
#endif

  return stack;
}

ZStack* ZStackReader::ReadSeries(const QUrl &url)
{
  ZStack *stack = nullptr;

#if defined(_QT5_)
  QUrlQuery query(url);
  QString prefix = query.queryItemValue("prefix");
  QString suffix = query.queryItemValue("suffix");
#else
  QString prefix = url.queryItemValue("prefix");
  QString suffix = url.queryItemValue("suffix");
#endif
  QDir dir(url.path());
  QFileInfoList fileList =
      dir.entryInfoList(QStringList() << prefix + "*" + suffix);

  if (!fileList.isEmpty()) {
    QString filePath = fileList[0].absoluteFilePath();
    int nchannel = ZStack::getChannelNumber(filePath.toStdString());
    Stack *slice = Read_Sc_Stack(filePath.toLocal8Bit(), 0);
    int kind = C_Stack::kind(slice);
    int width = C_Stack::width(slice);
    int height = C_Stack::height(slice);
    int depth = fileList.size();
    Mc_Stack *stackData = C_Stack::make(kind, width, height, depth, nchannel);
    C_Stack::kill(slice);
    slice = NULL;

    for (int i = 0; i < nchannel; i++) {
      for (int j = 0; j < fileList.size(); j++) {
        slice = Read_Sc_Stack(fileList[j].absoluteFilePath().toLocal8Bit(), i);
        C_Stack::copyPlaneValue(stackData, slice->array, i, j);
        C_Stack::kill(slice);
        slice = NULL;
      }
    }

    if (stack == NULL) {
      stack = new ZStack;
    }
    stack->setData(stackData);
  }

  return stack;
}

ZIntCuboid ZStackReader::GetRange(const QUrl &url)
{
  ZIntCuboid box;
#ifdef _QT5_
  QUrlQuery query(url.query());
  QString x0Str = query.queryItemValue("x0");
  int x0 = x0Str.toInt();

  QString y0Str = query.queryItemValue("y0");
  int y0 = y0Str.toInt();

  QString z0Str = query.queryItemValue("z0");
  int z0 = z0Str.toInt();

  QString x1Str = query.queryItemValue("x1");
  int x1 = x1Str.toInt();

  QString y1Str = query.queryItemValue("y1");
  int y1 = y1Str.toInt();

  QString z1Str = query.queryItemValue("z1");
  int z1 = z1Str.toInt();

  box.set(x0, y0, z0, x1, y1, z1);

  QString widthStr = query.queryItemValue("width");
  if (!widthStr.isEmpty()) {
    box.setWidth(widthStr.toInt());
  }

  QString heightStr = query.queryItemValue("height");
  if (!heightStr.isEmpty()) {
    box.setHeight(heightStr.toInt());
  }

  QString depthStr = query.queryItemValue("depth");
  if (!depthStr.isEmpty()) {
    box.setDepth(depthStr.toInt());
  }
#else
  QString x0Str = url.queryItemValue("x0");
  int x0 = x0Str.toInt();

  QString y0Str = url.queryItemValue("y0");
  int y0 = y0Str.toInt();

  QString z0Str = url.queryItemValue("z0");
  int z0 = z0Str.toInt();

  QString x1Str = url.queryItemValue("x1");
  int x1 = x1Str.toInt();

  QString y1Str = url.queryItemValue("y1");
  int y1 = y1Str.toInt();

  QString z1Str = url.queryItemValue("z1");
  int z1 = z1Str.toInt();

  box.set(x0, y0, z0, x1, y1, z1);

  QString widthStr = url.queryItemValue("width");
  if (!widthStr.isEmpty()) {
    box.setWidth(widthStr.toInt());
  }

  QString heightStr = url.queryItemValue("height");
  if (!heightStr.isEmpty()) {
    box.setHeight(heightStr.toInt());
  }

  QString depthStr = url.queryItemValue("depth");
  if (!depthStr.isEmpty()) {
    box.setDepth(depthStr.toInt());
  }
#endif
  return box;
}

ZStack* ZStackReader::ReadJson(const ZJsonObject &obj)
{
  ZJsonObjectParser parser;

  //Json {"source": <path>, "offset": [x0, y0, z0]}
  std::string source = parser.getValue(obj, "source", "");
  ZStack *stack = Read(source);
  if (stack) {
    std::vector<int64_t> offset =
        parser.getValue(obj, "offset", std::vector<int64_t>());
    if (offset.size() == 3) {
      stack->setOffset(
            stack->getOffset() + ZIntPoint(offset[0], offset[1], offset[2]));
    }
  }

  return stack;
}

ZStack* ZStackReader::Read(const std::string &path)
{
  ZStack *stack = NULL;

#if defined(_QT_GUI_USED_)
  QUrl url(path.c_str());
  if (url.scheme() == "dvid") {
    //Initial design of dvid scheme:
    //  dvid://<host>:<port>/<uuid>/<dataname>?<query>
    //    query: x0, y0, z0, x1, y1, z1, width, height, depth
    //           (width, height, depth) overwrite (x1, y1, z1)
#if defined(_ENABLE_LIBDVIDCPP_)
    stack = ReadDvid(url);
#endif
  } else if (url.scheme() == "file") {
    //File scheme
    //  file://<path>?<query>
    //    query: prefix=<prefix>&suffix=<suffix>&numwidth=<suffix>
    stack = ReadSeries(url);
  } else {
    ZStackFile stackFile;
    stackFile.import(path);
    stack = stackFile.readStack();
    if (stack == nullptr) {
      if (ZFileType::FileType(path) == ZFileType::EFileType::JSON) {
        ZJsonArray json;
        json.load(path);
        std::vector<ZStack*> stackArray;
        for (size_t i = 0; i < json.size(); ++i) {
          ZStack *substack = ReadJson(ZJsonObject(json.value(i)));
          if (substack) {
            stackArray.push_back(substack);
          }
        }
        stack = ZStackFactory::Compose(stackArray);
      }
    }
  }
#endif


  return stack;
}
