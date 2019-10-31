#ifndef ZFLYEMPROOFMVCCONTROLLER_H
#define ZFLYEMPROOFMVCCONTROLLER_H

#include <cstdint>
#include <memory>
#include <QSet>

class ZFlyEmProofMvc;
class ZIntPoint;
class ZStackDoc;
class ZIntCuboid;
class ZDvidLabelSlice;

/*!
 * \brief An experimental class for controlling mvc widgets
 */
class ZFlyEmProofMvcController
{
public:
  ZFlyEmProofMvcController();

public:
  static void GoToBody(ZFlyEmProofMvc *mvc, uint64_t bodyId);
  static void SelectBody(ZFlyEmProofMvc *mvc, uint64_t bodyId);
  static void SelectBody(ZFlyEmProofMvc *mvc, const QSet<uint64_t> &bodySet);
  static void GoToPosition(ZFlyEmProofMvc *mvc, const ZIntPoint &pos);
  static void Close(ZFlyEmProofMvc *mvc);
  static void EnableHighlightMode(ZFlyEmProofMvc *mvc);
  static void Disable3DVisualization(ZFlyEmProofMvc *mvc);
  static void DisableSequencer(ZFlyEmProofMvc *mvc);
  static void DisableContextMenu(ZFlyEmProofMvc *mvc);
  static void SetTodoDelegate(ZFlyEmProofMvc *mvc, ZStackDoc *todoDoc);
  static void UpdateProtocolRangeGlyph(
      ZFlyEmProofMvc *mvc, const ZIntCuboid &range);
  static void SetProtocolRangeGlyphVisible(ZFlyEmProofMvc *mvc, bool visible);
};

#endif // ZFLYEMPROOFMVCCONTROLLER_H
