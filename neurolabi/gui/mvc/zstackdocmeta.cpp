#include "zstackdocmeta.h"

#include "zjsonobjectparser.h"

ZStackDocMeta::ZStackDocMeta()
{
}

bool ZStackDocMeta::allowingTracing() const
{
  return m_allowingTracing;
}

bool ZStackDocMeta::allowingSegmentation() const
{
  return m_allowingSegmentation;
}

void ZStackDocMeta::allowTracing(bool on)
{
  m_allowingTracing = on;
}


void ZStackDocMeta::loadJsonObject(const ZJsonObject &obj)
{
  ZJsonObjectParser parser;
  m_allowingTracing = parser.getValue(obj, "tracing", false);
  m_allowingSegmentation = parser.getValue(obj, "segmentation", true);
}
