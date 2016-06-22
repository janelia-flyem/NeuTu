/**@file zinteractivecontext.h
 * @brief Interaction context
 * @author Ting Zhao
 */
#ifndef ZINTERACTIVECONTEXT_H
#define ZINTERACTIVECONTEXT_H

#include <QRect>
#include "neutube_def.h"

class ZPoint;
class ZImageWidget;

class ZInteractiveContext
{
public:
  enum TraceMode {
    TRACE_OFF = 0,
    TRACE_SINGLE,
    TRACE_TUBE,
    TRACE_MANUAL,
    TRACE_PREIVEW_RECONSTRUCTION
  };

  enum MarkPunctaMode {
    MARK_PUNCTA_OFF = 0,
    MARK_PUNCTA
  };

  enum BookmarkEditMode {
    BOOKMARK_EDIT_OFF = 0,
    BOOKMARK_ADD
  };

  enum TubeEditMode {
    TUBE_EDIT_OFF = 0,
    TUBE_EDIT_HOOK,
    TUBE_EDIT_SP_HOOK,
    TUBE_EDIT_LINK,
    TUBE_EDIT_MERGE,
    TUBE_EDIT_WALK,
    TUBE_EDIT_CHECK_CONN,
    TUBE_EDIT_EXTEND,
    TUBE_EDIT_CONNECT,
    TUBE_EDIT_DISCONNECT
  };

  enum SwcEditMode {
    SWC_EDIT_OFF = 0,
    SWC_EDIT_SELECT,
    SWC_EDIT_CONNECT,
    SWC_EDIT_EXTEND,
    SWC_EDIT_SMART_EXTEND,
    SWC_EDIT_LOCK_FOCUS,
    SWC_EDIT_ADD_NODE,
    SWC_EDIT_MOVE_NODE
  };

  enum StrokeEditMode {
    STROKE_EDIT_OFF = 0,
    STROKE_DRAW
  };

  enum RectEditMode {
    RECT_EDIT_OFF = 0,
    RECT_DRAW
  };

  enum SynapseEditMode {
    SYNAPSE_EDIT_OFF = 0,
    SYNAPSE_ADD_PRE,
    SYNAPSE_ADD_POST,
    SYNAPSE_MOVE
  };

  enum ViewMode {
    VIEW_NORMAL,
    VIEW_PROJECT,
    VIEW_OBJECT_PROJECT
  };

  enum ExploreMode {
    EXPLORE_OFF = 0,
    EXPLORE_MOVE_IMAGE,
    EXPLORE_ZOOM_IN_IMAGE,
    EXPLORE_ZOOM_OUT_IMAGE,
    EXPLORE_CAPTURE_MOUSE
  };

  enum EUniqueMode{
    INTERACT_NONE = 0, INTERACT_FREE, INTERACT_SWC_CONNECT, INTERACT_SWC_EXTEND,
    INTERACT_SWC_SMART_EXTEND, INTERACT_SWC_LOCK_FOCUS, INTERACT_SWC_ADD_NODE,
    INTERACT_SWC_MOVE_NODE, INTERACT_OBJECT_MOVE, INTERACT_STROKE_DRAW,
    INTERACT_RECT_DRAW, INTERACT_PUNCTA_MARK, INTERACT_IMAGE_MOVE,
    INTERACT_IMAGE_CAPTURE, INTERACT_IMAGE_ZOOM_IN, INTERACT_IMAGE_ZOOM_OUT,
    INTERACT_ADD_BOOKMARK, INTERACT_ADD_SYNAPSE, INTERACT_MOVE_SYNAPSE
  };

public:
  ZInteractiveContext();
  inline void setTraceMode(TraceMode mode) { m_traceMode = mode; }
  inline void setMarkPunctaMode(MarkPunctaMode mode) {m_markPunctaMode = mode;}
  inline void setTubeEditMode(TubeEditMode mode) { m_tubeEditMode = mode; }
  inline void setViewMode(ViewMode mode) { m_viewMode = mode; }
  inline void setExploreMode(ExploreMode mode) { m_exploreMode = mode; }
  inline void setSwcEditMode(SwcEditMode mode) { m_swcEditMode = mode; }
  inline void setStrokeEditMode(StrokeEditMode mode) { m_strokeEditMode = mode; }
  inline void setRectEditMode(RectEditMode mode) { m_rectEditMode = mode; }
  inline void setSynapseEditMode(SynapseEditMode mode) {
    m_synapseEditMode = mode; }
  inline void setBookmarkEditMode(BookmarkEditMode mode)
  { m_bookmarkEditMode = mode; }

