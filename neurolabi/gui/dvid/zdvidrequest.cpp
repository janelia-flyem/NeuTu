#include "zdvidrequest.h"
#include <QVector>

ZDvidRequest::ZDvidRequest()
{
}

void ZDvidRequest::setGetObjectRequest(int bodyId)
{
  m_requestType = ZDvidRequest::DVID_GET_OBJECT;
  m_parameter = bodyId;
}

void ZDvidRequest::setGetSwcRequest(int bodyId)
{
  m_requestType = ZDvidRequest::DVID_GET_SWC;
  m_parameter = bodyId;
}

void ZDvidRequest::setGetThumbnailRequest(int bodyId)
{
  m_requestType = ZDvidRequest::DVID_GET_THUMBNAIL;
  m_parameter = bodyId;
}

void ZDvidRequest::setGetImageRequest(
    int x0, int y0, int z0, int width, int height)
{
  m_requestType = ZDvidRequest::DVID_GET_GRAY_SCALE;
  QList<QVariant> parameter;
  parameter << x0 << y0 << z0 << width << height;
  m_parameter = parameter;
}

void ZDvidRequest::setGetImageRequest(
    int x0, int y0, int z0, int width, int height, int depth)
{
  m_requestType = ZDvidRequest::DVID_GET_GRAY_SCALE;
  QList<QVariant> parameter;
  parameter << x0 << y0 << z0 << width << height << depth;
  m_parameter = parameter;
}

void ZDvidRequest::setGetBodyLabelRequest(
    int x0, int y0, int z0, int width, int height, int depth)
{
  m_requestType = ZDvidRequest::DVID_GET_BODY_LABEL;
  QList<QVariant> parameter;
  parameter << x0 << y0 << z0 << width << height << depth;
  m_parameter = parameter;
}

void ZDvidRequest::setGetBodyLabelRequest(const QString &dataName,
    int x0, int y0, int z0, int width, int height, int depth)
{
  m_requestType = ZDvidRequest::DVID_GET_BODY_LABEL;
  QList<QVariant> parameter;
  parameter << x0 << y0 << z0 << width << height << depth << dataName;
  m_parameter = parameter;
}

void ZDvidRequest::setGetInfoRequest(const QString &dataType)
{
  if (dataType == "superpixels") {
    m_requestType = ZDvidRequest::DVID_GET_SUPERPIXEL_INFO;
  } else if (dataType == "grayscale") {
    m_requestType = ZDvidRequest::DVID_GET_GRAYSCALE_INFO;
  } else {
    m_requestType = ZDvidRequest::DVID_NULL_REQUEST;
  }
}

void ZDvidRequest::setGetStringRequest(const QString &dataType)
{
  if (dataType == "sp2body") {
    m_requestType = ZDvidRequest::DVID_GET_SP2BODY_STRING;
  } else {
    m_requestType = ZDvidRequest::DVID_NULL_REQUEST;
  }
}

void ZDvidRequest::setGetKeyValueRequest(const QString &dataName, const QString &key)
{
  m_requestType = ZDvidRequest::DVID_GET_KEYVALUE;
  QList<QVariant> parameter;
  parameter << dataName << key;
  m_parameter = parameter;
}

void ZDvidRequest::setGetKeysRequest(
    const QString &dataName, const QString &minKey, const QString &maxKey)
{
  m_requestType = ZDvidRequest::DVID_GET_KEYS;
  QList<QVariant> parameter;
  parameter << dataName << minKey << maxKey;
  m_parameter = parameter;
}

void ZDvidRequest::setParameter(const QVariant &parameter)
{
  m_parameter = parameter;
}
