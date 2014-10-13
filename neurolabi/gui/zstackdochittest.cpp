#include "zstackdochittest.h"
#include "zstackdoc.h"
#include "zstroke2d.h"
#include "zpoint.h"

ZStackDocHitTest::ZStackDocHitTest() : m_hitSwcNode(NULL), m_hitStroke(NULL)
{
}

bool ZStackDocHitTest::hitTest(const ZStackDoc *doc, double x, double y)
{
  m_hitStroke = NULL;
  m_hitSwcNode = NULL;

  const QList<ZStroke2d*> strokeList = doc->getStrokeList();
  foreach (const ZStroke2d *stroke, strokeList) {
    if (stroke->hitTest(x, y)) {
      m_hitStroke = const_cast<ZStroke2d*>(stroke);
      break;
    }
  }

  if (m_hitStroke == NULL) {
    m_hitSwcNode = doc->swcHitTest(x, y);
  }

  return m_hitStroke != NULL;
}

bool ZStackDocHitTest::hitTest(const ZStackDoc *doc, const ZPoint &pt)
{
  return hitTest(doc, pt.x(), pt.y(), pt.z());
}

bool ZStackDocHitTest::hitTest(
    const ZStackDoc *doc, double x, double y, double z)
{
  m_hitStroke = NULL;
  m_hitSwcNode = NULL;

  const QList<ZStroke2d*> strokeList = doc->getStrokeList();
  foreach (const ZStroke2d *stroke, strokeList) {
    if (stroke->hitTest(x, y, z)) {
      m_hitStroke = const_cast<ZStroke2d*>(stroke);
      break;
    }
  }

  if (m_hitStroke == NULL) {
    m_hitSwcNode = doc->swcHitTest(x, y, z);
  }

  return m_hitStroke != NULL;
}