  inline TraceMode traceMode() const { return m_traceMode; }
  inline TubeEditMode tubeEditMode() const { return m_tubeEditMode; }
  inline SwcEditMode swcEditMode() const { return m_swcEditMode; }
  inline ViewMode viewMode() const { return m_viewMode; }
  inline ExploreMode exploreMode() const { return m_exploreMode; }
  inline MarkPunctaMode editPunctaMode() const {return m_markPunctaMode;}
  inline StrokeEditMode strokeEditMode() const { return m_strokeEditMode; }
  inline RectEditMode rectEditMode() const { return m_rectEditMode; }
  inline BookmarkEditMode bookmarkEditMode() const { return m_bookmarkEditMode; }
  inline SynapseEditMode synapseEditMode() const { return m_synapseEditMode; }

  bool isTraceModeOff()  const;
  inline bool isReconPreview() const {
    return (m_traceMode == TRACE_PREIVEW_RECONSTRUCTION); }
  inline bool isTubeEditModeOff() const {
    return (m_tubeEditMode == TUBE_EDIT_OFF); }
  inline bool isExploreModeOff() const { return (m_exploreMode == EXPLORE_OFF); }
  inline bool isProjectView() const {
    return m_viewMode == VIEW_PROJECT; }
  inline bool isObjectProjectView() const {
    return (m_viewMode == VIEW_OBJECT_PROJECT) || isProjectView(); }
  inline bool isNormalView() const { return m_viewMode == VIEW_NORMAL; }
  inline bool isStackSliceView() const { return !isNormalView(); }
  inline bool isObjectSliceView() const { return !isObjectProjectView(); }
  //inline bool is3DView() {return m_viewMode == VIEW_3D; }
  inline bool fittingSegment() { return m_traceMode == TRACE_SINGLE; }
  inline bool tracingTube() { return m_traceMode == TRACE_TUBE; }
  inline bool markPuncta() {return m_markPunctaMode == MARK_PUNCTA;}
  inline void backupExploreMode() { m_oldExploreMode = m_exploreMode; }
  inline void restoreExploreMode() { m_exploreMode = m_oldExploreMode; }
  inline bool isStrokeEditModeOff() const {
    return m_strokeEditMode == STROKE_EDIT_OFF; }
  inline bool isRectEditModeOff() const {
    return m_rectEditMode == RECT_EDIT_OFF;
  }
  inline bool isBookmarkEditModeOff() const {
    return m_bookmarkEditMode == BOOKMARK_EDIT_OFF;
  }

  bool isContextMenuActivated() const;
  void blockContextMenu(bool blocking = true);

  inline void setExitingEdit(bool s) { m_exitingEdit = s; }
  inline bool isExitingEdit() const { return m_exitingEdit; }

  EUniqueMode getUniqueMode() const;
  //void setView(const QRect &projRegion, const QRect &viewPort);

  NeuTube::EAxis getSliceAxis() const { return m_sliceAxis; }
  void setSliceAxis(NeuTube::EAxis axis) { m_sliceAxis = axis; }

private:
  MarkPunctaMode m_markPunctaMode;
  TraceMode m_traceMode;
  TubeEditMode m_tubeEditMode;
  ViewMode m_viewMode;
  ExploreMode m_exploreMode;
  ExploreMode m_oldExploreMode;
  SwcEditMode m_swcEditMode;
  StrokeEditMode m_strokeEditMode;
  RectEditMode m_rectEditMode;
  BookmarkEditMode m_bookmarkEditMode;
  SynapseEditMode m_synapseEditMode;
  bool m_exitingEdit;
  bool m_blockingContextMenu;
  NeuTube::EAxis m_sliceAxis;
  //ZImageWidget *m_imageWidget;
  //QRect m_projRegion;
  //QRect m_viewPort;
};

#endif // ZINTERACTIVECONTEXT_H
