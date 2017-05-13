#include "zstackreader.h"

#include "zintcuboid.h"

#if defined(_ENABLE_LIBDVIDCPP_)
#include "dvid/zdvidreader.h"
#endif

#if defined(_QT_GUI_USED_)
#ifdef _QT5_
#include<QUrlQuery>
#endif
#include <QUrl>
#endif

ZStackReader::ZStackReader()
{

}

ZIntCuboid ZStackReader::GetRange(const QUrl &url)
{
  ZIntCuboid box;
#if defined(_QT_GUI_USED_)

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

ZStack* ZStackReader::read(const std::string &path)
{
  ZStack *stack = NULL;

#if defined(_QT_GUI_USED_) && defined(_ENABLE_LIBDVIDCPP_)
  QUrl url(path.c_str());
  //Initial design of dvid scheme:
  //  dvid://<host>:<port>/<uuid>/<dataname>?<query>
  if (url.scheme() == "dvid") {
    QString urlPath = url.path();
    QStringList parts = urlPath.split('/', QString::SkipEmptyParts);

    ZIntCuboid range = GetRange(url);

    if (parts.size() == 2 && !range.isEmpty()) {
      ZDvidTarget target;
      target.set(url.host().toStdString(), parts[0].toStdString(), url.port(-1));
      target.setGrayScaleName(parts[1].toStdString());
      target.setLabelBlockName("*");
      ZDvidReader reader;
      if (reader.open(target)) {
        stack = reader.readGrayScale(range);
      }
    }
  }
#endif
#endif

  return stack;
}
